// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The speedometer object.  This draws an array of speedometers as adjacent
 * vertical bars.  The height of the bars is a percentage of the maximum
 * possible speed.
 */


goog.provide('Speedometer');
goog.provide('Speedometer.Attributes');

goog.require('goog.Disposable');
goog.require('goog.events');
goog.require('goog.events.EventTarget');

/**
 * Manages a map of individual speedometers.  Use addMeterWithName()
 * to add speedometer bars.
 * @constructor
 * @extends {goog.events.EventTarget}
 */
Speedometer = function() {
  goog.events.EventTarget.call(this);
};
goog.inherits(Speedometer, goog.events.EventTarget);

/**
 * The dictionary of individual meters.  These meters are drawn as a row of
 * vertical bars.
 * @type {Array.Object}
 * @private
 */
Speedometer.prototype.meters_ = [];

/**
 * The number of unique meters added to the speedometer.
 * @type {number}
 * @private
 */
Speedometer.prototype.meterCount_ = 0;

/**
 * The maximum speed that any meter can reach.  Meters that go past this
 * value are clipped.
 * @type {real}
 * @private
 */
Speedometer.prototype.maxSpeed_ = 1.0;

/**
 * Meter dictionary values.
 * @enum {string}
 */
Speedometer.Attributes = {
  DISPLAY_NAME: 'displayName',  // The name used to display the meter.
  NAME: 'name',  // The name of the meter.  Not optional.
  VALUE: 'value',  // The value of the meter.  Not optional.
  // The id of a DOM element that can display the meter's value as text.
  VALUE_LABEL: 'valueLabel'
};

/**
 * Meter colours.  Meters are coloured according to their value: red for
 * 25% of |maxSpeed_| and below, yellow for 25% - 50% of |maxSpeed_| and
 * green for 50% - 100%.
 * @enum {string}
 * @private
 */
Speedometer.prototype.MeterColours_ = {
  LOW_SPEED: 'red',  // Low speed is below 25% of |maxSpeed_|.
  MEDIUM_SPEED: 'yellow',  // 25% - 50%
  HIGH_SPEED: 'green'  // 50% - 100%
};

/**
 * Override of disposeInternal() to dispose of retained objects and unhook all
 * events.
 * @override
 */
Speedometer.prototype.disposeInternal = function() {
  this.domElement_ = null;
  Speedometer.superClass_.disposeInternal.call(this);
}

/**
 * Return the current maximum speed.
 * @return {real} The current maximum speed.  Guaranteed to be >= 0.0
 */
Speedometer.prototype.maximumSpeed = function() {
  return this.maxSpeed_;
}

/**
 * Set the maximum speed value for all meters.  The height of a meter's bar
 * is directly proportinal to meter.value / |maxSpeed_|.
 * @param {real} maxSpeed The new maximum speed.  Must be > 0.
 * @return {real} The previous maximum speed.
 */
Speedometer.prototype.setMaximumSpeed = function(maxSpeed) {
  if (maxSpeed <= 0.0) {
    throw Error('maxSpeed must be >= 0.0');
  }
  var oldMaxSpeed = this.maxSpeed_;
  this.maxSpeed_ = maxSpeed;
  return oldMaxSpeed;
}

/**
 * Add a named meter.  If a meter with |meterName| already exists, then do
 * nothing.
 * @param {!string} meterName The name for the new meter.
 * @param {?string} opt_displayDictionary A dictionary containing optional
 *     values for the meter.  Some key names are special, for example
 *     the 'valueLabel' key contains the id of a value label DOM element for
 *     the meter - this label will be updated with a test representation of the
 *     meter's value.
 */
Speedometer.prototype.addMeterWithName =
    function(meterName, opt_displayDictionary) {
  if (!(meterName in this.meters_)) {
    this.meterCount_++;
  }
  var meterDictionary = opt_displayDictionary || {};
  // Fill in the non-optional attributes.
  meterDictionary[Speedometer.Attributes.NAME] = meterName;
  if (!(Speedometer.Attributes.VALUE in meterDictionary)) {
    meterDictionary[Speedometer.Attributes.VALUE] = 0.0;
  }
  this.meters_[meterName] = meterDictionary;
}

/**
 * Update the value of a named meter.  If the meter does not exist, then do
 * nothing.  If the value is successfully changed, then post a VALUE_CHANGED
 * event.
 * @param {!string} meterName The name of the meter.
 * @param {!real} value The new value for the meter.  Must be >= 0
 * @return {real} The previous value of the meter.
 */
Speedometer.prototype.updateMeterNamed =
    function(meterName, value) {
  var oldValue = 0.0;
  if (meterName in this.meters_) {
    oldValue = this.meters_[meterName][Speedometer.Attributes.VALUE];
    this.meters_[meterName][Speedometer.Attributes.VALUE] = value;
  }
  return oldValue;
}

/**
 * Render the speedometer.  Draws each meter in turn, displaying their labels.
 * Meter width and height is adjusted to fit proportionally inside |context2d|.
 * If a meter has an attached value label, then its text is updated with the
 * new meter value as well.
 * @param {!Canvas} canvas The 2D canvas.
 */
Speedometer.prototype.render = function(canvas, opt_labelElements) {
  var meterWidth = canvas.width / this.meterCount_;
  var context2d = canvas.getContext('2d');
  context2d.save();
  // Paint the background image.
  context2d.fillStyle = 'red';
  context2d.fillRect(0, 0, canvas.width, canvas.height);
  // Paint the meters.
  context2d.fillStyle = 'green';
  // Change the coordinate system to y-positive going up.
  context2d.translate(0, context2d.canvas.height);
  context2d.scale(1, -1);
  var i = 0;
  for (meterName in this.meters_) {
    var meter = this.meters_[meterName]
    var meterHeight = (meter.value * context2d.canvas.height) /
                      this.maximumSpeed() ;
    context2d.fillRect(i * meterWidth, 0, meterWidth, meterHeight);
    i++;
    if (Speedometer.Attributes.VALUE_LABEL in meter) {
      var labelElement =
          document.getElementById(meter[Speedometer.Attributes.VALUE_LABEL]);
      if (labelElement) {
        labelElement.innerHTML = meter.value.toFixed(3);
      }
    }
  }
  context2d.restore();
}

