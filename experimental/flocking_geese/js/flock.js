// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The JavaScript simulation of the flocking algorithm.  Owns an array of
 * geese, and runs the flocking simulation that computes new goose locations.
 */

goog.provide('Flock');

goog.require('FrameCounter');
goog.require('Goose');
goog.require('goog.Disposable');
goog.require('goog.array');

/**
 * Constructor for the Flock class.
 * @constructor
 * @extends {goog.Disposable}
 */
Flock = function() {
  goog.Disposable.call(this);

  /**
   * The flock of geese.
   * @type {Array.<Goose>}
   * @private
   */
  this.geese_ = [];

  /**
   * An array of attractors.  The flock has affinity for these points, and
   * will head toward them when close enough.
   * @type {Array.<goog.Math.Vec2>}
   */
  this.attractors = [];

  /**
   * The framerate meter.
   * type {FrameCounter}
   * @private
   */
  this.frameCounter_ = new FrameCounter();

  /**
   * The goose sprite image.
   * @type {Image}
   * @private
   */
  this.gooseImage_ = new Image();
  this.gooseImage_.src = 'images/Flock32_red.png';
}
goog.inherits(Flock, goog.Disposable);

/**
 * The angular increment between goose sprites.  Measured in radians.
 * @type {number}
 * @private
 */
Flock.prototype.GOOSE_HEADING_INCREMENT_ = (5.0 * Math.PI) / 180.0;

/**
 * Override of disposeInternal() to dispose of retained objects.
 * @override
 */
Flock.prototype.disposeInternal = function() {
  goog.array.clear(this.geese_);
  delete this.frameCounter_;
  Flock.superClass_.disposeInternal.call(this);
}

/**
 * Create a flock of geese.  The geese start at the given location with
 * random velocities.  Any previous geese are deleted and the simulation
 * starts with an entirely new flock.
 * @param {!number} size The size of the flock.
 * @param {?goog.math.Vec2} opt_initialLocation The initial location of each
 *     goose in the flock.
 */
Flock.prototype.resetFlock = function(size, opt_initialLocation) {
  goog.array.clear(this.geese_);
  var initialLocation = opt_initialLocation || new goog.math.Vec2(0, 0);
  for (var goose = 0; goose < size; goose++) {
    this.geese_[goose] = new Goose(initialLocation);
  }
  this.frameCounter_.reset();
}

/**
 * Run one tick of the simulation, recording the time.
 * @param {?goog.math.Rect} opt_flockBox The geese will stay inside of this
 *     box.  If the parameter is not given, the geese don't have boundaries.
 * @return {number} the simulation tick duration in miliseconds.
 */
Flock.prototype.flock = function(opt_flockBox) {
  var flockBox = opt_flockBox || new goog.math.Rect(0, 0, 200, 200);
  this.frameCounter_.beginFrame();
  for (var goose = 0; goose < this.geese_.length; goose++) {
    this.geese_[goose].simulationTick(this.geese_, this.attractors, flockBox);
  }
  this.frameCounter_.endFrame();
  return this.frameCounter_.framesPerSecond();
}

/**
 * Render the flock into the given canvas.
 * @param {!Canvas} canvas The target canvas.
 */
Flock.prototype.render = function(canvas) {
  if (!this.gooseImage_.complete) {
    return;
  }
  var context2d = canvas.getContext('2d');
  for (var g = 0; g < this.geese_.length; g++) {
    var goose = this.geese_[g];
    var heading = goose.velocity().heading();
    if (heading < 0.0) {
      heading = Math.PI * 2.0 + heading;
    }
    // The goose points down the positive x-axis when its heading is 0.
    //context2d.rotate(heading);
    var spriteWidth = this.gooseImage_.height;
    var spriteOffset = Math.floor(heading / this.GOOSE_HEADING_INCREMENT_) *
                      spriteWidth;
    context2d.drawImage(this.gooseImage_,
                        spriteOffset, 0, spriteWidth, this.gooseImage_.height,
                        goose.location().x, goose.location().y,
                        spriteWidth, this.gooseImage_.height);
  }
  context2d.fillStyle = 'blue';
  for (var a = 0; a < this.attractors.length; a++) {
    context2d.fillRect(this.attractors[a].x - 2, this.attractors[a].y - 2, 4, 4);
  }
}
