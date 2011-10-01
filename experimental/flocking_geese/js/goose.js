// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The Goose object.  This implements a single goose that flocks with other
 * geese in the flocking simulation.  Taken almost directly from
 * http://processingjs.org/learning/topic/flocking with references to
 * http://harry.me/2011/02/17/neat-algorithms---flocking.
 */


goog.provide('Goose');

goog.require('goog.Disposable');
goog.require('goog.math.Vec2');

/**
 * A Goose.  Starts the goose with a location and a velocity.
 * @param {?goog.math.Vec2} opt_location The initial location of this
 *     goose.  If not given, then the goose is started at (0, 0).
 * @param {?goog.math.Vec2} opt_velocity The initial velocity of this goose.
 *     If not given, then the goose is assigned a random velocity.
 * @constructor
 * @extends {goog.Disposable}
 */
Goose = function(opt_location, opt_velocity) {
  goog.Disposable.call(this);
  /**
   * The goose's current location.
   * @type {goog.math.Vec2}
   * @private
   */
  if (opt_location) {
    this.location_ = opt_location.clone();
  } else {
    this.location_ = new goog.math.Vec2(0, 0);
  }

  /**
   * The goose's current velocity.
   * @type {goog.math.Vec2}
   * @private
   */
  if (opt_velocity) {
    this.velocity_ = opt_velocity.clone();
  } else {
    this.velocity_ = new goog.math.Vec2.randomUnit();
  }
};
goog.inherits(Goose, goog.Disposable);

/**
 * The maximum speed of a goose.  Measured in meters/second.
 * @type {number}
 */
Goose.prototype.MAX_SPEED = 2.0;

/**
 * The maximum force that can be applied to turn a goose when computing the
 * aligment.  Measured in meters/second/second.
 * @type {number}
 */
Goose.prototype.MAX_TURNING_FORCE = 0.05;

/**
 * The neighbour radius of a goose.  Only geese within this radius will affect
 * the flocking computations of this goose.  Measured in pixels.
 * @type {number}
 */
Goose.prototype.NEIGHBOUR_RADIUS = 64.0;

/**
 * The minimum distance that a goose can be from this goose.  If another goose
 * comes within this distance of this goose, the flocking algorithm tries to
 * move the geese apart.  Measured in pixels.
 * @type {number}
 */
Goose.prototype.PERSONAL_SPACE = 32.0;

/**
 * The distance at which attractors have effect on a goose's direction.
 * @type {number}
 */
Goose.prototype.ATTRACTOR_RADIUS = 320.0;

/**
 * The goose will try to turn towards geese within this distance (computed
 * during the cohesion phase).  Measured in pixels.
 * @type {number}
 */
Goose.prototype.MAX_TURNING_DISTANCE = 100.0;

/**
 * The weights used when computing the weighted sum the three flocking
 * components.
 * @type {number}
 */
Goose.prototype.SEPARATION_WEIGHT = 2.0;
Goose.prototype.ALIGNMENT_WEIGHT = 1.0;
Goose.prototype.COHESION_WEIGHT = 1.0;

/**
 * Override of disposeInternal() to dispose of retained objects and unhook all
 * events.
 * @override
 */
Goose.prototype.disposeInternal = function() {
  this.domElement_ = null;
  Speedometer.superClass_.disposeInternal.call(this);
}

/**
 * Accessor for location.
 * @return {goog.math.Vec2} The current location.
 */
Goose.prototype.location = function() {
  return this.location_;
}

/**
 * Accessor for velocity.
 * @return {goog.math.Vec2} The current velocity.
 */
Goose.prototype.velocity = function() {
  return this.velocity_;
}

/**
 * Run one tick of the simulation.  Compute a new acceleration based on the
 * flocking algorithm (see Goose.flock()) and update the goose's location
 * by integrating acceleration and velocity.
 * @param {!Array.<Goose>} geese The list of all the geese in the flock.
 * @param {!Array.<goog.math.Vec2>} attractors The list of attractors.
 *     Geese have affinity for these points.
 * @param {?goog.math.Rect} opt_flockBox The geese will stay inside of this
 *     box.  If the parameter is not given, the geese don't have boundaries.
 */
Goose.prototype.simulationTick = function(geese, attractors, opt_flockBox) {
  var acceleration = this.flock(geese, attractors);
  this.velocity_.add(acceleration);
  // Limit the velocity to a maximum speed.
  this.velocity_.clamp(this.MAX_SPEED);
  this.location_.add(this.velocity_);
  // Wrap the goose location to the flock box.
  if (opt_flockBox) {
    if (this.location_.x < opt_flockBox.left) {
      this.location_.x = opt_flockBox.left + opt_flockBox.width;
    }
    if (this.location_.x > (opt_flockBox.left + opt_flockBox.width)) {
      this.location_.x = opt_flockBox.left;
    }
    if (this.location_.y < opt_flockBox.top) {
      this.location_.y = opt_flockBox.top + opt_flockBox.height;
    }
    if (this.location_.y > (opt_flockBox.top + opt_flockBox.height)) {
      this.location_.y = opt_flockBox.top;
    }
  }
}

