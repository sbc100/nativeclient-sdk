// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nacl_app/flock.h"

#include <sys/time.h>
#include <algorithm>
#include <cmath>
#include <string>

#include "nacl_app/scoped_pixel_lock.h"
#include "ppapi/cpp/size.h"
#include "threading/scoped_mutex_lock.h"

namespace {
inline uint32_t MakeRGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
  return (((a) << 24) | ((r) << 16) | ((g) << 8) | (b));
}
}  // namespace

namespace flocking_geese {

Flock::Flock() : flock_simulation_thread_(NULL),
                 sim_state_condition_(kSimulationStopped),
                 simulation_mode_(kPaused),
                 sim_tick_duration_(0) {
  pthread_mutex_init(&flock_simulation_mutex_, NULL);
}

Flock::~Flock() {
  set_is_simulation_running(false);
  if (flock_simulation_thread_) {
    pthread_join(flock_simulation_thread_, NULL);
  }
  pthread_mutex_destroy(&flock_simulation_mutex_);
}

void Flock::StartSimulation() {
  pthread_create(&flock_simulation_thread_, NULL, FlockSimulation, this);
}

Flock::SimulationMode Flock::WaitForRunMode() {
  simulation_mode_.LockWhenNotCondition(kPaused);
  simulation_mode_.Unlock();
  return simulation_mode();
}

void Flock::Resize(const pp::Size& size) {
  flockBounds_.SetRect(0, 0,
                       std::max(size.width(), 0),
                       std::max(size.height(), 0));
}

void Flock::ResetFlock(size_t size, const Vector2& initialLocation) {
  threading::ScopedMutexLock scoped_mutex(simulation_mutex());
  if (!scoped_mutex.is_valid())
    return;
  geese_.clear();
  while (--size) {
    geese_.push_back(Goose(initialLocation));
  }
}

void Flock::SimulationTick() {
  struct timeval clockStart, clockEnd;
  gettimeofday(&clockStart, NULL);
  for (size_t goose = 0; goose < geese_.size(); goose++) {
    geese_[goose].SimulationTick(geese_, flockBounds_);
  }
  gettimeofday(&clockEnd, NULL);
  // Convert dt to milliseconds.
  double dt = ((clockEnd.tv_sec * 1000000.0) + clockEnd.tv_usec) -
              ((clockStart.tv_sec * 1000000.0) + clockStart.tv_usec);
  dt /= 1000.0;
  set_simulation_tick_duration(dt);
}

void* Flock::FlockSimulation(void* param) {
  Flock* flock = static_cast<Flock*>(param);
  // Run the Life simulation in an endless loop.  Shut this down when
  // is_simulation_running() returns |false|.
  flock->set_is_simulation_running(true);
  while (flock->is_simulation_running()) {
    SimulationMode sim_mode = flock->WaitForRunMode();
    flock->SimulationTick();
  }
  return NULL;
}
}  // namespace flocking_geese

