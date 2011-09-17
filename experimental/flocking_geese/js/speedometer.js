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
   * The images.
   * @type {Object.Image}
   * @private
   */
  this.images_ = {}
  this.loadImages_();
};
goog.inherits(Speedometer, goog.events.EventTarget);

/**
 * Meter dictionary values.
 * @enum {string}
 */
Speedometer.Attributes = {
  THEME: 'theme',  // The needle theme.  Has value Speedometer.Themes.
  DISPLAY_NAME: 'displayName',  // The name used to display the meter.
  NAME: 'name',  // The name of the meter.  Not optional.
  ODOMETER_LEFT: 'odometerLeft',  // The left coordinate of the odometer.
  ODOMETER_TOP: 'odometerTop',
  VALUE: 'value',  // The value of the meter.  Not optional.
  // The id of a DOM element that can display the meter's value as text.
  VALUE_LABEL: 'valueLabel'
};

/**
 * Drawing themes.
 * @enum {string}
 */
Speedometer.Themes = {
  DEFAULT: 'greenTheme',
  GREEN: 'greenTheme',
  RED: 'redTheme'
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
  NEEDLE_GRADIENT_START: '#7F7F7F',
  NEEDLE_STOPPED: '#999999',  // Colour of the needle at rest.
  NEEDLE_OUTLINE: 'white'
};

/**
 * Odometer dimensions.
 * @enum {number}
 * @private
 */
Speedometer.prototype.OdometerDims_ = {
  DIGIT_WIDTH: 12,
  DIGIT_HEIGHT: 14,
};

/**
 * Meter needles are inset from the meter radius by this amount.
 * @type {number}
 * @private
 */
Speedometer.prototype.NEEDLE_INSET_ = 15;

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
 * Maximum odometer value.  Speeds are clapmed to this.
 * @type {number}
 * @private
 */
Speedometer.prototype.MAX_ODOMETER_VALUE_ = 999999.99;

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
 * @param {?string} opt_attributes A dictionary containing optional attributes
 *     for the meter.  Some key names are special, for example the 'valueLabel'
 *     key contains the id of a value label DOM element for the meter - this
 *     label will be updated with a text representation of the meter's value.
 */
