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
const uint32_t kBackgroundColor = 0xFFFFFFFF;  // Opaque white.
// The goose sprites rotate in increments of 5 degrees.
const double kGooseHeadingIncrement = (5.0 * M_PI) / 180.0;
}  // namespace

namespace flocking_geese {

Flock::Flock() : flock_simulation_thread_(NULL),
                 sim_state_condition_(kSimulationStopped),
                 simulation_mode_(kPaused) {
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
  frame_counter_.BeginFrame();
  for (std::vector<Goose>::iterator goose = geese_.begin();
       goose < geese_.end();
       ++goose) {
    goose->SimulationTick(geese_, flockBounds_);
  }
  frame_counter_.EndFrame();
}

void Flock::Render() {
  ScopedPixelLock scoped_pixel_lock(shared_pixel_buffer_);
  uint32_t* pixel_buffer = scoped_pixel_lock.pixels();
  if (pixel_buffer == NULL || goose_sprite_ == NULL) {
    // Note that if the pixel buffer never gets initialized, this won't ever
    // paint anything.  Which is probably the right thing to do.  Also, this
    // clause means that the image will not get the very first few sim cells,
    // since it's possible that this thread starts before the pixel buffer is
    // initialized.
    return;
  }
  // Clear out the pixel buffer, then render all the geese.
  pp::Size canvas_size = shared_pixel_buffer_->size();
  const size_t pixel_count = canvas_size.width() * canvas_size.height();
  std::fill(pixel_buffer, pixel_buffer + pixel_count, kBackgroundColor);
  threading::ScopedMutexLock scoped_mutex(simulation_mutex());
  if (!scoped_mutex.is_valid())
    return;

  int32_t sprite_stamp_width = goose_sprite_->size().height();
  pp::Rect sprite_src_rect(0, 0,
                           sprite_stamp_width, goose_sprite_->size().height());
  for (std::vector<Goose>::const_iterator goose = geese_.begin();
       goose < geese_.end();
       ++goose) {
    pp::Point dest_point(goose->location().x(), goose->location().y());
    double heading = goose->velocity().Heading();
    if (heading < 0.0)
      heading = M_PI * 2.0 + heading;
    int32_t sprite_index =
        static_cast<int32_t>(heading / kGooseHeadingIncrement) *
        sprite_stamp_width;
    sprite_src_rect.set_x(sprite_index);
    goose_sprite_->CompositeFromRectToPoint(
        sprite_src_rect,
        pixel_buffer, canvas_size, 0,
        dest_point);
  }
}

void* Flock::FlockSimulation(void* param) {
  Flock* flock = static_cast<Flock*>(param);
  // Run the Life simulation in an endless loop.  Shut this down when
  // is_simulation_running() returns |false|.
  flock->set_is_simulation_running(true);
  while (flock->is_simulation_running()) {
    flock->WaitForRunMode();
    flock->SimulationTick();
    //flock->Render();
  }
  return NULL;
}
}  // namespace flocking_geese

