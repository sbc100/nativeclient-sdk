// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The life Application object.  This object instantiates a Dragger object and
 * connects it to the element named @a life_module.
 */

goog.provide('life.Application');

goog.require('goog.Disposable');
goog.require('goog.array');
goog.require('goog.events.EventType');
goog.require('goog.style');

goog.require('life.controllers.ViewController');
goog.require('stamp.StampPanel');

/**
 * Constructor for the Application class.  Use the run() method to populate
 * the object with controllers and wire up the events.
 * @constructor
 * @extends {goog.Disposable}
 */
life.Application = function() {
  goog.Disposable.call(this);
}
goog.inherits(life.Application, goog.Disposable);

/**
 * The view controller for the application.  A DOM element that encapsulates
 * the grayskull plugin; this is allocated at run time.  Connects to the
 * element with id life.Application.DomIds_.VIEW.
 * @type {life.ViewController}
 * @private
 */
life.Application.prototype.viewController_ = null;

/**
 * The automaton rule string.  It is expressed as SSS/BB, where S is the
 * neighbour count that keeps a cell alive, and B is the count that causes a
 * cell to become alive.  See the .LIF 1.05 section in
 * http://psoup.math.wisc.edu/mcell/ca_files_formats.html for more info.
 * @default 23/3 the "Normal" Conway rules.
 * @type {Object.<Array>}
 * @private
 */
life.Application.prototype.automatonRules_ = {
  birthRule: [3],
  keepAliveRule: [2, 3]
};

/**
 * The id of the current stamp.  Defaults to the DEFAULT_STAMP_ID.
 * @type {string}
 * @private
 */
life.Application.prototype.currentStampId_ =
    life.controllers.ViewController.DEFAULT_STAMP_ID;

/**
 * The ids used for elements in the DOM.  The Life Application expects these
 * elements to exist.
 * @enum {string}
 * @private
 */
life.Application.DomIds_ = {
  BIRTH_FIELD: 'birth_field',  // Text field with the birth rule string.
  CLEAR_BUTTON: 'clear_button',  // The clear button element.
  KEEP_ALIVE_FIELD: 'keep_alive_field',  // Keep alive rule string.
  MODULE: 'life_module',  // The <embed> element representing the NaCl module.
  PLAY_MODE_SELECT: 'play_mode_select',  // The <select> element for play mode.
  PLAY_BUTTON: 'play_button',  // The play button element.
  SOUND_SELECT: 'sound_select',  // The <select> element for the stamp sound.
  VIEW: 'life_view'  // The <div> containing the NaCl element.
};

/**
 * The Run/Stop button attribute labels.  These are used to determine the state
 * and label of the button.
 * @enum {string}
 * @private
 */
life.Application.PlayButtonAttributes_ = {
  ALT_IMAGE: 'altimage',  // Image to display in the "on" state.
  STATE: 'state'  // The button's state.
};

/**
 * Override of disposeInternal() to dispose of retained objects.
 * @override
 */
life.Application.prototype.disposeInternal = function() {
  this.terminate();
  life.Application.superClass_.disposeInternal.call(this);
}

/**
 * Called by the module loading function once the module has been loaded. Wire
 * up a Dragger object to @a this.
 * @param {?String} opt_naclModuleId The id of an <EMBED> element which
 *     contains the NaCl module.  If unspecified, defaults to DomIds_.MODULE.
 *     If the DOM element doesn't exist, the program asserts and exits.
 */
