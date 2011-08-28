// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flocking_geese_app.h"

#include <algorithm>
#include <sstream>
#include <string>
#include "nacl_app/scoped_pixel_lock.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/module.h"
#include "threading/scoped_mutex_lock.h"

namespace {
// Input method and parameter Ids.  These correspond to C++ calls.
const char* const kResetFlockMethodId = "resetFlock";
const char* const kRunSimulationMethodId = "runSimulation";
const char* const kPauseSimulationMethodId = "pauseSimulation";

// Output method and parameter Ids.  These correspond to methods in the
// browser.
const char* const kSetSimulationInfoMethodId = "setSimulationInfo";
const char* const kFrameRateParamId = "frameRate";

const int kSimulationTickInterval = 10;  // Measured in msec.
const uint32_t kBackgroundColor = 0xFFFFFFFF;  // Opaque white.

// Return the value of parameter named |param_name| from |parameters|.  If
// |param_name| doesn't exist, then return an empty string.
std::string GetParameterNamed(
    const std::string& param_name,
    const scripting::MethodParameter& parameters) {
  scripting::MethodParameter::const_iterator i =
      parameters.find(param_name);
  if (i == parameters.end()) {
    return "";
  }
  return i->second;
}

// Return the int32_t equivalent of |str|.  All the usual C++ rounding rules
// apply.
int32_t StringAsInt32(const std::string& str) {
  std::istringstream cvt_stream(str);
  double double_val;
  cvt_stream >> double_val;
  return static_cast<int32_t>(double_val);
}

// Called from the browser when the delay time has elapsed.  This routine
// runs a simulation update, then reschedules itself to run the next
// simulation tick.
void SimulationTickCallback(void* data, int32_t result) {
  flocking_geese::FlockingGeeseApp* geese_app =
      static_cast<flocking_geese::FlockingGeeseApp*>(data);
  // Flush the graphics so far and post the current frame rate.
  geese_app->PostSimulationInfo();
  geese_app->Update();
  if (geese_app->is_running()) {
    pp::Module::Get()->core()->CallOnMainThread(
        kSimulationTickInterval,
        pp::CompletionCallback(&SimulationTickCallback, data),
        PP_OK);
  }
}

// Called from the browser when the 2D graphics have been flushed out to the
// device.
void FlushCallback(void* data, int32_t result) {
  static_cast<flocking_geese::FlockingGeeseApp*>(data)->
      set_flush_pending(false);
}
}  // namespace

using scripting::ScriptingBridge;

