// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nacl_app/goose.h"

namespace {
// The maximum speed of a goose.  Measured in meters/second.
const double kMaxSpeed = 2.0;

// The maximum force that can be applied to turn a goose when computing the
// aligment.  Measured in meters/second/second.
const double kMaxTurningForce = 0.05;

// The neighbour radius of a goose.  Only geese within this radius will affect
// the flocking computations of this goose.  Measured in pixels.
const double kNeighbourRadius = 64.0;

// The minimum distance that a goose can be from this goose.  If another goose
// comes within this distance of this goose, the flocking algorithm tries to
// move the geese apart.  Measured in pixels.
const double kPersonalSpace = 32.0;

// The distance at which attractors have effect on a goose's direction.
const double kAttractorRadius = 320.0;

// The goose will try to turn towards geese within this distance (computed
// during the cohesion phase).  Measured in pixels.
const double kMaxTurningDistance = 100.0;

// The weights used when computing the weighted sum the three flocking
// components.
const double kSeparationWeight = 2.0;
const double kAlignmentWeight = 1.0;
const double kCohesionWeight = 1.0;

}  // namespace

namespace flocking_geese {

Goose::Goose() : location_(0, 0) {
  velocity_ = Vector2::RandomUnit();
}

Goose::Goose(const Vector2& location) : location_(location) {
  velocity_ = Vector2::RandomUnit();
}

void Goose::SimulationTick(const std::vector<Goose>& geese,
                           const std::vector<Vector2>& attractors,
                           const pp::Rect& flockBox) {
  Vector2 acceleration = Flock(geese, attractors);
  velocity_.Add(acceleration);
  // Limit the velocity to a maximum speed.
  velocity_.Clamp(kMaxSpeed);
  location_.Add(velocity_);
  // Wrap the goose location to the flock box.
  if (!flockBox.IsEmpty()) {
    if (location_.x() < flockBox.x()) {
      location_.set_x(flockBox.right() - 1);
    }
    if (location_.x() >= flockBox.right()) {
      location_.set_x(flockBox.x());
    }
    if (location_.y() < flockBox.y()) {
      location_.set_y(flockBox.bottom() - 1);
    }
    if (location_.y() >= flockBox.bottom()) {
      location_.set_y(flockBox.y());
    }
  }
}

Vector2 Goose::Flock(const std::vector<Goose>& geese,
                     const std::vector<Vector2>& attractors) {
  // Loop over all the neighbouring geese in the flock, accumulating
  // the separation mean, the alignment mean and the cohesion mean.
  int32_t separationCount = 0;
  Vector2 separation;
  int32_t alignCount = 0;
  Vector2 alignment;
  int32_t cohesionCount = 0;
  Vector2 cohesion;

  for (std::vector<Goose>::const_iterator goose_it = geese.begin();
       goose_it < geese.end();
       ++goose_it) {
    const Goose& goose = *goose_it;
    // Compute the distance from this goose to its neighbour.
    Vector2 gooseDirection = Vector2::Difference(
        location_, goose.location());
    double distance = gooseDirection.Magnitude();
    separationCount = AccumulateSeparation(
        distance, gooseDirection, &separation, separationCount);
    alignCount = AccumulateAlignment(
        distance, goose, &alignment, alignCount);
    cohesionCount = AccumulateCohesion(
        distance, goose, &cohesion, cohesionCount);
  }
  // Compute the means and create a weighted sum.  This becomes the goose's new
  // acceleration.
  if (separationCount > 0) {
    separation.Scale(1.0 / static_cast<double>(separationCount));
  }
  if (alignCount > 0) {
    alignment.Scale(1.0 / static_cast<double>(alignCount));
    // Limit the effect that alignment has on the final acceleration.  The
    // alignment component can overpower the others if there is a big
    // difference between this goose's velocity and its neighbours'.
    alignment.Clamp(kMaxTurningForce);
  }

  // Compute the effect of the attractors and blend this in with the flock
  // cohesion component.  An attractor has to be within kAttractorRadius to
  // effect the heading of a goose.
  for (size_t i = 0; i < attractors.size(); ++i) {
    Vector2 attractorDirection = Vector2::Difference(
        attractors[i], location_);
    double distance = attractorDirection.Magnitude();
    if (distance < kAttractorRadius) {
      attractorDirection.Scale(1000);  // Each attractor acts like 1000 geese.
      cohesion.Add(attractorDirection);
      cohesionCount++;
    }
  }

  // If there is a non-0 cohesion component, steer the goose so that it tries
  // to follow the flock.
  if (cohesionCount > 0) {
    cohesion.Scale(1.0 / static_cast<double>(cohesionCount));
    cohesion = TurnTowardsTarget(cohesion);
  }
  // Compute the weighted sum.
  separation.Scale(kSeparationWeight);
  alignment.Scale(kAlignmentWeight);
  cohesion.Scale(kCohesionWeight);
  Vector2 weightedSum = cohesion;
  weightedSum.Add(alignment);
  weightedSum.Add(separation);
  return weightedSum;
}

Vector2 Goose::TurnTowardsTarget(const Vector2& target) {
  Vector2 desiredDirection = Vector2::Difference(target, location_);
  double distance = desiredDirection.Magnitude();
  Vector2 newDirection;
  if (distance > 0.0) {
    desiredDirection.Normalize();
    // If the target is within the turning affinity distance, then make the
    // desired direction based on distance to the target.  Otherwise, base
    // the desired direction on MAX_SPEED.
    if (distance < kMaxTurningDistance) {
      // Some pretty arbitrary dampening.
      desiredDirection.Scale(kMaxSpeed * distance / 100.0);
    } else {
      desiredDirection.Scale(kMaxSpeed);
    }
    newDirection = Vector2::Difference(desiredDirection, velocity_);
    newDirection.Clamp(kMaxTurningForce);
  }
  return newDirection;
}

int32_t Goose::AccumulateSeparation(double distance,
                                    const Vector2& gooseDirection,
                                    Vector2* separation, /* inout */
                                    int32_t separationCount) {
  if (distance > 0.0 && distance < kPersonalSpace) {
    Vector2 weightedDirection = gooseDirection;
    weightedDirection.Normalize();
    weightedDirection.Scale(1.0  / distance);
    separation->Add(weightedDirection);
    separationCount++;
  }
  return separationCount;
}

int32_t Goose::AccumulateAlignment(double distance,
                                   const Goose& goose,
                                   Vector2* alignment, /* inout */
                                   int32_t alignCount) {
  if (distance > 0.0 && distance < kNeighbourRadius) {
    alignment->Add(goose.velocity());
    alignCount++;
  }
  return alignCount;
}

int32_t Goose::AccumulateCohesion(double distance,
                                  const Goose& goose,
                                  Vector2* cohesion, /* inout */
                                  int32_t cohesionCount) {
  if (distance > 0.0 && distance < kNeighbourRadius) {
    cohesion->Add(goose.location());
    cohesionCount++;
  }
  return cohesionCount;
}

}  // namespace flocking_geese

