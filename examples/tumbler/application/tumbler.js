// Copyright 2010 The Native Client SDK Authors.
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
 * The run() method starts and 'runs' the application.  The trackball object
 * is allocated and all the events get wired up.  Throws an error
 * if any of the required DOM elements are missing.
 */
tumbler.Application.prototype.run = function(opt_contentDivName) {
  contentDivName = opt_contentDivName || tumbler.Application.DEFAULT_DIV_NAME;
  var contentDiv = document.getElementById(contentDivName);
  if (!contentDiv) {
    alert('missing ' + contentDivName);
    throw new Error('Application.run(): missing element ' + "'" +
        contentDivName + "'");
  }
  // Conditionally load the develop or publish version of the module.
  if (window.location.hash == '#develop') {
    contentDiv.innerHTML = '<embed id="'
                           + tumbler.Application.TUMBLER_MODULE_NAME + '" '
                           + 'type="pepper-application/tumbler" '
                           + 'width="480" height="480" '
                           + 'dimensions="3" />'
  } else {
    contentDiv.innerHTML = '<embed id="'
                           + tumbler.Application.TUMBLER_MODULE_NAME + '" '
                           + 'src="tumbler.nexe" '
                           + 'type="application/x-nacl-srpc" '
                           + 'width="480" height="480" '
                           + 'dimensions="3" />'
  }

  /**
   * Class function that loads the module.  If the module is not available,
   * this function starts a timer with itself as the argument.  As soon as the
   * module is loaded, it sends the OnModuleLoaded() message.
   */
  this.module_ = null;
  var thisref = this;
  LoadModule = function() {
    module = document.getElementById(tumbler.Application.TUMBLER_MODULE_NAME);
    if (module == undefined || module.setCameraOrientation == undefined) {
      // Recursive call to the LoadModule function.
      thisref.loadTimer_ = setTimeout(arguments.callee, 500);
    } else {
      clearTimeout(thisref.loadTimer_);
      thisref.moduleDidLoad(module);
    }
  }

  LoadModule();
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
