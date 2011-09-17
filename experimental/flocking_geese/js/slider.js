// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * A slider control.
 */

goog.provide('Slider');
goog.provide('Slider.Orientation');
goog.provide('Slider.SliderEvents');

goog.require('goog.Disposable');
goog.require('goog.dom');
goog.require('goog.events');
goog.require('goog.events.EventTarget');
goog.require('goog.events.EventType');
goog.require('goog.style');

/**
 * The slider control.  Wire up the slider events.
 * @param {!Element} container The DOM element containing the entire slider.
 * @param {!Element} ruler The DOM element containing the slider's ruler.
 * @param {!Element} thumb The DOM elelemnt representing the slider's thumb.
 * @param {!Array} stepObjects An array of objects, each object is associated
 *     with the step at the object's index in the array.  The object is
 *     returned by objectAtStepValue().
 * @param {?string} opt_orientation The orentation, default is HORIZONTAL.
 * @constructor
 * @extends {goog.EventTarget}
 */
Slider = function(container, ruler, thumb, stepObjects, opt_orientation) {
  goog.events.EventTarget.call(this);

  /**
   * Cached DOM elements for the slider parts.
   * @type {Element}
   * @private
   */
  this.container_ = container;  // The slider's outer container.
  this.ruler_ = ruler;  // The slider's ruler element.
  this.thumb_ = thumb;  // The slider's thumb.

  /**
   * The current value.  In a discreet slider, this has to be an even multiple
   * of |this.stepSize_|.
   */
  this.currentValue_ = 0;

  /**
   * The number of steps between the start and end point (not including the
   * end points).
   * @type {number}
   * @private
   */
  this.stepCount_ = stepObjects.length;

  /**
   * The step objects.
   * @type {Array}
   * @private
   */
  this.stepObjects_ = stepObjects;

  /**
   * The step size.  In a discreet slider, the thumb can only be an even
   * multiple of this value.  Measured in pixels.
   * @type {number}
   * @private
   */
  this.stepSize_ = 1;

  /**
   * The orientation.  'h' means horizontal (the default); 'v' means vertical.
   * @type {string}
   * @private
   */
  this.orientation_ = opt_orientation || Slider.Orientation.HORIZONTAL;

  /**
   * Indicates whether the thumb is being dragged.
   * @type {boolean}
   * @private
   */
  this.isDragging_ = false;

  /**
   * The length of the ruler.  This is sized to fit within the slider's
   * container.  Measured in pixels.
   * @type {number}
   * @private
   */
  this.rulerLength_ = 0;

  // Wire up the mouse event handlers.
  this.rulerClickListener_ = goog.events.listen(
      this.ruler_,
      goog.events.EventType.CLICK,
      this.rulerClick_, true, this);
  this.thumbMouseDownListener_ = goog.events.listen(
      this.thumb_,
      goog.events.EventType.MOUSEDOWN,
      this.thumbMouseDown_, true, this);
  this.thumbClickListener_ = goog.events.listen(
      this.thumb_,
      goog.events.EventType.CLICK,
      function(e) {
          e.stopPropagation();
      }, true, this);
  this.thumbMouseMoveListener_ = null;
  this.thumbMouseUpListener_ = null;

  // Complete initialization.
  this.updateSize_();
  this.slideToValue(this.currentValue_);
}
goog.inherits(Slider, goog.events.EventTarget);

/**
 * The slider's orientation: vertical or hoirizontal.
 * @enum {string}
 */
Slider.Orientation = {
  HORIZONTAL: 'h',
  VERTICAL: 'v'
}

/**
 * Override of disposeInternal() to dispose of retained objects and unhook all
 * events.
 * @override
 */
Slider.prototype.disposeInternal = function() {
  goog.events.removeAll(this.container_);
  goog.events.removeAll(this.ruler_);
  goog.events.removeAll(this.thumb_);
  this.container_ = null;
  this.ruler_ = null;
  this.thumb_ = null;
  Slider.superClass_.disposeInternal.call(this);
}

/**
 * The slider's continuous value.
 * @return {number} the value.
 */
Slider.prototype.value = function() {
  return this.currentValue_;
}

/**
 * The slider's discreet value as a step index.
 * @return {number} the value.
 */
Slider.prototype.stepValue = function() {
  return Math.round(this.currentValue_ / this.stepSize_);
}

/**
 * The object at a step value.
 * @param {number} opt_step The step index (0-based).  Default is to use the
 *     current step value.
 * @return {number} the value.
 */
Slider.prototype.objectAtStepValue = function(opt_step) {
  var stepValue = opt_step || this.stepValue();
  return this.stepObjects_[stepValue];
}

