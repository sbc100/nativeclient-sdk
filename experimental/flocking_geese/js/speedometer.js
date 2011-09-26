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
 * @param {Canvas} opt_canvas The <CANVAS> element to use for drawing.
 * @constructor
 * @extends {goog.events.EventTarget}
 */
Speedometer = function(opt_canvas) {
  goog.events.EventTarget.call(this);

  /**
   * The canvas.
   * @type {Canvas}
   * @private
   */
  this.canvas_ = opt_canvas || null;

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

  /**
   * Display timers.  There is a timer for updating the needles and one for
   * updating the odometers, these run on different frequencies.  These
   * timers are started with the first call to render().
   * @type {Timer}
   * @private
   */
  this.needleUpdateTimer_ = null;
  this.odometerUpdateTimer_ = null;

  /**
   * The images.
   * @type {Object.Image}
   * @private
   */
  this.images_ = {}
};
goog.inherits(Speedometer, goog.events.EventTarget);

/**
 * Meter dictionary values.
 * @enum {string}
 */
Speedometer.Attributes = {
  DISPLAY_VALUE: 'displayValue',
  THEME: 'theme',  // The needle theme.  Has value Speedometer.Themes.
  NAME: 'name',  // The name of the meter.  Not optional.
  ODOMETER_DISPLAY_VALUE: 'odometerDisplayValue',
  ODOMETER_LEFT: 'odometerLeft',  // The left coordinate of the odometer.
  ODOMETER_TOP: 'odometerTop',
  VALUE: 'value',  // The value of the meter.  Not optional.
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
 * Odometer dimensions.
 * @enum {number}
 * @private
 */
Speedometer.prototype.OdometerDims_ = {
  COLUMN_HEIGHT: 16,
  COLUMN_WIDTH: 12,
  DIGIT_HEIGHT: 12,
  DIGIT_WIDTH: 8
};

/**
 * The render animation interval time.  Measured in milliseconds.
 * @type {number}
 * @private
 */
Speedometer.prototype.RENDER_INTERVAL_ = 83.0;  // About 12 frames/sec.

/**
 * The needle update interval time.  Measured in milliseconds.
 * @type {number}
 * @private
 */
Speedometer.prototype.NEEDLE_UPDATE_INTERVAL_ = 218.0;

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
 * The odometer update interval time.  Measured in milliseconds.
 * @type {number}
 * @private
 */
Speedometer.prototype.ODOMETER_UPDATE_INTERVAL_ = 1000.0;

/**
 * The transition time for odometer digits.  Measured in milliseconds.
 * @type {number}
 * @private
 */
Speedometer.prototype.ODOMETER_TRANSITION_TIME_ = 218.0;

/**
 * Override of disposeInternal() to dispose of retained objects and unhook all
 * events.
 * @override
 */
Speedometer.prototype.disposeInternal = function() {
  function stopTimer(timer) {
    if (timer) {
      clearTimeout(timer);
    }
  }

  stopTimer(this.needleUpdateTimer_);
  this.needleUpdateTimer_ = null;
  stopTimer(this.odometerUpdateTimer_);
  this.odometerUpdateTimer_ = null;
  for (image in this.images_) {
    delete this.images_[image];
    this.images_[image] = null;
  }
  this.canvas_ = null;
  Speedometer.superClass_.disposeInternal.call(this);
}

/**
 * Set the canvas for drawing.
 * @param !{Canvas} canvas The <CANVAS> element for drawing.
 */
Speedometer.prototype.setCanvas = function(canvas) {
  this.canvas_ = canvas;
}

/**
 * Return the current maximum speed.
 * @return {real} The current maximum speed.  Guaranteed to be >= 0.0
 */
Speedometer.prototype.maximumSpeed = function() {
  return this.maxSpeed_;
}

/**
 * Set the maximum speed value for all meters.
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
 * @param {?Object} opt_attributes A dictionary containing optional attributes
 *     for the meter.  Some key names are special, for example the 'valueLabel'
 *     key contains the id of a value label DOM element for the meter - this
 *     label will be updated with a text representation of the meter's value.
 */
Speedometer.prototype.addMeterWithName = function(meterName, opt_attributes) {
  if (!(meterName in this.meters_)) {
    this.meterCount_++;
  }
  var attributes = opt_attributes || {};
  // Fill in the non-optional attributes.
  var meter = {
    name: meterName,
    value: 0.0,
    displayValue: 0.0,  // Updated every NEEDLE_UPDATE_INTERVAL_
    maxValue: 0.0,
    theme: Speedometer.Themes.DEFAULT,
    odometerLeft: 0,
    odometerTop: 0,
    odometerDisplayValue: 0.0,  // Updated every ODOMETER_UPDATE_INTERVAL_
    previousAngle: 0.0,
  };
  for (attrib in attributes) {
    // Overwrite meter defaults with passed-in values if present.  Otherwise,
    // add the passed-in element.
    meter[attrib] = attributes[attrib];
  }
  this.meters_[meterName] = meter;
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
    var meter = this.meters_[meterName];
    oldValue = meter.value;
    // Update the actual values.  Display values are updated by the update
    // interval handlers.
    meter.value = value;
    meter.maxValue = Math.max(meter.maxValue, value);
  }
  return oldValue;
}

/**
 * The value for a meter.  If the meter doesn't exist, or if the given key is
 * not valid, returns -1.
 * @param {!string} meterName The name of the meter.
 * @param {?string} opt_key The key name of the value.  Defaults to 'value'.
 * @return {number} The current meter's value or -1 if the meter doesn't exist.
 */
