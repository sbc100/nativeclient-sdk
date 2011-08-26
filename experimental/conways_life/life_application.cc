// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "experimental/conways_life/life_application.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

#include "experimental/conways_life/audio/web_wav_sound_resource.h"
#include "experimental/conways_life/scoped_pixel_lock.h"
#include "experimental/conways_life/threading/scoped_mutex_lock.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/var.h"

namespace {
const char* const kClearMethodId = "clear";
const char* const kPutStampAtPointMethodId = "putStampAtPoint";
const char* const kRunSimulationMethodId = "runSimulation";
const char* const kSetAutomatonRulesMethodId = "setAutomatonRules";
const char* const kSetCurrentStampMethodId = "setCurrentStamp";
const char* const kSetStampSoundUrlMethodId = "setStampSoundUrl";
const char* const kStopSimulationMethodId = "stopSimulation";

const int kSimulationTickInterval = 10;  // Measured in msec.
const uint32_t kBackgroundColor = 0xFFFFFFFF;  // Opaque white.

// Simulation modes.  These strings are matched by the browser script.
const char* const kRandomSeedModeId = "random_seed";
const char* const kStampModeId = "stamp";

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
  life::LifeApplication* life_app = static_cast<life::LifeApplication*>(data);
  life_app->Update();
  if (life_app->is_running()) {
    pp::Module::Get()->core()->CallOnMainThread(
        kSimulationTickInterval,
        pp::CompletionCallback(&SimulationTickCallback, data),
        PP_OK);
  }
}

// Called from the browser when the 2D graphics have been flushed out to the
// device.
void FlushCallback(void* data, int32_t result) {
  static_cast<life::LifeApplication*>(data)->set_flush_pending(false);
}
}  // namespace

using scripting::ScriptingBridge;

