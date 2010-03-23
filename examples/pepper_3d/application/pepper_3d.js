// Copyright 2010 The Native Client SDK Authors.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/**
 * @fileoverview  The Pepper3D application object.  This object instantiates a
 * Trackball object and connects it to the element named |pepper_3d_content|.
 * It also conditionally embeds a debuggable module or a release module into
 * the |pepper_3d_content| element.
 */

// Requires pepper3d.Dragger
// Requires pepper3d.Trackball

/**
 * Constructor for the Application class.  Use the run() method to populate
 * the object with controllers and wire up the events.
 * @constructor
 */
pepper3d.Application = function() {
}

/**
 * The native module for the application.  This refers to the module loaded via
 * the <embed> tag.
 * @type {Element}
 * @private
 */
pepper3d.Application.prototype.module_ = null;

/**
 * The trackball object.
 * @type {pepper3d.Trackball}
 * @private
 */
pepper3d.Application.prototype.trackball_ = null;

/**
 * The mouse-drag event object.
 * @type {pepper3d.Dragger}
 * @private
 */
pepper3d.Application.prototype.dragger_ = null;

/**
 * A timer used to retry loading the native client module; the application
 * tries to reload the module every 100 msec.
 * @type {Number}
 * @private
 */
pepper3d.Application.prototype.loadTimer_ = null;

/**
 * Called by the module loading function once the module has been loaded.
 * @param {?Element} nativeModule The instance of the native module.
 */
pepper3d.Application.prototype.moduleDidLoad = function(nativeModule) {
  this.module_ = nativeModule;
  this.trackball_ = new pepper3d.Trackball();
  this.dragger_ = new pepper3d.Dragger(this.module_);
  this.dragger_.addDragListener(this.trackball_);
}

/**
 * The run() method starts and 'runs' the application.  The trackball object
 * is allocated and all the events get wired up.  Throws an error
 * if any of the required DOM elements are missing.
 */
pepper3d.Application.prototype.run = function(opt_contentDivName) {
  contentDivName = opt_contentDivName || pepper3d.Application.DEFAULT_DIV_NAME;
  var contentDiv = document.getElementById(contentDivName);
  if (!contentDiv) {
    alert('missing ' + contentDivName);
    throw new Error('Application.run(): missing element ' + "'" +
        contentDivName + "'");
  }
  // Conditionally load the debug or release version of the module.
  if (window.location.hash == '#debug') {
    contentDiv.innerHTML = '<embed id="'
                           + pepper3d.Application.PEPPER_MODULE_NAME + '" '
                           + 'type="pepper-application/x-nacl-srpc" '
                           + 'width="480" height="480" '
                           + 'dimensions="3" />'
  } else {
    contentDiv.innerHTML = '<embed id="'
                           + pepper3d.Application.PEPPER_MODULE_NAME + '" '
                           + 'src="pepper_3d.nexe" '
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
    module = document.getElementById(pepper3d.Application.PEPPER_MODULE_NAME);
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
pepper3d.Application.PEPPER_MODULE_NAME = 'pepper3d';

/**
 * The default name for the 3D content div element.
 * @type {string}
 */
pepper3d.Application.DEFAULT_DIV_NAME = 'pepper_3d_content';
