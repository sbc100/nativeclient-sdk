// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The event dispatched by a Slider.
 */

goog.provide('SliderEvent');

goog.require('goog.events');
goog.require('goog.events.Event');

/**
 * A slider event. Contains the slider value and stepValue.
 * @param {!number} value The slider's value (measured in pixels).
 * @param {!number} stepValue The corresponding step value of the slider.
 * @param {?Element} opt_target Reference to the object that is the target of
 *     this event. It has to implement the {@code EventTarget} interface
 *     declared at {@link http://developer.mozilla.org/en/DOM/EventTarget}.
 * @constructor
 * @extends {goog.Event}
 */
SliderEvent = function(value, stepValue, opt_target) {
  goog.events.Event.call(this, SliderEvent.EventType.CHANGE, opt_target);

  /**
   * Cached properties.
   * @type {number}
   */
  this.value = value;
  this.stepValue = stepValue;
}
goog.inherits(SliderEvent, goog.events.Event);

/**
 * The ids used for slider event types.
 * @enum {string}
 */
SliderEvent.EventType = {
  CHANGE: 'sliderchanged'  // The slider's value changed.
};

/**
 * Override of disposeInternal() to dispose of retained objects and unhook all
 * events.
 * @override
 */
SliderEvent.prototype.disposeInternal = function() {
  SliderEvent.superClass_.disposeInternal.call(this);
}

