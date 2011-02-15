// Copyright 2010 The Ginsu Authors.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/**
 * @fileoverview  The ginsu Application object.  This object instantiates a
 * Trackball object and connects it to the element named |ginsu_instance|.
 */

goog.provide('ginsu.Application');

goog.require('goog.Disposable');
goog.require('goog.style')

goog.require('ginsu.controllers.KeyboardHandler');
goog.require('ginsu.controllers.Toolbar');
goog.require('ginsu.controllers.ToolManager');
goog.require('ginsu.controllers.ViewController');
goog.require('ginsu.events.Event');
goog.require('ginsu.events.EventType');
goog.require('ginsu.tools.Tool');
goog.require('ginsu.tools.OrbitTool');
goog.require('ginsu.tools.LineTool');

/**
 * Constructor for the Application class.  Use the run() method to populate
 * the object with controllers and wire up the events.
 * @constructor
 * @extends {goog.Disposable}
 */
ginsu.Application = function() {
  goog.Disposable.call(this);
}
goog.inherits(ginsu.Application, goog.Disposable);

/**
 * The tool manager for the application.  This object keeps track of all the
 * tools, and which one is currently active.
 * @type {ginsu.ToolManager}
 * @private
 */
ginsu.Application.prototype.toolManager_ = null;

/**
 * The toolbar for the application.  This object is just a UI element that
 * selects a tool.  Connects to element with id
 * ginsu.Application.DomIds_.TOOLBAR.
 * @type {ginsu.view.Toolbar}
 * @private
 */
ginsu.Application.prototype.toolBar_ = null;

/**
 * The keyboard shortcut handler for the application.  A UI element that
 * listens for keyboard input and transforms that into ACTION events.
 * @type {ginsu.KeyboardHandle}
 * @private
 */
ginsu.Application.prototype.keyboardHandler_ = null;

/**
 * The view controller for the application.  A DOM element that encapsulates
 * the grayskull plugin; this is allocated at run time.  Connects to the
 * element with id ginsu.Application.DomIds_.VIEW.
 * @type {ginsu.ViewController}
 * @private
 */
ginsu.Application.prototype.viewController_ = null;

/**
 * The ids used for elements in the DOM.  The GSApplication expects these
 * elements to exist.
 * @enum {string}
 * @private
 */
ginsu.Application.DomIds_ = {
  TOOLBAR: 'ginsu_toolbar',  // The toolbar element id.
  VIEW: 'ginsu_view',  // The <div> containing the NaCl element.
  MODULE: 'ginsu_module'  // The <embed> element representing the NaCl module.
};

/**
 * The images used to render the tool buttons.
 * @enum {string}
 * @private
 */
ginsu.Application.ToolImages_ = {
  LINE: 'icons/line.png',  // The line tool.
  ORBIT: 'icons/orbit.png',  // The orbit tool.
  PUSH_PULL: 'icons/pushpull.png'  // The push/pull tool.
};

/**
 * @desc Label for the Line tool, used for both the 'alt' and 'title'
 *     properties.
 * @private
 */
ginsu.Application.MSG_LINE_ = goog.getMsg('Line');

/**
 * @desc Label for the Orbit tool, used for both the 'alt' and 'title'
 *     properties.
 * @private
 */
ginsu.Application.MSG_ORBIT_ = goog.getMsg('Orbit');

/**
 * @desc Label for the Push/Pull tool, used for both the 'alt' and 'title'
 *     properties.
 * @private
 */
ginsu.Application.MSG_PUSH_PULL_ = goog.getMsg('Push/Pull');

/**
 * Error messages.
 * @type {string}
 * @private
 */
ginsu.Application.MSG_MISSING_ELEMENT_ = goog.getMsg('missing element');

/**
 * Place-holder to make the onload event handling process all work.
 */
var loadingGinsuApp_ = {};

/**
 * Override of disposeInternal() to dispose of retained objects.
 * @override
 */
ginsu.Application.prototype.disposeInternal = function() {
  this.terminate();
  ginsu.Application.superClass_.disposeInternal.call(this);
}

/**
 * Helper function to allocate the tools and build a map that contains things
 * like the DOM content used to render the tool's button and the tool's
 * keyboard shortcut.
 * @private
 * @return {Array.<Object>} The tool map that can be used later to register the
 *     tools with the various GUI controllers.
 */
