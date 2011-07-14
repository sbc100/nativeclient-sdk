// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview  Implement the mouse drag wrapper class, Dragger, that
 * produces simple mouse-dragged events.  These work like mouse-moved events,
 * only they are sent while a mouse button is down (that is, when the mouse is
 * being dragged).
 */

goog.provide('uikit.events.Dragger');

goog.require('goog.fx.Dragger');

/**
 * Constructor for the Dragger object.  This is a subclass of goog.fx.Dragger
 * that overrides defaultAction() to do nothing, so that the drag event is
 * simply sent onto its target, instead of moving its target.  Overriding the
 * defaultAction() method to do nothing implements a simple mouse-drag event.
 * @param {?Object} opt_element Generate mouse-drag events using this element.
 *     The default element is the document.
 * @constructor
 * @extends {goog.fx.Dragger}
 */
uikit.events.Dragger = function(opt_element) {
  goog.fx.Dragger.call(this, opt_element || document);
};
goog.inherits(uikit.events.Dragger, goog.fx.Dragger);

/**
 * Override defaultAction() to do nothing.  Instead, the target listens for
 * DRAG events and reacts to them by rolling the trackball.
 * @param {number} x X-coordinate for target element.
 * @param {number} y Y-coordinate for target element.
 * @override
 */
uikit.events.Dragger.prototype.defaultAction = function(x, y) {
  // Do nothing.  This overrides the default implementation that changes the
  // position style properties of the target.
};
