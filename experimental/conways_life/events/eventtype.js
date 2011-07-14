// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Definitions of common modeler events triggered by various
 * toolbars, keyboard shortcuts, and other interface elements.
 */

goog.provide('uikit.events.EventType');

/**
 * Event types triggered by the various user interface elements.
 * @enum {string}
 */
uikit.events.EventType = {
  ACTION: 'uikit_action',  // General UI action event.
  DRAG_START: 'drag_start',  // Start of a mouse-drag event sequence.
  DRAG: 'drag',  // Sent while the mouse is dragging.
  DRAG_END: 'drag_end',  // End of a mouse-drag sequence.
  ORBIT: 'orbit',  // Orbit tool events.
  LINE: 'line',  // Line tool events
  PUSH_PULL: 'pushpull'  // Push-pull tool events.
}