life.Application.prototype.moduleDidLoad = function(opt_naclModuleId) {
  // Listen for 'unload' in order to terminate cleanly.
  goog.events.listen(window, goog.events.EventType.UNLOAD, this.terminate);

  // Set up the stamp editor.
  var stampEditorButton =
      document.getElementById(stamp.StampPanel.DomIds.STAMP_EDITOR_BUTTON);
  this.stampPanel_ = new stamp.StampPanel(stampEditorButton);
  var stampEditorButtons = {
    mainPanel:
        document.getElementById(stamp.StampPanel.DomIds.STAMP_EDITOR_PANEL),
    editorContainer:
        document.getElementById(stamp.StampPanel.DomIds.STAMP_EDITOR_CONTAINER),
    addColumnButton:
        document.getElementById(stamp.StampPanel.DomIds.ADD_COLUMN_BUTTON),
    removeColumnButton:
        document.getElementById(stamp.StampPanel.DomIds.REMOVE_COLUMN_BUTTON),
    addRowButton:
        document.getElementById(stamp.StampPanel.DomIds.ADD_ROW_BUTTON),
    removeRowButton:
        document.getElementById(stamp.StampPanel.DomIds.REMOVE_ROW_BUTTON),
    cancelButton:
        document.getElementById(stamp.StampPanel.DomIds.CANCEL_BUTTON),
    okButton: document.getElementById(stamp.StampPanel.DomIds.OK_BUTTON)
  };
  this.stampPanel_.makeStampEditorPanel(stampEditorButtons);

  // When the stamp editor panel is about to open, set its stamp to the
  // current stamp.
  goog.events.listen(this.stampPanel_, stamp.StampPanel.Events.PANEL_WILL_OPEN,
      this.handlePanelWillOpen_, false, this);

  // Listen for the "PANEL_DID_SAVE" event posted by the stamp editor.
  goog.events.listen(this.stampPanel_, stamp.StampPanel.Events.PANEL_DID_SAVE,
      this.handlePanelDidSave_, false, this);

  // Set up the view controller, it contains the NaCl module.
  var naclModuleId = opt_naclModuleId || life.Application.DomIds_.MODULE;
  this.viewController_ = new life.controllers.ViewController(
      document.getElementById(naclModuleId));
  this.viewController_.setAutomatonRules(this.automatonRules_);
  // Initialize the module with the default stamp.
  this.currentStampId_ = this.viewController_.DEFAULT_STAMP_ID;
  this.viewController_.selectStamp(this.currentStampId_);

  this.viewController_.setStampSoundUrl('sounds/boing_x.wav');

  // Wire up the various controls.
  var playModeSelect =
      document.getElementById(life.Application.DomIds_.PLAY_MODE_SELECT);
  if (playModeSelect) {
    goog.events.listen(playModeSelect, goog.events.EventType.CHANGE,
        this.selectPlayMode, false, this);
  }

  var soundSelect =
      document.getElementById(life.Application.DomIds_.SOUND_SELECT);
  if (playModeSelect) {
    goog.events.listen(soundSelect, goog.events.EventType.CHANGE,
        this.selectSound, false, this);
  }

  var clearButton =
      document.getElementById(life.Application.DomIds_.CLEAR_BUTTON);
  if (clearButton) {
    goog.events.listen(clearButton, goog.events.EventType.CLICK,
        this.clear, false, this);
  }

  var playButton =
      document.getElementById(life.Application.DomIds_.PLAY_BUTTON);
  if (playButton) {
    goog.events.listen(playButton, goog.events.EventType.CLICK,
        this.togglePlayButton, false, this);
  }

  var birthField =
      document.getElementById(life.Application.DomIds_.BIRTH_FIELD);
  if (birthField) {
    goog.events.listen(birthField, goog.events.EventType.CHANGE,
        this.updateBirthRule, false, this);
  }

  var keepAliveField =
      document.getElementById(life.Application.DomIds_.KEEP_ALIVE_FIELD);
  if (keepAliveField) {
    goog.events.listen(keepAliveField, goog.events.EventType.CHANGE,
        this.updateKeepAliveRule, false, this);
  }
}

/**
 * Change the play mode.
 * @param {!goog.events.Event} changeEvent The CHANGE event that triggered this
 *     handler.
 */
life.Application.prototype.selectPlayMode = function(changeEvent) {
  changeEvent.stopPropagation();
  this.viewController_.setPlayMode(changeEvent.target.value);
}

/**
 * Change the stamp sound.
 * @param {!goog.events.Event} changeEvent The CHANGE event that triggered this
 *     handler.
 */
life.Application.prototype.selectSound = function(changeEvent) {
  changeEvent.stopPropagation();
  this.viewController_.setStampSoundUrl(changeEvent.target.value);
}

