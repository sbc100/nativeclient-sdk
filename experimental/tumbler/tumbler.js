// Copyright 2011 The Native Client Authors.
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
 * The ids used for elements in the DOM.  The Tumlber Application expects these
 * elements to exist.
 * @enum {string}
 * @private
 */
tumbler.Application.DomIds_ = {
  MODULE: 'tumbler',  // The <embed> element representing the NaCl module
  VIEW: 'tumbler_view'  // The <div> containing the NaCl element.
}

/**
 * Called by the module loading function once the module has been loaded.
 * @param {?Element} nativeModule The instance of the native module.
 */
tumbler.Application.prototype.moduleDidLoad = function(nativeModule) {
  this.module_ = nativeModule;

  /**
   * Set the camera orientation property on the NaCl module.
   * @param {Array.<number>} orientation A 4-element array representing the
   *     camera orientation as a quaternion.
   */
  this.module_.setCameraOrientation = function(orientation) {
      var methodString = 'setCameraOrientation ' +
                         'orientation:' +
                         JSON.stringify(orientation);
      this.postMessage(methodString);
  }

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
 *     VIEW.  The DOM element must exist.
 */
tumbler.Application.prototype.run = function(opt_contentDivName) {
  contentDivName = opt_contentDivName || tumbler.Application.DomIds_.VIEW;
  var contentDiv = document.getElementById(contentDivName);
  this.assert(contentDiv, "Missing DOM element '" + contentDivName + "'");

  // Note that the <EMBED> element is wrapped inside a <DIV>, which has a 'load'
  // event listener attached.  This method is used instead of attaching the
  // 'load' event listener directly to the <EMBED> element to ensure that the
  // listener is active before the NaCl module 'load' event fires.
  contentDiv.addEventListener('load', function() {
      var module =
          document.getElementById(tumbler.Application.DomIds_.MODULE);
      tumbler.application.moduleDidLoad(module);
    }, true);

  // Load the published .nexe.  This includes the 'nacl' attribute which
  // shows how to load multi-architecture modules.  Each entry in the "nexes"
  // object in the  .nmf manifest file is a key-value pair: the key is the
  // runtime ('x86-32', 'x86-64', etc.); the value is a URL for the desired
  // NaCl module.  To load the debug versions of your .nexes, set the 'nacl'
  //  attribute to the _dbg.nmf version of the manifest file.
  contentDiv.innerHTML = '<embed id="'
                         + tumbler.Application.DomIds_.MODULE + '" '
                         + 'src=tumbler.nmf '
                         + 'type="application/x-nacl" '
                         + 'width="480" height="480" />'
}
