// Copyright 2011 The Native Client SDK Authors.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/**
 * @fileoverview  The tumbler Application object.  This object instantiates a
 * Trackball object and connects it to the element named |tumbler_content|.
 * It also conditionally embeds a debuggable module or a release module into
 * the |tumbler_content| element.
 */

// Requires tumbler
// Requires tumbler.Dragger
// Requires tumbler.Trackball

/**
 * Constructor for the Application class.  Use the run() method to populate
 * the object with controllers and wire up the events.
 * @constructor
 */
tumbler.Application = function() {
}

/**
 * The native module for the application.  This refers to the module loaded via
 * the <embed> tag.
 * @type {Element}
 * @private
 */
tumbler.Application.prototype.module_ = null;

/**
 * The trackball object.
 * @type {tumbler.Trackball}
 * @private
 */
tumbler.Application.prototype.trackball_ = null;

/**
 * The mouse-drag event object.
 * @type {tumbler.Dragger}
 * @private
 */
tumbler.Application.prototype.dragger_ = null;

/**
 * A timer used to retry loading the native client module; the application
 * tries to reload the module every 100 msec.
 * @type {Number}
 * @private
 */
tumbler.Application.prototype.loadTimer_ = null;

/**
 * Called by the module loading function once the module has been loaded.
 * @param {?Element} nativeModule The instance of the native module.
 */
tumbler.Application.prototype.moduleDidLoad = function(nativeModule) {
  this.module_ = nativeModule;
  this.trackball_ = new tumbler.Trackball();
  this.dragger_ = new tumbler.Dragger(this.module_);
  this.dragger_.addDragListener(this.trackball_);
}

/**
 * Asserts that cond is true; issues an alert and throws an Error otherwise.
 * @param {bool} cond The condition.
 * @param {String} message The error message issued if cond is false.
 */
tumbler.Application.prototype.assert = function(cond, message) {
  if (!cond) {
    message = "Assertion failed: " + message;
    alert(message);
    throw new Error(message);
  }
}

/**
 * The run() method starts and 'runs' the application.  The trackball object
 * is allocated and all the events get wired up.
 * @param {?String} opt_contentDivName The id of a DOM element in which to
 *     embed the Native Client module.  If unspecified, defaults to
 *     DEFAULT_DIV_NAME.  The DOM element must exist.
 */
tumbler.Application.prototype.run = function(opt_contentDivName) {
  contentDivName = opt_contentDivName || tumbler.Application.DEFAULT_DIV_NAME;
  var contentDiv = document.getElementById(contentDivName);
  this.assert(contentDiv, "Missing DOM element '" + contentDivName + "'");
  // Load the published .nexe.  This includes the 'nacl' attribute which
  // shows how to load multi-architecture modules.  Each entry in the "nexes"
  // object in the  .nmf manifest file is a key-value pair: the key is the
  // runtime ('x86-32', 'x86-64', etc.); the value is a URL for the desired
  // NaCl module.  To load the debug versions of your .nexes, set the 'nacl'
  //  attribute to the _dbg.nmf version of the manifest file.
  contentDiv.innerHTML = '<embed id="'
                         + tumbler.Application.TUMBLER_MODULE_NAME + '" '
                         + 'nacl=tumbler.nmf '
                         + 'type="application/x-nacl" '
                         + 'width="480" height="480" '
                         + 'onload="moduleDidLoad()" />'
}

/**
 * The name for the pepper module element.
 * @type {string}
 */
tumbler.Application.TUMBLER_MODULE_NAME = 'tumbler';

/**
 * The default name for the 3D content div element.
 * @type {string}
 */
tumbler.Application.DEFAULT_DIV_NAME = 'tumbler_content';
