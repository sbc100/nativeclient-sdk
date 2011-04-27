// Copyright 2011 The Native Client SDK Authors.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/**
 * @fileoverview  The life Application object.  This object instantiates a
 * Trackball object and connects it to the element named |tumbler_content|.
 * It also conditionally embeds a debuggable module or a release module into
 * the |tumbler_content| element.
 */

// Requires life
// Requires uikit.Dragger

// The life namespace
var life = life || {};

/**
 * Constructor for the Application class.  Use the run() method to populate
 * the object with controllers and wire up the events.
 * @constructor
 */
life.Application = function() {
}

/**
 * The native module for the application.  This refers to the module loaded via
 * the <embed> tag.
 * @type {Element}
 * @private
 */
life.Application.prototype.module_ = null;

/**
 * The mouse-drag event object.
 * @type {tumbler.Dragger}
 * @private
 */
life.Application.prototype.dragger_ = null;

/**
 * The timer update interval object.
 * @type {tumbler.Dragger}
 * @private
 */
life.Application.prototype.updateInterval_ = null;

/**
 * Called by the module loading function once the module has been loaded. Wire
 * up a Dragger object to |this|.
 * @param {!Element} nativeModule The instance of the native module.
 * @param {?String} opt_contentDivName The id of a DOM element which captures
 *     the UI events.  If unspecified, defaults to DEFAULT_DIV_NAME.  The DOM
 *     element must exist.
 */
life.Application.prototype.moduleDidLoad =
    function(nativeModule, opt_contentDivName) {
  contentDivId = opt_contentDivName || life.Application.DEFAULT_DIV_NAME;
  var contentDiv = document.getElementById(contentDivId);
  this.module_ = nativeModule;
  this.dragger_ = new uikit.Dragger(contentDiv);
  this.dragger_.addDragListener(this);
  if (this.module_) {
    // Use a 10ms update interval to drive frame rate
    this.updateInterval_ = setInterval("life.application.update()", 10);
  }
}

/**
 * Called when the page is unloaded.
 */
life.Application.prototype.moduleDidUnload = function() {
  clearInterval(this.updateInterval_);
}

/**
 * Called from the interval timer.  This is a simple wrapper to call the
 * "update" method on the Life NaCl module.
 */
life.Application.prototype.update = function() {
  if (this.module_)
    this.module_.update();
}

/**
 * Add a simulation cell at a 2D point.
 * @param {!number} point_x The x-coordinate, relative to the origin of the
 *     enclosing element.
 * @param {!number} point_y The y-coordinate, relative to the origin of the
 *     enclosing element, y increases downwards.
 */
life.Application.prototype.AddCellAtPoint = function(point_x, point_y) {
  if (this.module_)
    this.module_.addCellAtPoint(point_x, point_y);
}

/**
 * Handle the drag START event: Drop a new life cell at the mouse location.
 * @param {!life.Application} view The view controller that called
 *     this method.
 * @param {!uikit.DragEvent} dragStartEvent The DRAG_START event that
 *     triggered this handler.
 */
life.Application.prototype.handleStartDrag =
    function(controller, dragStartEvent) {
  this.AddCellAtPoint(dragStartEvent.clientX, dragStartEvent.clientY);
}

/**
 * Handle the drag DRAG event: Drop a new life cell at the mouse location.
 * @param {!life.Application} view The view controller that called
 *     this method.
 * @param {!uikit.DragEvent} dragEvent The DRAG event that triggered
 *     this handler.
 */
life.Application.prototype.handleDrag = function(controller, dragEvent) {
  this.AddCellAtPoint(dragEvent.clientX, dragEvent.clientY);
}

/**
 * Handle the drag END event: This is a no-op.
 * @param {!life.Application} view The view controller that called
 *     this method.
 * @param {!uikit.DragEvent} dragEndEvent The DRAG_END event that
 *     triggered this handler.
 */
life.Application.prototype.handleEndDrag = function(controller, dragEndEvent) {
}

/**
 * The default name for the 3D content div element.
 * @type {string}
 */
life.Application.DEFAULT_DIV_NAME = 'tumbler_content';