/**
 * Implement the flocking algorithm in five steps:
 *   1. Compute the separation component,
 *   2. Compute the alignment component,
 *   3. Compute the flock cohesion component.
 *   4. Compute the effect of the attractors and blend this in with the
 *      cohesion component.
 *   5. Create a weighted sum of the three components and use this as the
 *      new acceleration for the goose.
 * This is an O(n^2) version of the algorithm.  There are ways to speed this
 * up using spatial coherence techniques, but this version is much simpler.
 * @param {!Array.<Goose>} geese The list of all the neighbouring geese (in
 *     this implementation, this is all the geese in the flock).
 * @param {!Array.<goog.math.Vec2>} attractors The list of attractors.
 *     Geese have affinity for these points.
 * @return {goog.math.Vec2} The acceleration vector for this goose based on the
 *     flocking algorithm.
 */
Goose.prototype.flock = function(geese, attractors) {
  // Loop over all the neighbouring geese in the flock, accumulating
  // the separation mean, the alignment mean and the cohesion mean.
  var separationCount = 0;
  var separation = new goog.math.Vec2(0, 0);
  var alignCount = 0;
  var alignment = new goog.math.Vec2(0, 0);
  var cohesionCount = 0;
  var cohesion = new goog.math.Vec2(0, 0);
  for (var i = 0; i < geese.length; i++) {
    var goose = geese[i];
    // Compute the distance from this goose to its neighbour.
    var gooseDirection = goog.math.Vec2.difference(
        this.location_, goose.location());
    var distance = gooseDirection.magnitude();
    separationCount = this.accumulateSeparation_(
        distance, gooseDirection, separation, separationCount);
    alignCount = this.accumulateAlignment_(
        distance, goose, alignment, alignCount);
    cohesionCount = this.accumulateCohesion_(
        distance, goose, cohesion, cohesionCount);
  }
  // Compute the means and create a weighted sum.  This becomes the goose's new
  // acceleration.
  if (separationCount > 0) {
    separation.scale(1.0 / separationCount);
  }
  if (alignCount > 0) {
    alignment.scale(1.0 / alignCount);
    // Limit the effect that alignment has on the final acceleration.  The
    // alignment component can overpower the others if there is a big
    // difference between this goose's velocity and its neighbours'.
    alignment.clamp(this.MAX_TURNING_FORCE);
  }

  // Compute the effect of the attractors and blend this in with the flock
  // cohesion component.  An attractor has to be within ATTRACTOR_RADIUS to
  // effect the heading of a goose.
  for (var i = 0; i < attractors.length; i++) {
    var attractorDirection = goog.math.Vec2.difference(
        attractors[i], this.location_);
    var distance = attractorDirection.magnitude();
    if (distance < this.ATTRACTOR_RADIUS) {
      attractorDirection.scale(1000);  // Each attractor acts like 1000 geese.
      cohesion.add(attractorDirection);
      cohesionCount += 1;
    }
  }

  // If there is a non-0 cohesion component, steer the goose so that it tries
  // to follow the flock.
  if (cohesionCount > 0) {
    cohesion.scale(1.0 / cohesionCount);
    cohesion = this.turnTowardsTarget(cohesion);
  }

  // Compute the weighted sum.
  separation.scale(this.SEPARATION_WEIGHT);
  alignment.scale(this.ALIGNMENT_WEIGHT);
  cohesion.scale(this.COHESION_WEIGHT);
  var weightedSum = cohesion.clone();
  weightedSum.add(alignment);
  weightedSum.add(separation);
  return weightedSum;
}

/**
 * Add a neighbouring goose's contribution to the separation mean.  Only
 * consider geese that have moved inside of this goose's personal space.
 * Modifies the separation accumulator |separation| in-place.
 * @param {!number} distance The distance from this goose to the neighbouring
 *     goose.
 * @param {!goog.math.Vec2} gooseDirection The direction vector from this
 *     goose to the neighbour.
 * @param {!goog.math.Vec2} separation The accumulated separation from all the
 *     neighbouring geese.
 * @param {!number} separationCount The current number of geese that have
 *     contributed to the separation component so far.
 * @return {!number} The new count of geese that contribute to the separation
 *     component.  If the goose under consideration does not contribute, this
 *     value is the same as |separationCount|.
 * @private
 */