ginsu.Application.prototype.createToolMap_ = function() {

  /**
   * Helper function to create a tool map entry.
   * @param {!ginsu.Tool} tool The instance of a tool for this entry.
   * @param {!goog.ui.ControlContent} content The DOM content used to display
   *     the tool's button.
   * @param {string} shortcut The keyboard shortcut used to activate the tool.
   * @return {Object} An entry into the tool map array.
   */
  function createToolEntry(tool, content, shortcut) {
    return toolEntry = {'tool': tool, 'content': content, 'shortcut': shortcut};
  };

  /**
   * Build a map of the tools.  This is an array of objects that contains an
   * instance of the tool, the DOM content used to display the tool's button,
   * and the tool's keyboard shortcut.
   * Note: add new tools here; they will be picked up in the following
   * forEach() loop.
   * @type {Array.<Object>}
   */
  var toolMap = [
    createToolEntry(new ginsu.tools.LineTool(),
        goog.dom.createDom('img', {
            'src': ginsu.Application.ToolImages_.LINE,
            'alt': ginsu.Application.MSG_LINE_,
            'title': ginsu.Application.MSG_LINE_}),
        'l'),
    createToolEntry(new ginsu.tools.OrbitTool(),
        goog.dom.createDom('img', {
            'src': ginsu.Application.ToolImages_.ORBIT,
            'alt': ginsu.Application.MSG_ORBIT_,
            'title': ginsu.Application.MSG_ORBIT_}),
        'o'),
    createToolEntry(new ginsu.tools.Tool(ginsu.events.EventType.PUSH_PULL),
        goog.dom.createDom('img', {
            'src': ginsu.Application.ToolImages_.PUSH_PULL,
            'alt': ginsu.Application.MSG_PUSH_PULL_,
            'title': ginsu.Application.MSG_PUSH_PULL_}),
        'p')
  ];
  return toolMap;
};

/**
 * Called by the module loading function once the module has been loaded.
 * At this point, the Ginsu NaCL module is known to be loaded, so now is the
 * time to wire up all the UI elements and create the controller objects.
 * @param {?Element} nativeModule The instance of the native module.
 */
ginsu.Application.prototype.moduleDidLoad = function(nativeModule) {
  // First make sure that all the necessary DOM elements are in place.  If any
  // are missing, throw an error and exit.
  var toolBarElement = goog.dom.$(ginsu.Application.DomIds_.TOOLBAR);
  if (!toolBarElement) {
    // TODO(dspringer): we need a GSError object.
    throw new Error('Application.moduleDidLoad(): ' +
        ginsu.Application.MSG_MISSING_ELEMENT_ +
        "'" + ginsu.Application.DomIds_.TOOLBAR + "'");
  }

  // Allocate an instance of the various controllers and register all the tools.
  this.toolManager_ = new ginsu.controllers.ToolManager();
  this.toolBar_ = new ginsu.controllers.Toolbar();
  this.keyboardHandler_ = new ginsu.controllers.KeyboardHandler();
  var toolMap = this.createToolMap_();

  // Listen for 'unload' in order to terminate cleanly.
  goog.events.listen(window, goog.events.EventType.UNLOAD, this.terminate);

  // Add all the tools to the various UI elements and other controllers.
  goog.array.forEach(toolMap, function(toolEntry) {
      var toolId = toolEntry.tool.toolId();
      this.toolBar_.addToggleButton(toolEntry.content, toolId);
      this.toolManager_.addTool(toolEntry.tool);
      this.keyboardHandler_.registerShortcut(toolId, toolEntry.shortcut);
    }, this);

  // Put the toolbar into the document.
  this.toolBar_.render(toolBarElement, true);

  // Set up the plugin wrapper.
  this.viewController_ = new ginsu.controllers.ViewController(nativeModule);

  // Set up the event listeners.  First, tell the tool manager to observe
  // the drag events coming from the grayskull view container.
  goog.events.listen(this.viewController_, ginsu.events.EventType.DRAG_START,
      this.toolManager_.handleDragStartEvent, false, this.toolManager_);
  goog.events.listen(this.viewController_, ginsu.events.EventType.DRAG,
      this.toolManager_.handleDragEvent, false, this.toolManager_);
  goog.events.listen(this.viewController_, ginsu.events.EventType.DRAG_END,
      this.toolManager_.handleDragEndEvent, false, this.toolManager_);

  // Tell the tool manager to observe the various ACTION events.
  goog.events.listen(this.toolBar_, ginsu.events.EventType.ACTION,
      this.toolManager_.handleActionEvent, false, this.toolManager_);
  goog.events.listen(this.keyboardHandler_, ginsu.events.EventType.ACTION,
      this.toolManager_.handleActionEvent, false, this.toolManager_);
  goog.events.listen(this.viewController_, ginsu.events.EventType.ACTION,
      this.toolManager_.handleActionEvent, false, this.toolManager_);

  // Tell the toolbar to observe the various ACTION events.
  goog.events.listen(this.toolManager_, ginsu.events.EventType.ACTION,
      this.toolBar_.handleActionEvent, false, this.toolBar_);
  goog.events.listen(this.keyboardHandler_, ginsu.events.EventType.ACTION,
      this.toolBar_.handleActionEvent, false, this.toolBar_);
  goog.events.listen(this.viewController_, ginsu.events.EventType.ACTION,
      this.toolBar_.handleActionEvent, false, this.toolBar_);

  // Activate the orbit tool as the default.
  this.toolManager_.activateToolWithId(ginsu.events.EventType.ORBIT);
}