namespace life {
LifeApplication::LifeApplication(PP_Instance instance)
    : pp::Instance(instance),
      graphics_2d_context_(NULL),
      flush_pending_(false),
      view_changed_size_(true),
      audio_player_(this) {
}

LifeApplication::~LifeApplication() {
  life_simulation_.set_is_simulation_running(false);
  DestroyContext();
}

bool LifeApplication::Init(uint32_t /* argc */,
                           const char* /* argn */[],
                           const char* /* argv */[]) {
  // Add all the methods to the scripting bridge.
  ScriptingBridge::SharedMethodCallbackExecutor
      clear_method(new scripting::MethodCallback<LifeApplication>(
          this, &LifeApplication::Clear));
  scripting_bridge_.AddMethodNamed(kClearMethodId, clear_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      put_stamp_method(new scripting::MethodCallback<LifeApplication>(
          this, &LifeApplication::PutStampAtPoint));
  scripting_bridge_.AddMethodNamed(kPutStampAtPointMethodId, put_stamp_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      run_sim_method(new scripting::MethodCallback<LifeApplication>(
          this, &LifeApplication::RunSimulation));
  scripting_bridge_.AddMethodNamed(kRunSimulationMethodId, run_sim_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      set_auto_rules_method(new scripting::MethodCallback<LifeApplication>(
          this, &LifeApplication::SetAutomatonRules));
  scripting_bridge_.AddMethodNamed(kSetAutomatonRulesMethodId,
                                   set_auto_rules_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      set_stamp_method(new scripting::MethodCallback<LifeApplication>(
          this, &LifeApplication::SetCurrentStamp));
  scripting_bridge_.AddMethodNamed(kSetCurrentStampMethodId, set_stamp_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      set_stamp_sound_method(new scripting::MethodCallback<LifeApplication>(
          this, &LifeApplication::SetStampSoundUrl));
  scripting_bridge_.AddMethodNamed(kSetStampSoundUrlMethodId,
                                   set_stamp_sound_method);

  ScriptingBridge::SharedMethodCallbackExecutor
      stop_sim_method(new scripting::MethodCallback<LifeApplication>(
          this, &LifeApplication::StopSimulation));
  scripting_bridge_.AddMethodNamed(kStopSimulationMethodId, stop_sim_method);

  life_simulation_.StartSimulation();
  return true;
}

void LifeApplication::HandleMessage(const pp::Var& message) {
  if (!message.is_string())
    return;
  scripting_bridge_.InvokeMethod(message.AsString());
}

void LifeApplication::DidChangeView(const pp::Rect& position,
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

void LifeApplication::Update() {
  if (flush_pending())
    return;  // Don't attempt to flush if one is pending.

  if (view_changed_size_) {
    // Delete the old pixel buffer and create a new one.
    // Pause the simulation before changing all the buffer sizes.
    Life::SimulationMode sim_mode = life_simulation_.simulation_mode();
    life_simulation_.set_simulation_mode(Life::kPaused);
    // Create a new device context with the new size.
    // Note: DestroyContext() releases the simulation's copy of the shared
    // pixel buffer.  *This has to happen before the reset() below!*  If the
    // simulation's copy of hte shared pointer is released last, then the
    // underlying ImageData is released off the main thread, which is not
    // supported in Pepper.
    DestroyContext();
    CreateContext(view_size_);
    threading::ScopedMutexLock scoped_mutex(
        life_simulation_.simulation_mutex());
    if (!scoped_mutex.is_valid())
      // This potentially leaves the simulation in a paused state, but getting
      // here means something is very wrong and the simulation probably can't
      // run anyways.
      return;
    life_simulation_.DeleteCells();
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
      life_simulation_.Resize(width(), height());
      life_simulation_.set_pixel_buffer(shared_pixel_buffer_);
      life_simulation_.set_simulation_mode(sim_mode);
    }
    view_changed_size_ = false;
  }
  FlushPixelBuffer();
}

void LifeApplication::SetCurrentStamp(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  std::string stamp_desc = GetParameterNamed("description", parameters);
  if (stamp_desc.length()) {
    stamp_.InitFromDescription(stamp_desc);
  }
}

void LifeApplication::Clear(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  // Temporarily pause the the simulation while clearing the buffers.
  volatile Life::SimulationMode sim_mode = life_simulation_.simulation_mode();
  if (sim_mode != Life::kPaused)
    life_simulation_.set_simulation_mode(Life::kPaused);
  life_simulation_.ClearCells();
  ScopedPixelLock scoped_pixel_lock(shared_pixel_buffer_);
  uint32_t* pixel_buffer = scoped_pixel_lock.pixels();
  if (pixel_buffer) {
    const size_t size = width() * height();
    std::fill(pixel_buffer,
              pixel_buffer + size,
              kBackgroundColor);
  }
  Update();  // Flushes the buffer correctly.
  if (sim_mode != Life::kPaused)
    life_simulation_.set_simulation_mode(sim_mode);
}

void LifeApplication::SetAutomatonRules(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  std::string rules = GetParameterNamed("rules", parameters);
  if (rules.length()) {
    life_simulation_.SetAutomatonRules(rules);
  }
}

void LifeApplication::RunSimulation(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  std::string sim_mode = GetParameterNamed("mode", parameters);
  if (sim_mode.length() == 0) {
    return;
  }
  if (sim_mode == kRandomSeedModeId) {
    life_simulation_.set_simulation_mode(life::Life::kRunRandomSeed);
  } else {
    life_simulation_.set_simulation_mode(life::Life::kRunStamp);
  }
  // Schedule a simulation tick to get things going.
  pp::Module::Get()->core()->CallOnMainThread(
      kSimulationTickInterval,
      pp::CompletionCallback(&SimulationTickCallback, this),
      PP_OK);
}

void LifeApplication::StopSimulation(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  // This will pause the simulation on the next tick.
  life_simulation_.set_simulation_mode(life::Life::kPaused);
}

void LifeApplication::PutStampAtPoint(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  std::string x_coord = GetParameterNamed("x", parameters);
  std::string y_coord = GetParameterNamed("y", parameters);
  if (x_coord.length() == 0 || y_coord.length() == 0) {
    return;
  }
  int32_t x = StringAsInt32(x_coord);
  int32_t y = StringAsInt32(y_coord);
  life_simulation_.PutStampAtPoint(stamp_, pp::Point(x, y));
  // Play the stamp sound.
  if (audio_player_.IsReady())
    audio_player_.Play();
  // If the simulation isn't running, make sure the stamp shows up.
  if (!is_running())
    Update();
}

void LifeApplication::SetStampSoundUrl(
    const scripting::ScriptingBridge& bridge,
    const scripting::MethodParameter& parameters) {
  std::string sound_url = GetParameterNamed("soundUrl", parameters);
  if (sound_url.length() == 0) {
    return;
  }
  audio::WebWavSoundResource* sound = new audio::WebWavSoundResource();
  sound->Init(sound_url, this);
  // |audio_player_| takes ownership of |sound| and is responsible for
  // deleting it.
  audio_player_.AssignAudioSource(sound);
}

void LifeApplication::CreateContext(const pp::Size& size) {
  if (IsContextValid())
    return;
  graphics_2d_context_ = new pp::Graphics2D(this, size, false);
  if (!BindGraphics(*graphics_2d_context_)) {
    printf("Couldn't bind the device context\n");
  }
}

void LifeApplication::DestroyContext() {
  threading::ScopedMutexLock scoped_mutex(life_simulation_.simulation_mutex());
  if (!scoped_mutex.is_valid()) {
    return;
  }
  ScopedPixelLock scoped_pixel_lock(shared_pixel_buffer_);
  shared_pixel_buffer_.reset();
  life_simulation_.set_pixel_buffer(shared_pixel_buffer_);
  if (!IsContextValid())
    return;
  delete graphics_2d_context_;
  graphics_2d_context_ = NULL;
}

void LifeApplication::FlushPixelBuffer() {
  if (!IsContextValid() || shared_pixel_buffer_ == NULL)
    return;
  set_flush_pending(true);
  graphics_2d_context_->PaintImageData(*shared_pixel_buffer_, pp::Point());
  graphics_2d_context_->Flush(pp::CompletionCallback(&FlushCallback, this));
}
}  // namespace life