/**
 * Toggle the simulation.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 */
life.Application.prototype.togglePlayButton = function(clickEvent) {
  clickEvent.stopPropagation();
  var button = clickEvent.target;
  var buttonImage = button.style.backgroundImage;
  var altImage = button.getAttribute(
      life.Application.PlayButtonAttributes_.ALT_IMAGE);
  var state = button.getAttribute(
      life.Application.PlayButtonAttributes_.STATE);
  // Switch the inner and alternate labels.
  goog.style.setStyle(button, { 'backgroundImage': altImage });
  button.setAttribute(life.Application.PlayButtonAttributes_.ALT_IMAGE,
                      buttonImage);
  if (state == 'off') {
    button.setAttribute(
        life.Application.PlayButtonAttributes_.STATE, 'on');
    this.viewController_.run();
  } else {
    button.setAttribute(
        life.Application.PlayButtonAttributes_.STATE, 'off');
    this.viewController_.stop();
  }
}

/**
 * Handle the "panel will open" event: set the stamp in the stamp editor to
 * the current stamp.
 * @param {!goog.events.Event} event The event that triggered this handler.
 * @private
 */
life.Application.prototype.handlePanelWillOpen_ = function(event) {
  event.stopPropagation();
  var currentStamp =
      this.viewController_.stampWithId(this.currentStampId_);
  if (currentStamp)
    this.stampPanel_.setStampFromString(currentStamp);
  return true;
}

/**
 * Handle the "panel did save" event: grab the stamp from the stamp editor,
 * add it to the dictionary of stamps and set it as the current stamp.
 * @param {!goog.events.Event} event The event that triggered this handler.
 * @private
 */
life.Application.prototype.handlePanelDidSave_ = function(event) {
  event.stopPropagation();
  var stampString = this.stampPanel_.getStampAsString();
  this.viewController_.addStampWithId(stampString, this.currentStampId_);
  this.viewController_.selectStamp(this.currentStampId_);
}

/**
 * Clear the current simulation.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 */
life.Application.prototype.clear = function(clickEvent) {
  clickEvent.stopPropagation();
  this.viewController_.clear();
}

/**
 * Read the text input and change it from a comma-separated list into a string
 * of the form BB, where B is a digit in [0..9] that represents the neighbour
 * count that causes a cell to come to life.
 * @param {!goog.events.Event} changeEvent The CHANGE event that triggered this
 *     handler.
 */
life.Application.prototype.updateBirthRule = function(changeEvent) {
  changeEvent.stopPropagation();
  var birthRule = this.parseAutomatonRule_(changeEvent.target.value);
  // Put the sanitized version of the rule string back into the text field.
  changeEvent.target.value = birthRule.join(',');
  // Make the new rule string and tell the NaCl module.
  this.automatonRules_.birthRule = birthRule;
  this.viewController_.setAutomatonRules(this.automatonRules_);
}

/**
 * Read the text input and change it from a comma-separated list into a string
 * of the form SSS, where S is a digit in [0..9] that represents the neighbour
 * count that allows a cell to stay alive.
 * @param {!goog.events.Event} changeEvent The CHANGE event that triggered this
 *     handler.
 */
life.Application.prototype.updateKeepAliveRule = function(changeEvent) {
  changeEvent.stopPropagation();
  var keepAliveRule = this.parseAutomatonRule_(changeEvent.target.value);
  // Put the sanitized version of the rule string back into the text field.
  changeEvent.target.value = keepAliveRule.join(',');
  // Make the new rule string and tell the NaCl module.
  this.automatonRules_.keepAliveRule = keepAliveRule;
  this.viewController_.setAutomatonRules(this.automatonRules_);
}

/**
 * Parse a user-input string representing an automaton rule into an array of
 * neighbour counts.  |ruleString| is expected to be a comma-separated string
 * of integers in range [0..9].  This routine attempts to sanitize non-
 * conforming values by clipping (numbers outside [0..9] are clipped), and
 * replaces non-numberic input with 0.  The resulting array is sorted, and each
 * value is unique.  For example: '1,3,2,2' will produce [1, 2, 3].
 * @param {!string} ruleString The user-input string.
 * @return {Array.<number>} An array of neighbour counts that can be used to
 *    create an automaton rule.
 * @private
 */