Speedometer.prototype.addMeterWithName = function(meterName, opt_attributes) {
  if (!(meterName in this.meters_)) {
    this.meterCount_++;
  }
  var meterDictionary = opt_attributes || {};
  // Fill in the non-optional attributes.
  meterDictionary[Speedometer.Attributes.NAME] = meterName;
  if (!(Speedometer.Attributes.VALUE in meterDictionary)) {
    meterDictionary.value = 0.0;
  }
  if (!(Speedometer.Attributes.THEME in meterDictionary)) {
    meterDictionary.theme = Speedometer.Themes.DEFAULT;
  }
  if (!(Speedometer.Attributes.ODOMETER_LEFT in meterDictionary)) {
    meterDictionary.odometerLeft = 0;
  }
  if (!(Speedometer.Attributes.ODOMETER_TOP in meterDictionary)) {
    meterDictionary.odometerTop = 0;
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

  context2d.save();
  // Paint the background image.
  if (this.images_.background.complete) {
    context2d.drawImage(this.images_.background, 0, 0);
  } else {
    context2d.fillStyle = 'white';
    context2d.fillRect(0, 0, canvas.width, canvas.height);
  }

  // Paint the meters.
  for (meterName in this.meters_) {
    var meter = this.meters_[meterName];
    this.drawNeedle_(context2d, meter);
    this.drawOdometer_(context2d, meter);
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

/**
 * Draw a meter's needle.
 * @param {Graphics2d} context2d The 2D canvas context.
 * @param {Object} meter The meter.
 * @private
 */
Speedometer.prototype.drawNeedle_ = function(context2d, meter) {
  var canvasCenterX = context2d.canvas.width / 2;
  var canvasCenterY = context2d.canvas.height / 2;
  var radius = Math.min(canvasCenterX, canvasCenterY) - this.NEEDLE_INSET_;
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
  // Select the needle image based on the meter's theme and value.
  var needleImage = null;
  if (meter.value == 0) {
    needleImage = this.images_.stoppedNeedle;
  } else if (meter.theme == Speedometer.Themes.GREEN) {
    needleImage = this.images_.greenNeedle;
  } else if (meter.theme == Speedometer.Themes.RED) {
    needleImage = this.images_.redNeedle;
  }
  if (needleImage && needleImage.complete) {
    context2d.drawImage(
        needleImage,
        0, 0, needleImage.width, needleImage.height,
        -12, -12, needleImage.width, needleImage.height);
  }
  context2d.restore();
}

/**
 * Draw the odometer for a given meter.
 * @param {Graphics2d} context2d The 2D canvas context.
 * @param {Object} meter The meter.
 * @private
 */
Speedometer.prototype.drawOdometer_ = function(context2d, meter) {
  if (!this.images_.odometerTenths.complete ||
      !this.images_.odometerDigits.complete) {
    return;
  }
  context2d.save();
  context2d.translate(meter.odometerLeft, meter.odometerTop);
  var odometerValue = Math.min(meter.value.toFixed(2),
                               this.MAX_ODOMETER_VALUE_);
  // Draw the tenths digit.
  var tenths = (odometerValue - Math.floor(odometerValue)) * 10;
  this.drawOdometerDigit_(context2d, tenths, 6, this.images_.odometerTenths);
  // Draw the integer part, up to 5 digits (max value is 999,999.9).
  for (var column = 5; column >= 0; column--) {
    var digitValue = odometerValue / 10;
    digitValue = digitValue - Math.floor(digitValue);
    digitValue *= 10;
    this.drawOdometerDigit_(context2d, digitValue,
                            column, this.images_.odometerDigits);
    odometerValue = odometerValue / 10;
  }
  context2d.restore();
}

/**
 * Draw a single odometer digit.  The digit source is a vertical strip of
 * pre-rendered numbers.  When drawing the last digit in the strip, the image
 * source is wrapped back to the beginning of the strip.  In this way, drawing
 * a partial '9' will also draw part of the '0'.
 * @param {Graphics2d} context2d The drawing context.
 * @param {number} value The value, must be in range [0..10).  The fractional
 *     part is used to 'roll' the digit image.
 * @param {number} column The odometer column index, 0-based numbered from the
 *     left-most (most significant) digit.  For example, column 0 is the
 *     100,000's place, column 6 is the 1/10's place.
 * @param {Image} digits The digit source image.
 * @private
 */
Speedometer.prototype.drawOdometerDigit_ = function(
    context2d, value, column, digits) {
  var digitOffset = value * this.OdometerDims_.DIGIT_HEIGHT;
  var digitHeight = Math.min(digits.height - digitOffset,
                             this.OdometerDims_.DIGIT_HEIGHT);
  var digitWrapHeight = this.OdometerDims_.DIGIT_HEIGHT - digitHeight;
  context2d.drawImage(digits,
                      0, digitOffset,
                      this.OdometerDims_.DIGIT_WIDTH, digitHeight,
                      column * this.OdometerDims_.DIGIT_WIDTH, 0,
                      this.OdometerDims_.DIGIT_WIDTH, digitHeight);
  if (digitWrapHeight > 0) {
    context2d.drawImage(digits,
                        0, 0,
                        this.OdometerDims_.DIGIT_WIDTH, digitWrapHeight,
                        column * this.OdometerDims_.DIGIT_WIDTH,
                        this.OdometerDims_.DIGIT_HEIGHT - digitWrapHeight,
                        this.OdometerDims_.DIGIT_WIDTH, digitWrapHeight);
  }
}

/**
 * Load all the images.
 * @private
 */
Speedometer.prototype.loadImages_ = function() {
  this.images_.background = new Image();
  this.images_.background.src = 'images/Dial_background.png';
  this.images_.odometerDigits = new Image();
  this.images_.odometerDigits.src = 'images/Numbers_Light.png';
  this.images_.odometerTenths = new Image();
  this.images_.odometerTenths.src = 'images/Numbers_Red.png';
  this.images_.stoppedNeedle = new Image();
  this.images_.stoppedNeedle.src = 'images/GreyPointer.png'
  this.images_.greenNeedle = new Image();
  this.images_.greenNeedle.src = 'images/GreenPointer.png'
  this.images_.redNeedle = new Image();
  this.images_.redNeedle.src = 'images/RedPointer.png'
}

