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
  /**
   * The dictionary of individual meters.
   * @type {Array.Object}
   * @private
   */
  this.meters_ = [];

  /**
   * The number of unique meters added to the speedometer.
   * @type {number}
   * @private
   */
  this.meterCount_ = 0;

  /**
   * The maximum speed that any meter can reach.  Meters that go past this
   * value are clipped.
   * @type {real}
   * @private
   */
  this.maxSpeed_ = 1.0;
  this.logMaxSpeed_ = 0.0;  // log(maxSpeed_).
  this.logScale_ = Math.log(2.0);  // The log scaling factor.

  /**
   * The background image.
   * @type {Image}
   * @private
   */
  this.dialImage_ = new Image();
  this.dialImage_.src = 'images/Dial_background.png';
};
goog.inherits(Speedometer, goog.events.EventTarget);

/**
 * Meter dictionary values.
 * @enum {string}
 */
Speedometer.Attributes = {
  COLOR: 'color',  // The needle color.
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
 * @type {string}
 * @private
 */
Speedometer.prototype.MeterColours_ = {
  NEEDLE_DEFAULT: 'darkgray',  // Default needle colour.
  NEEDLE_GRADIENT_START: 'darkgray',
  NEEDLE_0_SPEED: 'darkgray',  // Colour of the needle at rest.
  NEEDLE_OUTLINE: 'lightgray'
};

/**
 * Meter needles are inset from the meter radius by this amount.
 * @type {number}
 * @private
 */
Speedometer.prototype.NEEDLE_INSET_ = 30;

/**
 * Beginning and ending angles for the meter.  |METER_ANGLE_START_| represents
 * "speed" of 0, |METER_ANGLE_RANGE_| is the angular range of the meter needle.
 * maximumSpeed() is |METER_ANGLE_START_ + METER_ANGLE_RANGE_|.  Measured
 * cockwise in radians from the line y = 0.
 * @type {number}
 * @private
 */
Speedometer.prototype.METER_ANGLE_START_ = 3.0 * Math.PI / 4.0;
Speedometer.prototype.METER_ANGLE_RANGE_ = 6.0 * Math.PI / 4.0;

/**
 * Value used to provde damping to the meter.  The meter reading can change by
 * at most this amount per frame.  Measured in radians.
 * @type {number}
 * @private
 */
Speedometer.prototype.DAMPING_FACTOR_ = Math.PI / 36.0;

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
  this.logMaxSpeed_ = Math.log(this.maxSpeed_) / this.logScale_;
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
  if (!(Speedometer.Attributes.COLOR in meterDictionary)) {
    meterDictionary[Speedometer.Attributes.COLOR] =
        this.MeterColours_.NEEDLE_DEFAULT;
  }
  meterDictionary.previousAngle = 0.0;
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
    oldValue = this.meters_[meterName].value;
    this.meters_[meterName].value = value;
  }
  return oldValue;
}

/**
 * Render the speedometer.  Draws each meter in turn, displaying their labels.
 * The meter value is displayed on a log scale.
 * @param {!Canvas} canvas The 2D canvas.
 */
Speedometer.prototype.render = function(canvas, opt_labelElements) {
  var context2d = canvas.getContext('2d');

  var canvasCenterX = canvas.width / 2;
  var canvasCenterY = canvas.height / 2;
  var radius = Math.min(canvasCenterX, canvasCenterY) - this.NEEDLE_INSET_;

  context2d.save();
  // Paint the background image.
  if (this.dialImage_.complete) {
    context2d.drawImage(this.dialImage_, 0, 0);
  } else {
    context2d.fillStyle = 'white';
    context2d.fillRect(0, 0, canvas.width, canvas.height);
  }

  // Paint the meters.
  for (meterName in this.meters_) {
    var meter = this.meters_[meterName]
    var logMeterValue = Math.log(meter.value) / this.logScale_;
    var meterAngle = (logMeterValue / this.logMaxSpeed_) *
                     this.METER_ANGLE_RANGE_;
    meterAngle = Math.min(meterAngle, this.METER_ANGLE_RANGE_);
    meterAngle = Math.max(meterAngle, 0.0);
    // Dampen the meter's angular change.
    var delta = meterAngle - meter.previousAngle;
    if (Math.abs(delta) > this.DAMPING_FACTOR_) {
      delta = delta < 0 ? -this.DAMPING_FACTOR_ : this.DAMPING_FACTOR_;
    }
    var dampedAngle = meter.previousAngle + delta;
    meter.previousAngle = dampedAngle;
    dampedAngle += this.METER_ANGLE_START_;
    context2d.save();
      context2d.translate(canvasCenterX, canvasCenterY);
      context2d.rotate(dampedAngle);
      if (meter.value == 0.0) {
        // Use the 0-speed colour for the needle.
        context2d.fillStyle = this.MeterColours_.NEEDLE_0_SPEED;
      } else {
        // Use a gradient to fill the needle.
        var needleGradient = context2d.createLinearGradient(0, 0, radius, 0);
        needleGradient.addColorStop(
            0, this.MeterColours_.NEEDLE_GRADIENT_START);
        needleGradient.addColorStop(1, meter[Speedometer.Attributes.COLOR]);
        context2d.fillStyle = needleGradient;
      }
      context2d.beginPath();
      // The meter needle points down the positive x-axis when the angle is 0.
      context2d.moveTo(radius, 0);
      context2d.lineTo(5, 5);
      context2d.arc(5, 0, 5, Math.PI / 2, 3 * Math.PI / 2, false);
      context2d.closePath();
      context2d.fill();
      context2d.strokeStyle = this.MeterColours_.NEEDLE_OUTLINE;
      context2d.lineWidth = 1;
      context2d.stroke();
    context2d.restore();
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

