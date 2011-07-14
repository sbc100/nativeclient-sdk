// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIFE_H_
#define LIFE_H_

#include <string>
#include <tr1/memory>
#include <vector>

#include "experimental/conways_life/locking_image_data.h"
#include "experimental/conways_life/stamp.h"
#include "experimental/conways_life/threading/condition_lock.h"
#include "experimental/conways_life/threading/pthread_ext.h"
#include "ppapi/cpp/point.h"

namespace life {
// The main object that runs Conway's Life simulation (for details, see:
// http://en.wikipedia.org/wiki/Conway's_Game_of_Life).
class Life {
 public:
  // The possible simulation modes.  Modes with "Run" in their name will cause
  // the simulation thread to start running if it was paused.
  enum SimulationMode {
    kRunRandomSeed,
    kRunStamp,
    kPaused
  };

  Life();
  virtual ~Life();

  // Start up the simulation thread.
  void StartSimulation();

  // Set the automaton rules.  The rules are expressed as a string, with the
  // Birth and Keep Alive rules separated by a '/'.  The format follows the .LIF
  // 1.05 format here: http://psoup.math.wisc.edu/mcell/ca_files_formats.html
  // Survival/Birth.
  void SetAutomatonRules(const std::string& rule_string);

  // Resize the simulation to |width|, |height|.  This will delete and
  // reallocate all necessary buffer to the new size, and set all the
  // buffers to a new initial state.
  void Resize(int width, int height);

  // Delete all the cell buffers.  Sets the buffers to NULL.
  void DeleteCells();

  // Clear out the cell buffers (reset to all-dead).
  void ClearCells();

  // Stamp |stamp| at point |point| in both the pixel and cell buffers.
  void PutStampAtPoint(const Stamp& stamp, const pp::Point& point);

  // Stop the simulation.  Does nothing if the simulation is stopped.
  void StopSimulation();

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

  int width() const {
    return width_;
  }
  int height() const {
    return height_;
  }

  void set_pixel_buffer(
      const std::tr1::shared_ptr<LockingImageData>& pixel_buffer) {
    shared_pixel_buffer_ = pixel_buffer;
  }

  pthread_mutex_t* simulation_mutex() {
    return &life_simulation_mutex_;
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

  // Produce single bit random values.  Successive calls to value() should
  // return 0 or 1 with a random distribution.
  class RandomBitGenerator {
   public:
    // Initialize the random number generator with |initial_seed|.
    explicit RandomBitGenerator(unsigned int initial_seed)
        : random_bit_seed_(initial_seed) {}
    // Return the next random bit value.  Note that value() can't be a const
    // function because it changes the internal state machine as part of its
    // mechanism.
    uint8_t value();

   private:
    unsigned int random_bit_seed_;
    RandomBitGenerator();  // Not implemented, do not use.
  };

  // Take each character in |rule_string| and convert it into an index value,
  // set the bit in |life_rules_table_| at each of these indices, applying
  // |rule_offset|.  Assumes that all necessary locks have been acquired.  Does
  // no range checking or validation of the strings.
  void SetRuleFromString(size_t rule_offset,
                         const std::string& rule_string);

  // Add in some random noise to the borders of the simulation, which is used
  // to determine the life of adjacent cells.  This is part of a simulation
  // tick.
  void AddRandomSeed();

  // Draw the current state of the simulation into the pixel buffer.
  void UpdateCells();

  // Swap the input and output cell arrays.
  void Swap();

  // The main simulation loop.  This loop runs the Life simulation.  |param| is
  // a pointer to the Life instance.  This routine is run on its own thread.
  static void* LifeSimulation(void* param);

  // Thread support variables.
  pthread_t life_simulation_thread_;
  pthread_mutex_t life_simulation_mutex_;
  threading::ConditionLock sim_state_condition_;
  std::tr1::shared_ptr<LockingImageData> shared_pixel_buffer_;

  // Simulation variables.
  int width_;
  int height_;
  threading::ConditionLock simulation_mode_;
  RandomBitGenerator random_bits_;
  std::vector<uint8_t> life_rules_table_;
  uint8_t* cell_in_;
  uint8_t* cell_out_;
};

}  // namespace life

#endif  // LIFE_H_