Speedometer.prototype.valueForMeterNamed = function(meterName, opt_key) {
  if (meterName in this.meters_) {
    var key = opt_key || Speedometer.Attributes.VALUE;
    var meter = this.meters_[meterName];
    if (key in meter) {
      return meter[key];
    }
  }
  return -1;
}

/**
 * Update the needles to their new values, and restart the needle update timer.
 */
Speedometer.prototype.updateNeedles = function() {
  for (meterName in this.meters_) {
    var meter = this.meters_[meterName];
    meter.displayValue = meter.value;
  }
  this.render();
  this.needleUpdateTimer_ = setTimeout(goog.bind(this.updateNeedles, this),
                                       this.NEEDLE_UPDATE_INTERVAL_);
}

/**
 * Update the odometers to their new values, and restart the odometer update
 * timer.
 */
Speedometer.prototype.updateOdometers = function() {
  for (meterName in this.meters_) {
    var meter = this.meters_[meterName];
    meter.odometerDisplayValue = meter.value;
  }
  this.render();
  this.needleUpdateTimer_ = setTimeout(goog.bind(this.updateOdometers, this),
                                       this.ODOMETER_UPDATE_INTERVAL_);
}

/**
 * Start the display timers and perform any other display initialization.
 */
Speedometer.prototype.start = function() {
  this.loadImages_();
  for (meterName in this.meters_) {
    var meter = this.meters_[meterName];
    meter.displayValue = meter.value;
    meter.odometerDisplayValue = meter.maxValue;
  }
  this.needleUpdateTimer_ = setTimeout(goog.bind(this.updateNeedles, this),
                                       this.NEEDLE_UPDATE_INTERVAL_);
  this.odometerUpdateTimer_ = setTimeout(goog.bind(this.updateOdometers, this),
                                         this.ODOMETER_UPDATE_INTERVAL_);
}

/**
 * Render the speedometer.
 */
Speedometer.prototype.render = function() {
  if (!this.canvas_) {
    return;
  }
  var context2d = this.canvas_.getContext('2d');
  context2d.save();
  this.drawBackground_(context2d, this.canvas_.width, this.canvas_.height);
  // Paint the meters.
  for (meterName in this.meters_) {
    var meter = this.meters_[meterName];
    this.drawNeedle_(context2d, meter);
    this.drawOdometer_(context2d, meter);
  }
  context2d.restore();
}

/**
 * Draw the background.
 * @param {Graphics2d} context2d The 2D canvas context.
 * @param {number} width The background width, measured in pixels.
 * @param {number} height The background height, measured in pixels.
 * @private
 */
Speedometer.prototype.drawBackground_ = function(context2d, width, height) {
  // Paint the background image.
  if (this.images_.background.complete) {
    context2d.drawImage(this.images_.background, 0, 0);
  } else {
    context2d.fillStyle = 'black';
    context2d.fillRect(0, 0, width, height);
  }
}

/**
 * Draw a meter's needle.
 * @param {Graphics2d} context2d The 2D canvas context.
 * @param {Object} meter The meter.
 * @private
 */
Speedometer.prototype.drawNeedle_ = function(context2d, meter) {
  var canvasCenterX = this.canvas_.width / 2;
  var canvasCenterY = this.canvas_.height / 2;
  var meterAngle = (meter.displayValue / this.maxSpeed_) *
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
  if (meter.displayValue == 0) {
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
  context2d.drawImage(this.images_.odometer, 0, 0);
  var odometerValue = Math.min(meter.odometerDisplayValue.toFixed(2),
                               this.MAX_ODOMETER_VALUE_);
  // Center the odometer digits in the odometer cells.
  context2d.save();
  context2d.translate(
      (this.OdometerDims_.COLUMN_WIDTH - this.OdometerDims_.DIGIT_WIDTH) / 2,
      (this.OdometerDims_.COLUMN_HEIGHT - this.OdometerDims_.DIGIT_HEIGHT) / 2);
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
  var digitOffset = Math.floor(value) * this.OdometerDims_.DIGIT_HEIGHT;
  var digitHeight = Math.min(digits.height - digitOffset,
                             this.OdometerDims_.DIGIT_HEIGHT);
  var digitWrapHeight = this.OdometerDims_.DIGIT_HEIGHT - digitHeight;
  context2d.drawImage(digits,
                      0, digitOffset,
                      this.OdometerDims_.DIGIT_WIDTH, digitHeight,
                      column * this.OdometerDims_.COLUMN_WIDTH, 0,
                      this.OdometerDims_.DIGIT_WIDTH, digitHeight);
  if (digitWrapHeight > 0) {
    context2d.drawImage(digits,
                        0, 0,
                        this.OdometerDims_.DIGIT_WIDTH, digitWrapHeight,
                        column * this.OdometerDims_.COLUMN_WIDTH,
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
  this.images_.background.src = 'images/DialFace.png';
  this.images_.odometer = new Image();
  this.images_.odometer.src = 'images/odometer.png';
  this.images_.odometerDigits = new Image();
  this.images_.odometerDigits.src = 'images/numbers_grey.png';
  this.images_.odometerTenths = new Image();
  this.images_.odometerTenths.src = 'images/numbers_red.png';
  this.images_.stoppedNeedle = new Image();
  this.images_.stoppedNeedle.src = 'images/pointer_grey.png'
  this.images_.greenNeedle = new Image();
  this.images_.greenNeedle.src = 'images/pointer_green.png'
  this.images_.redNeedle = new Image();
  this.images_.redNeedle.src = 'images/pointer_red.png'
}

