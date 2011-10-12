// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nacl_app/flock.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

#include "nacl_app/scoped_pixel_lock.h"
#include "ppapi/cpp/size.h"
#include "threading/scoped_mutex_lock.h"

namespace {
const uint32_t kBackgroundColor = 0xFFFFFFFF;  // Opaque white.

// Throttle the simulation to run a fixed number of ticks per Render() call.
const int32_t kSimulationTicksPerRender = 16;

// The goose sprites rotate in increments of 5 degrees.
const double kGooseHeadingIncrement = (5.0 * M_PI) / 180.0;
}  // namespace

namespace flocking_geese {

Flock::Flock() : flock_simulation_thread_(NULL),
                 sim_state_condition_(kSimulationStopped),
                 simulation_mode_(kPaused),
                 tick_counter_(0) {
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

void Flock::set_simulation_mode(Flock::SimulationMode new_mode) {
  simulation_mode_.Lock();
  if (new_mode == kRunning)
    set_tick_counter(0);
  simulation_mode_.UnlockWithCondition(new_mode);
}

Flock::SimulationMode Flock::WaitForRunMode() {
  simulation_mode_.LockWhenCondition(kRunning);
  simulation_mode_.Unlock();
  return simulation_mode();
}

void Flock::SetAttractorAtIndex(const Vector2& attractor, size_t index) {
  if (index >= attractors_.size()) {
    attractors_.resize(index + 1, Vector2());
  }
  attractors_[index] = attractor;
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
  geese_.resize(size + 1);
  geese_.assign(size, Goose(initialLocation));
  frame_counter_.Reset();
}

void Flock::SimulationTick() {
  frame_counter_.BeginFrame();
  for (std::vector<Goose>::iterator goose = geese_.begin();
       goose < geese_.end();
       ++goose) {
    goose->SimulationTick(geese_, attractors_, flockBounds_);
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
  pp::Rect canvas_bounds(pp::Point(), shared_pixel_buffer_->size());
  int32_t canvas_width = canvas_bounds.size().width();
  int32_t canvas_height = canvas_bounds.size().height();
  const size_t pixel_count = canvas_width * canvas_height;
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
    pp::Point dest_point(goose->location().x() - sprite_stamp_width / 2,
                         goose->location().y() - sprite_stamp_width / 2);
    double heading = goose->velocity().Heading();
    if (heading < 0.0)
      heading = M_PI * 2.0 + heading;
    int32_t sprite_index =
        static_cast<int32_t>(heading / kGooseHeadingIncrement) *
        sprite_stamp_width;
    sprite_src_rect.set_x(sprite_index);
    goose_sprite_->CompositeFromRectToPoint(
        sprite_src_rect,
        pixel_buffer, canvas_bounds, 0,
        dest_point);
  }

  // Hack: draw a blue rectangle at each attractor.
  // TODO(dspringer): This is ridiculous.  Fix this when a proper graphics lib
  // available.
  for (std::vector<Vector2>::const_iterator attractor = attractors_.begin();
       attractor < attractors_.end();
       ++attractor) {
    int32_t x = std::max(static_cast<int32_t>(attractor->x()) - 2, 0);
    int32_t y = std::max(static_cast<int32_t>(attractor->y()) - 2, 0);
    int32_t w = 4, h = 4;
    int32_t rw = canvas_width;
    if (x + w >= rw) { x = rw - 1 - w; }
    if (y + h >= canvas_height) { y = canvas_height - 1 - h; }
    uint32_t* rect = pixel_buffer + y * rw + x;
    for (int32_t row = 0; row < h; ++row) {
      uint32_t* scanline = rect;
      rect += rw;
      *scanline++ = 0xFF0000FF;
      *scanline++ = 0xFF0000FF;
      *scanline++ = 0xFF0000FF;
      *scanline++ = 0xFF0000FF;
    }
  }

  if (simulation_mode() == kRenderThrottle) {
    set_simulation_mode(kRunning);
  }
}

void* Flock::FlockSimulation(void* param) {
  Flock* flock = static_cast<Flock*>(param);
  // Run the Life simulation in an endless loop.  Shut this down when
  // is_simulation_running() returns |false|.
  flock->set_is_simulation_running(true);
  flock->set_tick_counter(0);
  while (flock->is_simulation_running()) {
    flock->WaitForRunMode();
    flock->SimulationTick();
    // Throttle the simulation so it doesn't outrun the browser by too much.
    if (flock->increment_tick_counter() > kSimulationTicksPerRender) {
      flock->set_simulation_mode(kRenderThrottle);
    }
    //flock->Render();
  }
  return NULL;
}
}  // namespace flocking_geese

