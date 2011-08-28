// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOCK_H_
#define FLOCK_H_

#include <string>
#include <time.h>
#include <tr1/memory>
#include <vector>

#include "nacl_app/goose.h"
#include "nacl_app/locking_image_data.h"
#include "nacl_app/scoped_pixel_lock.h"
#include "nacl_app/vector2.h"
#include "ppapi/cpp/point.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"
#include "threading/condition_lock.h"
#include "threading/pthread_ext.h"

namespace flocking_geese {
// The main object that runs a simple flocking simulation.  This object owns
// a flock of Geese.  See:
// http://processingjs.org/learning/topic/flocking with references to
// http://harry.me/2011/02/17/neat-algorithms---flocking.
class Flock {
 public:
  // The possible simulation modes.
  enum SimulationMode {
    kRunning,
    kPaused
  };

  Flock();
  virtual ~Flock();

  // Start up the simulation thread.  The simultion is started in paused
  // mode, to get the simulation running you have to set the simulation
  // mode to kRunning (see set_simulaiton_mode());
  void StartSimulation();

  // Resize the simulation to |size|.  The new dimensions are used as the
  // boundaries for the geese.
  void Resize(const pp::Size& size);

  // Create a flock of geese.  The geese start at the given location with
  // random velocities.  Any previous geese are deleted and the simulation
  // starts with an entirely new flock.
  // @param size The size of the flock.
  // @param initialLocation The initial location of each goose in the flock.
  void ResetFlock(size_t size, const Vector2& initialLocation);

  // Run one tick of the simulation.  This records the tick duration.
  void SimulationTick();

  // Render the flock into the shared pixel buffer.
  void Render();

  // Sleep on the condition that the current simulation mode is not the
  // paused mode.  Returns the new run mode.
  SimulationMode WaitForRunMode();

  SimulationMode simulation_mode() const {
    return static_cast<SimulationMode>(simulation_mode_.condition_value());
  }
  void set_simulation_mode(SimulationMode new_mode) {
    simulation_mode_.Lock();
    simulation_mode_.UnlockWithCondition(new_mode);
  }

  double FrameRate() const {
    return frame_counter_.frames_per_second();
  }

  void set_pixel_buffer(
      const std::tr1::shared_ptr<LockingImageData>& pixel_buffer) {
    shared_pixel_buffer_ = pixel_buffer;
  }

  pthread_mutex_t* simulation_mutex() {
    return &flock_simulation_mutex_;
  }

  // Set the condition lock to indicate whether the simulation thread is
  // running.
  bool is_simulation_running() const {
    return sim_state_condition_.condition_value() != kSimulationStopped;
  }
  void set_is_simulation_running(bool flag) {
    sim_state_condition_.Lock();
    sim_state_condition_.UnlockWithCondition(
        flag ? kSimulationRunning : kSimulationStopped);
  }

 private:
  // A small helper class that keeps track of tick deltas and a tick
  // count.  It accumulates the duration of each frame.  For every second of
  // accumulated time (not wall time), update a frame rate couter.
  class FrameCounter {
   public:
    FrameCounter()
        : frame_duration_accumulator_(0),
          frame_count_(0),
          frames_per_second_(0) {}
    ~FrameCounter() {}

    // Record the current time, which is used to compute the frame duration
    // when EndFrame() is called.
    void BeginFrame() {
      struct timeval start_time;
      gettimeofday(&start_time, NULL);
      frame_start_ = start_time.tv_sec * kMicroSecondsPerSecond +
                     start_time.tv_usec;
    }

    // Compute the delta since the last call to BeginFrame() and increment the
    // frame count.  If a second or more of time has accumulated in the
    // duration accumulator, then update the frames-per-second value.
    void EndFrame() {
      struct timeval end_time;
      gettimeofday(&end_time, NULL);
      double frame_end = end_time.tv_sec * kMicroSecondsPerSecond +
                         end_time.tv_usec;
      double dt = frame_end - frame_start_;
      if (dt < 0)
        return;
      frame_duration_accumulator_ += dt;
      frame_count_++;
      if (frame_duration_accumulator_ >= kMicroSecondsPerSecond) {
        double elapsed_time = frame_duration_accumulator_ /
                              kMicroSecondsPerSecond;
        frames_per_second_ = frame_count_ / elapsed_time;
        frame_duration_accumulator_ = 0;
        frame_count_ = 0;
      }
    }

    // The current frame rate.  Note that this is 0 for the first second in
    // the accumulator, and is updated about once per second.
    double frames_per_second() const {
      return frames_per_second_;
    }

   private:
    static const double kMicroSecondsPerSecond = 1000000.0;

    double frame_duration_accumulator_;  // Measured in microseconds.
    int32_t frame_count_;
    double frame_start_;
    double frames_per_second_;
  };

  // The state of the main simulaiton thread.  These are values that the
  // simulation condition lock can have.
  enum SimulationState {
    kSimulationStopped,
    kSimulationRunning
  };

  // The main simulation loop.  This loop runs the Life simulation.  |param| is
  // a pointer to the Life instance.  This routine is run on its own thread.
  static void* FlockSimulation(void* param);

  // Thread support variables.
  pthread_t flock_simulation_thread_;
  pthread_mutex_t flock_simulation_mutex_;
  threading::ConditionLock sim_state_condition_;
  std::tr1::shared_ptr<LockingImageData> shared_pixel_buffer_;

  // Simulation variables.
  pp::Rect flockBounds_;
  threading::ConditionLock simulation_mode_;
  std::vector<Goose> geese_;
  FrameCounter frame_counter_;
};

}  // namespace flocking_geese

#endif  // FLOCK_H_

