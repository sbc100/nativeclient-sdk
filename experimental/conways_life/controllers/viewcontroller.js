// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview  Implement the view controller class, ViewController, that
 * owns the Life NaCl module and wraps JavaScript bridge calls to it.  This
 * class also handles certain UI interactions, such as mouse drags and keyboard
 * shortcuts.
 */

goog.provide('life.controllers.ViewController');

goog.require('goog.Disposable');
goog.require('goog.dom');
goog.require('goog.events.EventTarget');
goog.require('goog.fx.DragEvent');
goog.require('goog.object');
goog.require('goog.style');
goog.require('uikit.events.Dragger');

/**
 * Constructor for the ViewController class.  This class encapsulates the
 * Life NaCl module in a view.  It also produces some UI events, such as mouse
 * drags.
 * @param {!Object} nativeModule The DOM element that represents a
 *      ViewController (usually the <EMBED> element that contains the NaCl
 *      module).  If undefined, an error is thrown.
 * @constructor
 * @extends {goog.events.EventTarget}
 */
life.controllers.ViewController = function(nativeModule) {
  goog.events.EventTarget.call(this);
  /**
   * The element containing the Life NaCl module that corresponds to
   * this object instance.  If null or undefined, an exception is thrown.
   * @type {Element}
   * @private
   */
  if (!nativeModule) {
    throw new Error('ViewController() requires a valid NaCl module');
  }
  // The container is the containing DOM element.
  this.module_ = nativeModule;

  /**
   * The play mode.  Accessed via playMode(), mutated via setPlayMode().
   * Note that changing the play mode can cause the simulation to restart.
   * @type {enum}
   * @private
   */
  this.playMode_ = life.controllers.ViewController.PlayModes_.RANDOM_SEED;

  /**
   * Indicate whether the simulation is running.
   * @type {bool}
   * @private
   */
  this.isRunning_ = false;

  /**
   * The map of stamps.  Initialized to a default stamp that produces a glider
   * when using the "normal" Conway rules of 23/3.
   * @type {Hash}
   * @private
   */
  this.stampDictionary_ = {};
  this.addStampWithId('***\n*..\n.*.\n', this.DEFAULT_STAMP_ID);

  /**
   * Mouse drag event object.
   * @type {life.events.Dragger}
   * @private
   */
  this.dragListener_ = new uikit.events.Dragger(nativeModule);
  // Hook up a Dragger and listen to the drag events coming from it, then
  // reprocess the events as Life DRAG events.
  goog.events.listen(this.dragListener_, goog.fx.Dragger.EventType.START,
      this.handleStartDrag_, false, this);
  goog.events.listen(this.dragListener_, goog.fx.Dragger.EventType.END,
      this.handleEndDrag_, false, this);
  goog.events.listen(this.dragListener_, goog.fx.Dragger.EventType.DRAG,
      this.handleDrag_, false, this);
};
goog.inherits(life.controllers.ViewController, goog.events.EventTarget);

/**
 * Values for the play mode.  These come from the |PLAY_MODE_SELECT| element.
 * @enum {string}
 * @private
 */
life.controllers.ViewController.PlayModes_ = {
  RANDOM_SEED: 'random_seed',
  STAMP: 'stamp'
};

/**
 * The id for the default stamp.
 * @type {string}
 */
life.controllers.ViewController.prototype.DEFAULT_STAMP_ID = 'default_stamp';

/**
 * Override of disposeInternal() to unhook all the listeners and dispose
 * of retained objects.
 * @override
 */
life.controllers.ViewController.prototype.disposeInternal = function() {
  life.controllers.ViewController.superClass_.disposeInternal.call(this);
  goog.events.unlisten(this.dragListener_, goog.fx.Dragger.EventType.START,
      this.handleStartDrag_, false, this);
  goog.events.unlisten(this.dragListener_, goog.fx.Dragger.EventType.DRAG,
      this.handleDrag_, false, this);
  goog.events.unlisten(this.dragListener_, goog.fx.Dragger.EventType.END,
      this.handleEndDrag_, false, this);
  this.dragListener_ = null;
  this.module_ = null;
  this.stampDictionary_ = null;
};

/**
 * Simple wrapper that forwards the "clear" method to the NaCl module.
 */
life.controllers.ViewController.prototype.clear = function() {
  this.invokeMethod_('clear');
}

/**
 * Return the current play mode.
 * @return {enum} The current play mode.
 */
life.controllers.ViewController.prototype.playMode = function() {
  return this.playMode_;
}

/**
 * Set the play mode to one of RANDOM_SEED or STAMP.  Changing the play mode
 * can cause the simulation to restart in the new play mode.  Do nothing if the
 * play mode is set to the current mode.
 * @param {string} playMode The new play mode.
 */
life.controllers.ViewController.prototype.setPlayMode = function(playMode) {
  if (playMode == this.playMode_)
    return;
  this.playMode_ = playMode;
  if (this.isRunning_) {
    this.invokeMethod_('runSimulation', { mode: this.playMode_ });
  }
}

/**
 * Set the automaton rules.  The rules are expressed as an object that maps
 * birth and keep-alive rules to neighbour counts.
 * @param {Object.<Array>} automatonRules The new rule string.
 */
life.controllers.ViewController.prototype.setAutomatonRules =
    function(automatonRules) {
  var ruleString = [automatonRules.keepAliveRule.join(''),
                    automatonRules.birthRule.join('')].join('/');
  this.invokeMethod_('setAutomatonRules', { rules: ruleString });
}