/**
 * Asserts that cond is true; issues an alert and throws an Error otherwise.
 * @param {bool} cond The condition.
 * @param {String} message The error message issued if cond is false.
 */
ginsu.Application.prototype.assert = function(cond, message) {
  if (!cond) {
    message = "Assertion failed: " + message;
    alert(message);
    throw new Error(message);
  }
}

/**
 * The run() method starts and 'runs' the application.  An <embed> tag is
 * injected into the <div> element |opt_viewDivName| which causes the Ginsu NaCl
 * module to be loaded.  Once loaded, the moduleDidLoad() method is called.
 * @param {?String} opt_viewDivName The id of a DOM element in which to
 *     embed the Ginsu module.  If unspecified, defaults to DomIds_.VIEW.  The
 *     DOM element must exist.
 */
ginsu.Application.prototype.run = function(opt_viewDivName) {
  var viewDivName = opt_viewDivName || ginsu.Application.DomIds_.VIEW;
  var viewDiv = goog.dom.$(viewDivName);
  this.assert(viewDiv, "Missing DOM element '" + viewDivName + "'");
  // Load the published .nexe.  This includes the 'nexes' attribute which
  // shows how to load multi-architecture modules.  Each entry in the
  // table is a key-value pair: the key is the runtime ('x86-32',
  // 'x86-64', etc.); the value is a URL for the desired NaCl module.
  var nexes = 'x86-32: ginsu_x86_32.nexe\n'
              + 'x86-64: ginsu_x86_64.nexe\n'
              + 'ARM: ginsu_arm.nexe ';
  // This assumes that the <div> containers for Ginsu modules each have a
  // unique name on the page.
  var uniqueModuleName = viewDivName + ginsu.Application.DomIds_.MODULE;
  // This is a bit of a hack: when the |onload| event fires, |this| is set to
  // the DOM window object, *not* the <embed> element.  So, we keep a global
  // pointer to |this| because there is no way to make a closure here. See
  // http://code.google.com/p/nativeclient/issues/detail?id=693
  loadingGinsuApp_[uniqueModuleName] = this;
  var onLoadJS = "loadingGinsuApp_['"
               + uniqueModuleName
               + "'].moduleDidLoad(document.getElementById('"
               + uniqueModuleName
               + "'));"
  var viewSize = goog.style.getSize(viewDiv);
  viewDiv.innerHTML = '<embed id="' + uniqueModuleName + '" '
                       + 'class="ginsu_autosize_view" '
                    // + 'nexes="' + nexes + '" '
                       + 'type="application/x-nacl-srpc" '
                       + 'width="' + viewSize.width + '" '
                       + 'height="' + viewSize.height + '" '
                       + 'dimensions="3" '
                       + 'onload="' + onLoadJS + '" />'
  // Note: this code is here to work around a bug in the Chrome Browser.
  // http://code.google.com/p/nativeclient/issues/detail?id=500
  goog.dom.$(uniqueModuleName).nexes = nexes;
}

/**
 * Shut down the application instance.  This unhooks all the event listeners
 * and deletes the objects created in moduleDidLoad().
 */
ginsu.Application.prototype.terminate = function() {
  goog.events.removeAll();
  this.toolManager_ = null;
  this.toolBar_ = null;
  this.keyboardHandler_ = null;
  this.viewController_ = null;
}
