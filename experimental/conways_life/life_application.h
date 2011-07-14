// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIFE_APPLICATION_H_
#define LIFE_APPLICATION_H_

#include <cstdlib>
#include <map>
#include <memory>
#include <tr1/memory>
#include <vector>

#include "experimental/conways_life/audio/audio_player.h"
#include "experimental/conways_life/life.h"
#include "experimental/conways_life/locking_image_data.h"
#include "experimental/conways_life/scripting/scripting_bridge.h"
#include "experimental/conways_life/stamp.h"
#include "experimental/conways_life/threading/pthread_ext.h"
#include "experimental/conways_life/threading/condition_lock.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"

namespace life {
// The main object that runs Conway's Life simulation (for details, see:
// http://en.wikipedia.org/wiki/Conway's_Game_of_Life).  This class holds all
// the Pepper objects (the 2D context and the instance) required to interface
// with the browser.  It also owns an instance of the simulation object.
class LifeApplication : public pp::Instance {
 public:
  // The states for the |sim_state_condition_| condition lock.
  enum SimulationState {
    kStopped,
    kRunning
  };

  explicit LifeApplication(PP_Instance instance);
  virtual ~LifeApplication();

  // Called by the browser when the NaCl module is loaded and all ready to go.
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

  // Update the graphics context to the new size, and reallocate all new
  // buffers to the new size.
  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

  // Called by the browser to handle the postMessage() call in Javascript.
  virtual void HandleMessage(const pp::Var& message);

  // Runs a tick of the simulations, updating all buffers.  Flushes the
  // contents of |pixel_buffer_| to the 2D graphics context.
  void Update();

  // Replace the current stamp.  See stamp.h for a description of the stamp
  // format.  Exposed to the browser as "setCurrentStamp".  Takes a parameter
  // named "description" that contains the string-encoded stamp description.
  void SetCurrentStamp(const scripting::ScriptingBridge& bridge,
                       const scripting::MethodParameter& parameters);

  // Set the automaton rules.  The rules are expressed as a string, with the
  // Birth and Keep Alive rules separated by a '/'.  The format follows the .LIF
  // 1.05 format here: http://psoup.math.wisc.edu/mcell/ca_files_formats.html
  // Survival/Birth.  Exposed to the browser as "setAutomatonRules".
  // Takes a single parameter named "rules".
  void SetAutomatonRules(const scripting::ScriptingBridge& bridge,
                         const scripting::MethodParameter& parameters);

  // Clears the current simulation (resets back to all-dead, graphics buffer to
  // black).  Exposed to the browser as "clear()".  |args| can be empty.
  void Clear(const scripting::ScriptingBridge& bridge,
             const scripting::MethodParameter& parameters);

  // Plot a new blob of life centered around the (x, y) coordinates described
  // in |parameters|.  This method is exposed to the browser as
  // "putStampAtPoint", it takes a parameter named "x" and one named "y".
  void PutStampAtPoint(const scripting::ScriptingBridge& bridge,
                       const scripting::MethodParameter& parameters);

  // Run the simulation in the specified mode.  If the mode is changed, then
  // the simulation is stopped and restarted in the new mode.  Takes one
  // parameter named "mode" which can be one of "random_seed" or "stamp".
  // Exposed to the browser as "runSimulation".
  void RunSimulation(const scripting::ScriptingBridge& bridge,
                     const scripting::MethodParameter& parameters);

  // Stop the simulation.  Does nothing if the simulation is stopped.
  // Exposed to the browser as "stopSimulation".  Takes no parameters.
  void StopSimulation(const scripting::ScriptingBridge& bridge,
                      const scripting::MethodParameter& parameters);

  // Set the URL used to get the stamp sound.  Takes one parameter named
  // "soundUrl" that is the URL of the sound file.  Does not affect a
  // stamp sound that is currently playing, but can affect all subsequent plays
  // of the stamp sound.  If there are errors fetching the sound data from
  // |soundUrl|, then the stamp sound is left unchanged.  Exposed to the browser
  // as "setStampSoundUrl".
  void SetStampSoundUrl(const scripting::ScriptingBridge& bridge,
                        const scripting::MethodParameter& parameters);

  int width() const {
    return shared_pixel_buffer_ ? shared_pixel_buffer_->size().width() : 0;
  }
  int height() const {
    return shared_pixel_buffer_ ? shared_pixel_buffer_->size().height() : 0;
  }

  // Indicate whether the simulation is running or paused.
  bool is_running() const {
    return life_simulation_.simulation_mode() != life::Life::kPaused;
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
  // Return a pointer to the pixels without acquiring the pixel buffer lock.
  uint32_t* PixelBufferNoLock();

  bool IsContextValid() const {
    return graphics_2d_context_ != NULL;
  }

  // Browser connectivity and scripting support.
  scripting::ScriptingBridge scripting_bridge_;

  // The simulation.
  Life life_simulation_;

  // 2D context variables.
  std::tr1::shared_ptr<LockingImageData> shared_pixel_buffer_;
  pp::Graphics2D* graphics_2d_context_;
  bool flush_pending_;
  bool view_changed_size_;
  pp::Size view_size_;

  // The current stamp.  The dictionary of stamps is kept in the browser.
  Stamp stamp_;

  // Audio.
  audio::AudioPlayer audio_player_;
};

}  // namespace life

#endif  // LIFE_APPLICATION_H_

