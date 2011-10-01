// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOCKING_GEESE_APP_H_
#define FLOCKING_GEESE_APP_H_

#include "nacl_app/flock.h"
#include "nacl_app/locking_image_data.h"
#include "nacl_app/png_loader.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"
#include "ppapi/cpp/var.h"
#include "scripting/scripting_bridge.h"

namespace flocking_geese {
// Manager class that contains the Flock simulation, and handles all the
// Pepper interaction (scripting, 2D graphics).  Pepper calls are all made
// on the main (browser) thread.  The pixel buffer that the simulation draws
// into is managed by this object, and shared with the simulation thread.
class FlockingGeeseApp : public pp::Instance {
 public:
  explicit FlockingGeeseApp(PP_Instance instance);
  virtual ~FlockingGeeseApp();

  // Called by the browser when the NaCl module is loaded and all ready to go.
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

  // Update the graphics context to the new size, and reallocate all new
  // buffers to the new size.
  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

  // Called by the browser to handle the postMessage() call in Javascript.
  virtual void HandleMessage(const pp::Var& message);

  // Flushes the contents of the simulation's pixel buffer  to the 2D graphics
  // context.  If needed, a new pixel buffer is created (for example, a new
  // pixel buffer is created when the simulation's view changes size).
  void Update();

  // Create a new flock of geese for the simulation.  Pauses the simulation
  // first, then calls ResetFlock() on the simulation with an initial location
  // that is the center of the simulation canvas.  If the simulation was
  // running before making this call, then start it up again with the newly-
  // created flock.  Takes one parameter: 'size' which is the new size of the
  // flock.
  void ResetFlock(const scripting::ScriptingBridge& bridge,
                  const scripting::MethodParameter& parameters);

  // Run the simulation.  Exposed to the browser as "runSimulation".  Takes
  // no parameters.
  void RunSimulation(const scripting::ScriptingBridge& bridge,
                     const scripting::MethodParameter& parameters);

  // Pause the simulation.  Does nothing if the simulation is stopped.
  // Exposed to the browser as "pauseSimulation".  Takes no parameters.
  void PauseSimulation(const scripting::ScriptingBridge& bridge,
                       const scripting::MethodParameter& parameters);

  // Set the attractor location.
  void SetAttractorLocation(const scripting::ScriptingBridge& bridge,
                            const scripting::MethodParameter& parameters);

  // Package up the current stats about the simulation (tick duration, etc.)
  // and post it as a formatted string back to the browser.  The string
  // starts with the method name "setSimulationInfo" followed and a list of
  // named parameters separated by spaces.  The parameter names are separated
  // from their values by a ':'.
  // For example:
  //   setSimulationInfo tickDuration:<int>
  // Note: this method can't be 'const' because PostMessage() isn't const.
  void PostSimulationInfo();

  int width() const {
    return shared_pixel_buffer_ ? shared_pixel_buffer_->size().width() : 0;
  }
  int height() const {
    return shared_pixel_buffer_ ? shared_pixel_buffer_->size().height() : 0;
  }

  // Indicate whether the simulation is running or paused.
  bool is_running() const {
    return flock_simulation_.simulation_mode() !=
        flocking_geese::Flock::kPaused;
  }

  // Indicate whether a flush is pending.  This can only be called from the
  // main thread; it is not thread safe.
  bool flush_pending() const {
    return flush_pending_;
  }
  void set_flush_pending(bool flag) {
    flush_pending_ = flag;
  }
 private:
  // Create and initialize the 2D context used for drawing.
  void CreateContext(const pp::Size& size);
  // Destroy the 2D drawing context.
  void DestroyContext();
  // Push the pixels to the browser, then attempt to flush the 2D context.  If
  // there is a pending flush on the 2D context, then update the pixels only
  // and do not flush.
  void FlushPixelBuffer();

  bool IsContextValid() const {
    return graphics_2d_context_ != NULL;
  }

  // The simulation.
  Flock flock_simulation_;

  // Browser connectivity and scripting support.
  scripting::ScriptingBridge scripting_bridge_;

  // 2D context variables.
  std::tr1::shared_ptr<LockingImageData> shared_pixel_buffer_;
  pp::Graphics2D* graphics_2d_context_;
  bool flush_pending_;
  bool view_changed_size_;
  pp::Size view_size_;

  // Resource management.
  PngLoader png_loader_;
};
}  // namespace flocking_geese

#endif  // FLOCKING_GEESE_APP_H_