/**
 * Is the slider horizontal?
 * @return {boolean} |true| if the slider is horizontal.
 */
Slider.prototype.isHorizontal = function() {
  return this.orientation_ == Slider.Orientation.HORIZONTAL;
}

/**
 * Slide the thumb along the slider so that it represents |value|.  If the
 * slider has no length (its step count is 0), then do nothing.
 * @param {number} value The new value of the thumb.  Measured in pixels from
 *     the left edge of the slider's container.
 */
Slider.prototype.slideToValue = function(value) {
  if (this.stepCount_ == 0) {
    return;
  }

  // Validate and clip |value| to the range of the slider.
  if (value < 0) {
    value = 0;
  }
  if (value >= this.rulerLength_) {
    value = this.rulerLength_;
  }
  var stepValue = Math.round(value / this.stepSize_);
  value = stepValue * this.stepSize_;
  if (this.isHorizontal()) {
    this.thumb_.style.left = value + 'px';
  } else {
    this.thumb_.style.top = value + 'px';
  }
  this.currentValue_ = value;
}

/**
 * Handle a 'click' event on the ruler itself.  This slides the thumb to the
 * closets tick to where the mouse click happened.
 * @param {Event} clickEvent The event that triggered this handler.
 */
Slider.prototype.rulerClick_ = function(clickEvent) {
  if (!this.isDragging_) {
    this.slideToValue(this.rulerOffsetFromEvent_(clickEvent));
    this.dispatchEvent(new SliderEvent(this.value(), this.stepValue()));
  }
}

/**
 * Handle a mousedown on the thumb.  This starts the thumb-drag state.
 * @param {Event} mouseEvent The event that triggered this handler.
 */
Slider.prototype.thumbMouseDown_ = function(mouseEvent) {
  if (this.isDragging_) {
    return;
  }
  this.isDragging_ = true;
  // Listen for mouse-move and mouse-up events.  Dragging stops when the
  // mouse-up event arrives.
  if (this.thumbMouseMoveListener_ == null) {
    this.thumbMouseMoveListener_ = goog.events.listen(
        this.container_,
        goog.events.EventType.MOUSEMOVE,
        this.thumbMouseMove_, true, this);
  }
  if (this.thumbMouseUpListener_ == null) {
    this.thumbMouseUpListener_ = goog.events.listen(
        window,
        goog.events.EventType.MOUSEUP,
        this.thumbMouseUp_, true, this);
  }
}

/**
 * Handle a mousemove on the thumb.  Drag thte thumb to the new value.
 * @param {Event} mouseEvent The event that triggered this handler.
 */
Slider.prototype.thumbMouseMove_ = function(mouseEvent) {
  this.slideToValue(this.rulerOffsetFromEvent_(mouseEvent));
}

/**
 * Handle a mouseup on the thumb.  Leave the drag state, post a change event.
 * @param {Event} mouseEvent The event that triggered this handler.
 */
Slider.prototype.thumbMouseUp_ = function(mouseEvent) {
  if (this.thumbMouseMoveListener_ != null) {
    goog.events.unlistenByKey(this.thumbMouseMoveListener_);
    this.thumbMouseMoveListener_ = null;
  }
  if (this.thumbMouseUpListener_ != null) {
    goog.events.unlistenByKey(this.thumbMouseUpListener_);
    this.thumbMouseUpListener_ = null;
  }
  this.dispatchEvent(new SliderEvent(this.value(), this.stepValue()));
  this.isDragging_ = false;
}

/**
 * Compute the pixel offset of an event from the ruler's origin.
 * @param {BrowserEvent} event A BrowserEvent that has clientX and clientY.
 * @return {number} A one-dimensional pixel offset of the event.
 */
Slider.prototype.rulerOffsetFromEvent_ = function(event) {
  var rulerOffset = 0;
  if (this.isHorizontal()) {
    var rulerOrigin = goog.style.getPageOffsetLeft(this.ruler_);
    rulerOffset = event.clientX - rulerOrigin;
  } else {
    var rulerOrigin = goog.style.getPageOffsetTop(this.ruler_);
    rulerOffset = event.clientY - rulerOrigin;
  }
  return rulerOffset;
}

/**
 * Update the size of the slider so that it fits within the container.
 */
Slider.prototype.updateSize_ = function() {
  var rulerSize = goog.style.getSize(this.ruler_)
  if (this.isHorizontal()) {
    this.rulerLength_ = rulerSize.width;
  } else {
    this.rulerLength_ = rulerSize.height;
  }
  if (this.stepCount_ != 0) {
    this.stepSize_ = this.rulerLength_ / (this.stepCount_ - 1);
  }
  this.slideToValue(this.currentValue_);
};