/**
 * Add a stamp to the simulation.  The stamp is expressed as a string where
 * each character represents a cell: '*' is a live cell and '.' is a dead one.
 * A new-line represents the end of arow of cells.  See the .LIF 1.05 format
 * for more details:
 *   http://psoup.math.wisc.edu/mcell/ca_files_formats.html
 * If a stamp with |stampId| already exists, then it gets replaced with the
 * new |stampDefinition|.
 * @param {!string} stampDescription The new stamp description.
 * @param {!string} stampId The id associated with this stamp.
 */
life.controllers.ViewController.prototype.addStampWithId =
    function(stampDescription, stampId) {
  this.stampDictionary_[stampId] = stampDescription;
}

/**
 * Set the current stamp.  If a stamp with id |stampId| doesn't exist, then
 * do nothing.
 * @param {!string} stampDescription The new stamp description.
 * @param {!string} stampId The id associated with this stamp.
 * @return {bool} Success.
 */
life.controllers.ViewController.prototype.selectStamp = function(stampId) {
  if (stampId in this.stampDictionary_) {
    this.invokeMethod_('setCurrentStamp',
                       { description: this.stampDictionary_[stampId] });
    return true;
  }
  return false;
}

/**
 * Return the encoded string for stamp with id |stampId|.  If no such stamp
 * exists, return null.
 * @return {?string} The current stamp string.
 */
life.controllers.ViewController.prototype.stampWithId = function(stampId) {
  if (stampId in this.stampDictionary_)
    return this.stampDictionary_[stampId];
  return null;
}

/**
 * Start the simulation.  Does nothing if it's already running.
 */
life.controllers.ViewController.prototype.run = function() {
  this.isRunning_ = true;
  this.invokeMethod_('runSimulation', { mode: this.playMode_ });
}

/**
 * Stop the simulation.  Does nothing if it's already stopped.
 */
life.controllers.ViewController.prototype.stop = function() {
  this.isRunning_ = false;
  this.invokeMethod_('stopSimulation');
}


/**
 * Set the stamp sound to the file pointed at by |stampSoundUrl|.  For now,
 * only .wav files are supported.  If there is a currentstamp sound in effect,
 * it will be replaced with this one.
 * @param {!string} stampSoundUrl The Url that points to a .wav file containing
 *     the stamp sound.
 */
life.controllers.ViewController.prototype.setStampSoundUrl =
    function(stampSoundUrl) {
  if (stampSoundUrl.length > 0) {
    this.invokeMethod_('setStampSoundUrl', { soundUrl: stampSoundUrl });
  }
}

/**
 * Convert the coordinate system of |point| to the root window's coordinate
 * system.
 * @param {!goog.math.Coordinate} point The point in the coordinate system
 *     of this view.
 * @return {goog.math.Coordinate} The converted point.
 */
life.controllers.ViewController.prototype.convertPointToWindow =
    function(point) {
  var offset = goog.style.getFramedPageOffset(this.module_, window);
  return goog.math.Coordinate.difference(point, offset);
}

/**
 * Format a method invocation and call postMessage with the formatted method
 * string.  This calls the NaCl module with the invocation string.  Note that
 * this is an asynchronous call into the NaCl module.
 * @param {!string} methodName The name of the method.  This must match a
 *     published method name in the NaCl module.
 * @param {?Object} parameters A dictionary that maps parameter names to
 *     values.  All parameter values are passed a strings.
 */
life.controllers.ViewController.prototype.invokeMethod_ =
    function(methodName, opt_parameters) {
  var method_invocation = methodName
  if (opt_parameters) {
    for (param in opt_parameters) {
      method_invocation += ' ' + param + ':' + opt_parameters[param]
    }
  }
  this.module_.postMessage(method_invocation);
}

/**
 * Handle the drag START event: add a cell at the event's coordinates.
 * @param {!goog.fx.DragEvent} dragStartEvent The START event that
 *     triggered this handler.
 * @private
 */
life.controllers.ViewController.prototype.handleStartDrag_ =
    function(dragStartEvent) {
  dragStartEvent.stopPropagation();
  var point = this.convertPointToWindow(
      new goog.math.Coordinate(dragStartEvent.clientX,
                               dragStartEvent.clientY));
  this.invokeMethod_('putStampAtPoint', { x: point.x, y: point.y });
};

/**
 * Handle the DRAG event: add a cell at the event's coordinates.
 * @param {!goog.fx.DragEvent} dragEvent The DRAG event that triggered this
 *     handler.
 * @private
 */
life.controllers.ViewController.prototype.handleDrag_ = function(dragEvent) {
  dragEvent.stopPropagation();
  var point = this.convertPointToWindow(
      new goog.math.Coordinate(dragEvent.clientX, dragEvent.clientY));
  this.invokeMethod_('putStampAtPoint', { x: point.x, y: point.y });
};

/**
 * Handle the drag END event: stop propagating the event.
 * @param {!goog.fx.DragEvent} dragEndEvent The END event that triggered this
 *     handler.
 * @private
 */
life.controllers.ViewController.prototype.handleEndDrag_ =
    function(dragEndEvent) {
  dragEndEvent.stopPropagation();
};

/**
 * Handle keyboard shortcut events.  These get transformed into Ginsu app
 * events and re-dispatched.
 * @param {!goog.ui.KeyboardShortcutEvent} keyboardEvent The SHORTCUT event
 * that triggered this handler.
 * @private
 */
life.controllers.ViewController.prototype.handleKeyboard_ =
    function(keyboardEvent) {
  var eventClone = goog.object.clone(keyboardEvent);
  keyboardEvent.stopPropagation();
  this.dispatchEvent(new life.events.Event(life.events.EventType.ACTION, this,
      keyboardEvent.identifier));
};
