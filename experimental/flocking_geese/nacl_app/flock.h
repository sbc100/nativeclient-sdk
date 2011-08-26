// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLOCK_H_
#define FLOCK_H_

#include <string>
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

  double simulation_tick_duration() const {
    return sim_tick_duration_;
  }
  void set_simulation_tick_duration(double dt) {
    sim_tick_duration_ = dt;
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
  double sim_tick_duration_;  // Measured in milliseconds.
};

}  // namespace flocking_geese

#endif  // FLOCK_H_