Goose.prototype.accumulateSeparation_ = function(distance,
                                                 gooseDirection,
                                                 separation,
                                                 separationCount) {
  if (distance > 0 && distance < this.PERSONAL_SPACE) {
    var weightedDirection = gooseDirection;
    weightedDirection.normalize();
    weightedDirection.scale(1.0  / distance);
    separation.add(weightedDirection);
    separationCount++;
  }
  return separationCount;
}

/**
 * Add a neighbouring goose's contribution to the alignment mean.  Alignment is
 * the average velocity of the neighbours. Only consider geese that are within
 * |NEIGHBOUR_RADIUS|.  Modifies the alignment accumulator |alignment| in-place.
 * @param {!number} distance The distance from this goose to the neighbouring
 *     goose.
 * @param {!Goose} goose The neighbouring goose under consideration.
 * @param {!goog.math.Vec2} alignment The accumulated alignment from all the
 *     neighbouring geese.
 * @param {!number} alignCount The current number of geese that have
 *     contributed to the alignment component so far.
 * @return {!number} The new count of geese that contribute to the alignment
 *     component.  If the goose under consideration does not contribute, this
 *     value is the same as |alignCount|.
 * @private
 */
Goose.prototype.accumulateAlignment_ = function(distance,
                                                goose,
                                                alignment,
                                                alignCount) {
  if (distance > 0 && distance < this.NEIGHBOUR_RADIUS) {
    alignment.add(goose.velocity());
    alignCount++;
  }
  return alignCount;
}

/**
 * Add a neighbouring goose's contribution to the cohesion mean.  Cohesion is
 * based on the average location of the neighbours.  The goose attempts to
 * point to this average location.  Only consider geese that are within
 * |NEIGHBOUR_RADIUS|.  Modifies the cohesion accumulator |cohesion| in-place.
 * @param {!number} distance The distance from this goose to the neighbouring
 *     goose.
 * @param {!Goose} goose The neighbouring goose under consideration.
 * @param {!goog.math.Vec2} cohesion The accumulated cohesion from all the
 *     neighbouring geese.
 * @param {!number} cohesionCount The current number of geese that have
 *     contributed to the cohesion component so far.
 * @return {!number} The new count of geese that contribute to the cohesion
 *     component.  If the goose under consideration does not contribute, this
 *     value is the same as |cohesionCount|.
 * @private
 */
Goose.prototype.accumulateCohesion_ = function(distance,
                                               goose,
                                               cohesion,
                                               cohesionCount) {
  if (distance > 0 && distance < this.NEIGHBOUR_RADIUS) {
    cohesion.add(goose.location());
    cohesionCount++;
  }
  return cohesionCount;
}

/**
 * Turn the goose towards a target.  The amount of turning force is clamped
 * to |MAX_TURNING_FORCE|.
 * @param {!goog.math.Vec2} target Turn the goose towards this target.
 * @return {goog.math.Vec2} A vector representing the new direction of the
 *     goose.
 */
Goose.prototype.turnTowardsTarget = function(target) {
  var desiredDirection = goog.math.Vec2.difference(target, this.location_);
  var distance = desiredDirection.magnitude();
  var newDirection = new goog.math.Vec2(0, 0);
  if (distance > 0) {
    desiredDirection.normalize();
    // If the target is within the turning affinity distance, then make the
    // desired direction based on distance to the target.  Otherwise, base
    // the desired direction on MAX_SPEED.
    if (distance < this.MAX_TURNING_DISTANCE) {
      // Some pretty arbitrary dampening.
      desiredDirection.scale(this.MAX_SPEED * distance / 100.0);
    } else {
      desiredDirection.scale(this.MAX_SPEED);
    }
    newDirection = goog.math.Vec2.difference(desiredDirection, this.velocity_);
    newDirection.clamp(this.MAX_TURNING_FORCE);
  }
  return newDirection;
}

/**
 * Clamp a vector to a maximum magnitude.  Works on the vector in-place.
 * @param {!goog.math.Vec2} v The vector.
 * @param {!number} max_mag The maximum magnitude of the vector.
 */
goog.math.Vec2.prototype.clamp = function(max_mag) {
  var mag = this.magnitude();
  if (mag > max_mag && max_mag != 0) {
    this.normalize();
    this.scale(max_mag);
  }
}

/**
 * Compute the "heading" of a vector - this is the angle in radians between
 * the vector and the x-axis.
 * @return {!number} The "heading" angle in radians.
 */
goog.math.Vec2.prototype.heading = function() {
  var angle = Math.atan2(this.y, this.x);
  return angle;
}