namespace flocking_geese {

FlockingGeeseApp::FlockingGeeseApp(PP_Instance instance)
    : pp::Instance(instance),
      graphics_2d_context_(NULL),
      flush_pending_(false),
      view_changed_size_(true) {
}

FlockingGeeseApp::~FlockingGeeseApp() {
  flock_simulation_.set_is_simulation_running(false);
  DestroyContext();
}

bool FlockingGeeseApp::Init(uint32_t /* argc */,
                            const char* /* argn */[],
                            const char* /* argv */[]) {
  // Add all the methods to the scripting bridge.
  ScriptingBridge::SharedMethodCallbackExecutor
      reset_flock_method(new scripting::MethodCallback<FlockingGeeseApp>(
          this, &FlockingGeeseApp::ResetFlock));
  scripting_bridge_.AddMethodNamed(kResetFlockMethodId, reset_flock_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      run_sim_method(new scripting::MethodCallback<FlockingGeeseApp>(
          this, &FlockingGeeseApp::RunSimulation));
  scripting_bridge_.AddMethodNamed(kRunSimulationMethodId, run_sim_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      pause_sim_method(new scripting::MethodCallback<FlockingGeeseApp>(
          this, &FlockingGeeseApp::PauseSimulation));
  scripting_bridge_.AddMethodNamed(kPauseSimulationMethodId, pause_sim_method);

  flock_simulation_.StartSimulation();
  return true;
}

void FlockingGeeseApp::DidChangeView(const pp::Rect& position,
                                     const pp::Rect& /* clip */) {
  if (position.size().width() == width() &&
      position.size().height() == height())
    return;  // Size didn't change, no need to update anything.
  // Indicate that all the buffers need to be resized at the next Update()
  // call.
  view_changed_size_ = true;
  view_size_ = position.size();
  // Make sure the buffers get changed if the simulation isn't running.
  if (!is_running())
    Update();
}

void FlockingGeeseApp::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string())
    return;
  scripting_bridge_.InvokeMethod(var_message.AsString());
}

void FlockingGeeseApp::ResetFlock(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  std::string flock_size_str = GetParameterNamed("size", parameters);
  if (flock_size_str.length()) {
    Flock::SimulationMode sim_mode = flock_simulation_.simulation_mode();
    flock_simulation_.set_simulation_mode(Flock::kPaused);
    size_t flock_size = static_cast<size_t>(StringAsInt32(flock_size_str));
    Vector2 initialLocation(view_size_.width(), view_size_.height());
    initialLocation.Scale(0.5);
    flock_simulation_.ResetFlock(flock_size, initialLocation);
    flock_simulation_.set_simulation_mode(sim_mode);
  }
}

void FlockingGeeseApp::RunSimulation(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  flock_simulation_.set_simulation_mode(flocking_geese::Flock::kRunning);
  // Schedule a simulation tick to get things going.
  pp::Module::Get()->core()->CallOnMainThread(
      kSimulationTickInterval,
      pp::CompletionCallback(&SimulationTickCallback, this),
      PP_OK);
}

void FlockingGeeseApp::PauseSimulation(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  // This will pause the simulation on the next tick.
  flock_simulation_.set_simulation_mode(flocking_geese::Flock::kPaused);
}

void FlockingGeeseApp::PostSimulationInfo() {
  double frame_rate = flock_simulation_.FrameRate();
  std::ostringstream method_invocation(std::ostringstream::out);
  method_invocation << kSetSimulationInfoMethodId << " ";
  method_invocation << kFrameRateParamId << ":" << frame_rate;
  pp::Var var_message(method_invocation.str());
  PostMessage(var_message);
}

void FlockingGeeseApp::Update() {
  if (flush_pending())
    return;  // Don't attempt to flush if one is pending.

  if (view_changed_size_) {
    // Delete the old pixel buffer and create a new one.
    // Pause the simulation before changing all the buffer sizes.
    Flock::SimulationMode sim_mode = flock_simulation_.simulation_mode();
    flock_simulation_.set_simulation_mode(Flock::kPaused);
    // Create a new device context with the new size.
    // Note: DestroyContext() releases the simulation's copy of the shared
    // pixel buffer.  *This has to happen before the reset() below!*  If the
    // simulation's copy of hte shared pointer is released last, then the
    // underlying ImageData is released off the main thread, which is not
    // supported in Pepper.
    DestroyContext();
    CreateContext(view_size_);
    threading::ScopedMutexLock scoped_mutex(
        flock_simulation_.simulation_mutex());
    if (!scoped_mutex.is_valid())
      // This potentially leaves the simulation in a paused state, but getting
      // here means something is very wrong and the simulation probably can't
      // run anyways.
      return;
    if (graphics_2d_context_ != NULL) {
      shared_pixel_buffer_.reset(
          new LockingImageData(this,
                               PP_IMAGEDATAFORMAT_BGRA_PREMUL,
                               graphics_2d_context_->size(),
                               false));
      set_flush_pending(false);
      // Ok to get a non-locked version because the simulation is guraranteed
      // to be paused here.
      uint32_t* pixels = shared_pixel_buffer_->PixelBufferNoLock();
      if (pixels) {
        const size_t size = width() * height();
        std::fill(pixels, pixels + size, kBackgroundColor);
      }
      flock_simulation_.Resize(pp::Size(width(), height()));
      flock_simulation_.set_pixel_buffer(shared_pixel_buffer_);
      flock_simulation_.set_simulation_mode(sim_mode);
    }
    view_changed_size_ = false;
  }
  FlushPixelBuffer();
}

void FlockingGeeseApp::CreateContext(const pp::Size& size) {
  if (IsContextValid())
    return;
  graphics_2d_context_ = new pp::Graphics2D(this, size, false);
  if (!BindGraphics(*graphics_2d_context_)) {
    PostMessage("Couldn't bind the device context");
  }
}

void FlockingGeeseApp::DestroyContext() {
  threading::ScopedMutexLock scoped_mutex(flock_simulation_.simulation_mutex());
  if (!scoped_mutex.is_valid()) {
    return;
  }
  ScopedPixelLock scoped_pixel_lock(shared_pixel_buffer_);
  shared_pixel_buffer_.reset();
  flock_simulation_.set_pixel_buffer(shared_pixel_buffer_);
  if (!IsContextValid())
    return;
  delete graphics_2d_context_;
  graphics_2d_context_ = NULL;
}

void FlockingGeeseApp::FlushPixelBuffer() {
  if (!IsContextValid() || shared_pixel_buffer_ == NULL)
    return;
  set_flush_pending(true);
  graphics_2d_context_->PaintImageData(*shared_pixel_buffer_, pp::Point());
  graphics_2d_context_->Flush(pp::CompletionCallback(&FlushCallback, this));
}
}  // namespace flocking_geese