life.Application.prototype.parseAutomatonRule_ = function(ruleString) {
  var rule = ruleString.split(',');

  /**
   * Helper function to parse a single rule element: trim off any leading or
   * trailing white-space, then attempt to convert the resulting string into
   * an integer.  Clip the integer to range [0..8].  Replace the element in
   * |array| with the resulting number.  Note: non-numeric values are replaced
   * with 0.
   * @param {string} ruleString The string to parse.
   * @param {number} index The index of the element in the original array.
   * @param {Array} ruleArray The array of rules.
   */
  function parseOneRule(ruleString, index, ruleArray) {
    var neighbourCount = parseInt(ruleString.trim());
    if (isNaN(neighbourCount) || neighbourCount < 0)
      neighbourCount = 0;
    if (neighbourCount > 8)
      neighbourCount = 8;
    ruleArray[index] = neighbourCount;
  }

  // Each rule has to be an integer in [0..8]
  goog.array.forEach(rule, parseOneRule, this);
  // Sort the rules numerically.
  rule.sort(function(a, b) { return a - b; });
  goog.array.removeDuplicates(rule);
  return rule;
}

/**
 * Asserts that cond is true; issues an alert and throws an Error otherwise.
 * @param {bool} cond The condition.
 * @param {String} message The error message issued if cond is false.
 */
life.Application.prototype.assert = function(cond, message) {
  if (!cond) {
    message = "Assertion failed: " + message;
    alert(message);
    throw new Error(message);
  }
}

/**
 * The run() method starts and 'runs' the application.  An <EMBED> tag is
 * injected into the <DIV> element |opt_viewDivName| which causes the NaCl
 * module to be loaded.  Once loaded, the moduleDidLoad() method is called via
 * a 'load' event handler that is attached to the <DIV> element.
 * @param {?String} opt_viewDivName The id of a DOM element in which to
 *     embed the NaCl module.  If unspecified, defaults to DomIds_.VIEW.  The
 *     DOM element must exist.
 */
life.Application.prototype.run = function(opt_viewDivName) {
  var viewDivName = opt_viewDivName || life.Application.DomIds_.VIEW;
  var viewDiv = document.getElementById(viewDivName);
  this.assert(viewDiv, "Missing DOM element '" + viewDivName + "'");

  // A small handler for the 'load' event.  It stops propagation of the 'load'
  // event and then calls moduleDidLoad().  The handler is bound to |this| so
  // that the calling context of the closure makes |this| point to this
  // instance of the life.Applicaiton object.
  var loadEventHandler = function(loadEvent) {
    this.moduleDidLoad(life.Application.DomIds_.MODULE);
  }

  // Note that the <EMBED> element is wrapped inside a <DIV>, which has a 'load'
  // event listener attached.  This method is used instead of attaching the
  // 'load' event listener directly to the <EMBED> element to ensure that the
  // listener is active before the NaCl module 'load' event fires.
  viewDiv.addEventListener('load', goog.bind(loadEventHandler, this), true);

  var viewSize = goog.style.getSize(viewDiv);
  viewDiv.innerHTML = '<embed id="' + life.Application.DomIds_.MODULE + '" ' +
                       ' class="autosize"' +
                       ' width=' + viewSize.width +
                       ' height=' + viewSize.height +
                       ' src="life.nmf"' +
                       ' type="application/x-nacl" />'
}

/**
 * Shut down the application instance.  This unhooks all the event listeners
 * and deletes the objects created in moduleDidLoad().
 */
life.Application.prototype.terminate = function() {
  goog.events.removeAll();
  this.viewController_ = null;
}

/**
 * Extend the String class to trim whitespace.
 * @return {string} the original string with leading and trailing whitespace
 *     removed.
 */
String.prototype.trim = function () {
  return this.replace(/^\s*/, '').replace(/\s*$/, '');
}
