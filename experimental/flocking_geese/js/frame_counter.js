// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * A small helper class that keeps track of tick deltas and a tick
 * count.  It accumulates the duration of each frame.  Updates the frame rate
 * every 100 frames.
 */

goog.provide('FrameCounter');

goog.require('goog.Disposable');

/**
 * Constructor for the FrameCounter class.
 * @constructor
 * @extends {goog.Disposable}
 */
FrameCounter = function() {
  goog.Disposable.call(this);

  /**
   * The frame duration accumulator.  Holds the accumulated simulation time.
   * Measured in milliseconds.
   * @type {number}
   * @private
   */
  this.frameDurationAccumulator_ = 0;

  /**
   * The number of frames accumulated so far.  This is reset to 0 about once
   * a second.
   * @type {number}
   * @private
   */
  this.frameCount_ = 0;

  /**
   * The time stamp at the the start of a simulation tick.
   * @type {number}
   * @private
   */
  this.frameStart_ = 0.0;

  /**
   * The current frame rate.  This is updated about once per second, and is
   * not a valid value for the first second of the simulation..
   * @type {number}
   * @private
   */
  this.framesPerSecond_ = 0;
}
goog.inherits(FrameCounter, goog.Disposable);

/**
 * Number of clock ticks per second.
 * @type {number}
 * @private
 */
FrameCounter.prototype.CLOCK_TICKS_PER_SECOND_ = 1000.0;

/**
 * Frame count that triggers an update the frame rate.
 * @type {number}
 * @private
 */
FrameCounter.prototype.FRAME_RATE_REFRESH_COUNT_ = 100;

/**
 * Override of disposeInternal() to dispose of retained objects.
 * @override
 */
FrameCounter.prototype.disposeInternal = function() {
  FrameCounter.superClass_.disposeInternal.call(this);
}

/**
 * Record the current time, which is used to compute the frame duration
 * when endFrame() is called.
 */
FrameCounter.prototype.beginFrame = function() {
  this.frameStart_ = new Date().getTime();
}

/**
 * Compute the delta since the last call to beginFrame() and increment the
 * frame count.  Update the frame rate whenever the prescribed number of
 * frames have been counted.
 */
FrameCounter.prototype.endFrame = function() {
  var frameEnd = new Date().getTime();
  var dt = frameEnd - this.frameStart_;
  if (dt < 0) {
    return;  // Just in case.
  }
  this.frameDurationAccumulator_ += dt;
  this.frameCount_++;
  if (this.frameCount_ >= this.FRAME_RATE_REFRESH_COUNT_) {
    var elapsedTime = this.frameDurationAccumulator_ /
                      this.CLOCK_TICKS_PER_SECOND_;
    this.framesPerSecond_ = this.frameCount_ / elapsedTime;
    this.frameDurationAccumulator_ = 0;
    this.frameCount_ = 0;
  }
}

/**
 * The current frame rate.  Note that this is 0 for the first second in
 * the accumulator, and is updated about once per second.
 * @return {number} The current frame rate in frames per second.
 */
FrameCounter.prototype.framesPerSecond = function() {
  return this.framesPerSecond_;
}

