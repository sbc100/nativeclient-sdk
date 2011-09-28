// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The FlockingGeese application object.  Encapsulates three principle app
 * elements: the NaCl module which runs the C++ implementation of the flocking
 * algorithm, a canvas used by the JavaScript implementation, and a DOM
 * element that contains the info display (which includes things like the
 * speedometer, control buttons, etc.).
 */

goog.provide('FlockingGeese');

goog.require('Flock');
goog.require('Goose');
goog.require('LoadProgress');
goog.require('Speedometer');
goog.require('Speedometer.Attributes');
goog.require('goog.Disposable');
goog.require('goog.array');
goog.require('goog.dom');
goog.require('goog.events.EventType');
goog.require('goog.style');

/**
 * Constructor for the FlockingGeese class.  Use the run() method to populate
 * the object with controllers and wire up the events.
 * @constructor
 * @extends {goog.Disposable}
 */
FlockingGeese = function() {
  goog.Disposable.call(this);

  /**
   * The flock of geese as implemented in JavaScript.
   * @type {Flock}
   * @private
   */
  this.flock_ = new Flock();

  /**
   * The timer.
   * @type {Timer}
   * @private
   */
  this.flockingSimTimer_ = null;

  /**
   * The NaCl module.  This is an <EMBED> element that contains the NaCl
   * module.  Set during moduleDidLoad().
   * @type {Element}
   * @private
   */
  this.naclModule_ = null;

  /**
   * The load progress bar.
   * @type {LoadProgress}
   * @private
   */
  this.progressBar_ = new LoadProgress();

  /**
   * The speedometer.
   * @type {Speedometer}
   * @private
   */
  this.speedometer_ = new Speedometer();

  /**
   * The simluation mode slider.  This becomes a Slider object.
   * @type {Slider}
   * @private
   */
  this.simModeSlider_ = null;

  /**
   * The flock size slider.  This becomes a Slider object.
   * @type {Slider}
   * @private
   */
  this.flockSizeSlider_ = null;

  /**
   * THe current simulation mode.  Starts in 'JavaScript'.
   * @type {string}
   * @private
   */
  this.simulationMode_ = FlockingGeese.SimulationMode.JAVASCRIPT;
}
goog.inherits(FlockingGeese, goog.Disposable);

/**
 * The simulation tick time.  A full simulation step runs in at least this
 * amount of time.  Measured in milliseconds.
 * @type {number}
 */
FlockingGeese.prototype.SIMULATION_TICK = 10;

/**
 * Method signatures that are sent from the NaCl module.  A signature is really
 * just a method id - later we can upgrade this to accept (say) full JSON
 * strings and parse them, but for now a simple solution will work.  The strings
 * need to match the corresponding ones labeled "output method ids" in
 * flocking_geese_app.cc.
 * @enum {Object}
 * @private
 */
FlockingGeese.prototype.MethodSignatures_ = {
  SET_SIMULATION_INFO: 'setSimulationInfo',
  FRAME_RATE: 'frameRate'
};

/**
 * The ids used for elements in the DOM.  The FlockingGeese application
 * expects these elements to exist.
 * @enum {string}
 */
FlockingGeese.DomIds = {
  // The elements for the flock size slider.  Contained in the info panel.
  FLOCK_SIZE_SLIDER: 'flock_size_slider',
  FLOCK_SIZE_SLIDER_RULER: 'flock_size_slider_ruler',
  FLOCK_SIZE_SLIDER_THUMB: 'flock_size_slider_thumb',
  // The <FORM> containing various controls.
  CONTROLS_FORM: 'game_controls_form',
  // The <CANVAS> where the JS implementation draws its geese.
  GOOSE_CANVAS: 'flocking_geese_canvas',
  // The <EMBED> element containing the NaCl module.
  NACL_MODULE: 'flocking_geese_nacl_module',
  // The <DIV> containing the simulation drawing area (either the NaCl module
  // or the <CANVAS>.
  NACL_VIEW: 'nacl_flocking_geese',
  // The slider buttons used to select sim type (NaCL vs. JavaScript).
  // Contained in the info panel.
  SIM_MODE_SELECT: 'sim_mode_select',
  SIM_SELECT_RULER: 'sim_select_ruler',
  SIM_SELECT_THUMB: 'sim_select_thumb',
  // The text that displays the speed difference.
  SPEED_DIFFERENCE_LABEL: 'speed_difference',
  // The speedometer <CANVAS> element.  Contained in the info panel.
  SPEEDOMETER: 'speedometer_canvas'
};

/**
 * The list of simulation modes.  These are the values assigned to the
 * simulation mode radio buttons.
 * @enum {string}
 */
FlockingGeese.SimulationMode = {
  NACL: 'NaCl',
  JAVASCRIPT: 'JavaScript'
};

/**
 * The list of meter names.
 * @enum {string}
 */
FlockingGeese.MeterNames = {
  NACL: 'NaCl',
  JAVASCRIPT: 'JavaScript'
};

/**
 * Override of disposeInternal() to dispose of retained objects.
 * @override
 */
FlockingGeese.prototype.disposeInternal = function() {
  this.terminate();
  FlockingGeese.superClass_.disposeInternal.call(this);
}

/**
 * Set up all the DOM elements and wire together the events.
 */
FlockingGeese.prototype.initializeApplication = function() {
  // Listen for 'unload' in order to terminate cleanly.
  goog.events.listen(window, goog.events.EventType.UNLOAD, this.terminate);
  var naclMeterAttribs = {};
  naclMeterAttribs[Speedometer.Attributes.THEME] = Speedometer.Themes.GREEN;
  naclMeterAttribs[Speedometer.Attributes.ODOMETER_LEFT] = 84;
  naclMeterAttribs[Speedometer.Attributes.ODOMETER_TOP] = 156;
  this.speedometer_.addMeterWithName(FlockingGeese.MeterNames.NACL,
                                     naclMeterAttribs);
  var jsMeterAttribs = {};
  jsMeterAttribs[Speedometer.Attributes.THEME] = Speedometer.Themes.RED;
  jsMeterAttribs[Speedometer.Attributes.ODOMETER_LEFT] = 84;
  jsMeterAttribs[Speedometer.Attributes.ODOMETER_TOP] = 192;
  this.speedometer_.addMeterWithName(FlockingGeese.MeterNames.JAVASCRIPT,
                                     jsMeterAttribs);
  this.speedometer_.setMaximumSpeed(10000.0);  // Measured in frames per second.

  var speedometerCanvas =
      document.getElementById(FlockingGeese.DomIds.SPEEDOMETER);
  this.speedometer_.setCanvas(speedometerCanvas);
  this.speedometer_.reset();
  this.speedometer_.render(this.speedometerCanvas_);

  // Wire up the various controls.
  var simSelectSlider =
      document.getElementById(FlockingGeese.DomIds.SIM_MODE_SELECT);
  var simSelectRuler =
      document.getElementById(FlockingGeese.DomIds.SIM_SELECT_RULER);
  var simSelectThumb =
      document.getElementById(FlockingGeese.DomIds.SIM_SELECT_THUMB);
  this.simModeSlider_ = new Slider(
    [{simMode: FlockingGeese.SimulationMode.NACL},
     {simMode: FlockingGeese.SimulationMode.JAVASCRIPT}
    ]);
  this.simModeSlider_.decorate(simSelectSlider, simSelectRuler, simSelectThumb);
  this.simModeSlider_.rulerLength = 80;
  this.simModeSlider_.rulerOffset = 12;
  this.simModeSlider_.slideToStep(1);
  goog.events.listen(this.simModeSlider_, SliderEvent.EventType.CHANGE,
      this.toggleSimMode, false, this);

  var flockSizeSlider =
      document.getElementById(FlockingGeese.DomIds.FLOCK_SIZE_SLIDER);
  var sliderRuler = document.getElementById(
      FlockingGeese.DomIds.FLOCK_SIZE_SLIDER_RULER);
  var sliderThumb = document.getElementById(
      FlockingGeese.DomIds.FLOCK_SIZE_SLIDER_THUMB);
  this.flockSizeSlider_ = new Slider(
      [{gooseCount: 50, meterLimit: 30000.0},
       {gooseCount: 100, meterLimit: 8000.0},
       {gooseCount: 500, meterLimit: 400.0},
       {gooseCount: 1000, meterLimit: 100.0}
      ]);
  this.flockSizeSlider_.decorate(flockSizeSlider, sliderRuler, sliderThumb);
  this.flockSizeSlider_.rulerLength = 110;
  this.flockSizeSlider_.rulerOffset = 18;
  this.flockSizeSlider_.slideToStep(0);
  goog.events.listen(this.flockSizeSlider_, SliderEvent.EventType.CHANGE,
      this.selectFlockSize, false, this);

  // Populate the JavaScript verison of the simulation.
  var flockSize = 0;
  if (this.flockSizeSlider_) {
    var stepObject = this.flockSizeSlider_.objectAtStepValue();
    flockSize = stepObject.gooseCount;
    this.speedometer_.setMaximumSpeed(stepObject.meterLimit);
  }
  var initialLocation = this.getCanvasCenter_();
  this.flock_.resetFlock(flockSize, initialLocation);
}

/**
 * Called by the module loading function once the module has been loaded.
 * @param {?String} opt_naclModuleId The id of an <EMBED> element which
 *     contains the NaCl module.  If unspecified, defaults to |NACL_MODULE|.
 */
FlockingGeese.prototype.moduleDidLoad = function(opt_naclModuleId) {
  // Set up the view controller, it contains the NaCl module.
  var naclModuleId = opt_naclModuleId || FlockingGeese.DomIds.NACL_MODULE;
  this.naclModule_ = document.getElementById(naclModuleId);
  if (this.flockSizeSlider_) {
    this.invokeNaClMethod('resetFlock',
        {'size' : this.flockSizeSlider_.objectAtStepValue().gooseCount});
  }
  // Hide the load progress bar and make the module element visible.
  this.progressBar_.setVisible(false);
  this.naclModule_.style.visibility = 'inherit';
  // If the NaCL module is being displayed, then start the simulation.
  var naclView = document.getElementById(FlockingGeese.DomIds.NACL_VIEW);
  if (naclView.style.visibility == 'visible') {
    this.invokeNaClMethod('runSimulation');
  }
}

/**
 * Asserts that cond is true; issues an alert and throws an Error otherwise.
 * @param {bool} cond The condition.
 * @param {String} message The error message issued if cond is false.
 */
FlockingGeese.prototype.assert = function(cond, message) {
  if (!cond) {
    message = "Assertion failed: " + message;
    alert(message);
    throw new Error(message);
  }
}

/**
 * Change the flock size.
 * @param {!goog.events.Event} changeEvent The CHANGE event that triggered this
 *     handler.
 */
FlockingGeese.prototype.selectFlockSize = function(changeEvent) {
  changeEvent.stopPropagation();
  var initialLocation = this.getCanvasCenter_();
  var stepObject =
      this.flockSizeSlider_.objectAtStepValue(changeEvent.stepValue);
  var newFlockSize = stepObject.gooseCount;
  this.speedometer_.setMaximumSpeed(stepObject.meterLimit);
  // Reset the speedometers to 0.
  this.speedometer_.updateMeterNamed(FlockingGeese.MeterNames.NACL, 0);
  this.speedometer_.updateMeterNamed(FlockingGeese.MeterNames.JAVASCRIPT, 0);
  this.speedometer_.reset();
  this.flock_.resetFlock(newFlockSize, initialLocation);
  this.invokeNaClMethod('resetFlock', {'size' : newFlockSize});
  this.updateSpeedDifference_();
}

/**
 * Toggle the simulation mode.
 * @param {!goog.events.Event} changeEvent The CHANGE event that triggered this
 *     handler.
 */
FlockingGeese.prototype.toggleSimMode = function(changeEvent) {
  changeEvent.stopPropagation();
  var simModeObject =
      this.simModeSlider_.objectAtStepValue(changeEvent.stepValue);
  var simMode = simModeObject.simMode;
  if (simMode == this.simulationMode_)
    return;
  this.simulationMode_ = simMode;
  if (simMode == FlockingGeese.SimulationMode.NACL) {
    // When toggling to 'NaCl', turn off the JavaScript simulation timer and
    // swap the DOM elements to display the NaCl implementation.
    this.stopJavaScriptSimulation_();
    var flockingCanvas =
        document.getElementById(FlockingGeese.DomIds.GOOSE_CANVAS);
    flockingCanvas.style.visibility = 'hidden';
    var naclView =
        document.getElementById(FlockingGeese.DomIds.NACL_VIEW);
    naclView.style.visibility = 'visible';
    // Start up the NaCl simulation.
    this.invokeNaClMethod('runSimulation');
  } else {
    // In the 'on' state, the simulation runs the NaCl implementation.
    this.invokeNaClMethod('pauseSimulation');
    this.startJavaScriptSimulation_();
    var flockingCanvas =
        document.getElementById(FlockingGeese.DomIds.GOOSE_CANVAS);
    flockingCanvas.style.visibility = 'visible';
    var naclView =
        document.getElementById(FlockingGeese.DomIds.NACL_VIEW);
    naclView.style.visibility = 'hidden';
  }
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
FlockingGeese.prototype.invokeNaClMethod =
    function(methodName, opt_parameters) {
  if (!this.naclModule_) {
    return;
  }
  var methodInvocation = methodName
  if (opt_parameters) {
    for (param in opt_parameters) {
      methodInvocation += ' ' + param + ':' + opt_parameters[param]
    }
  }
  this.naclModule_.postMessage(methodInvocation);
}

/**
 * Handle a message posted by the NaCl module.  An incomming NaCl message is
 * formatted like the outgoing message in invokeNaClMethod() (see above).
 * There is a method name, followed by a space-separated list of named
 * parameters.  The parameters are separated from their values by a ':'.
 * @param {Event} messageEvent The Event wrapping the message.
 */
FlockingGeese.prototype.handleNaClMessage = function(messageEvent) {
  var methodInvocation = messageEvent.data.split(' ');
  // The first element of |methodInvocation| is the method name.  Use this as
  // the key to look up the rest of the signature.
  if (methodInvocation[0] == this.MethodSignatures_.SET_SIMULATION_INFO) {
    for (var paramCount = 1;
         paramCount < methodInvocation.length;
         paramCount++) {
      var parameter = methodInvocation[paramCount].split(':');
      // The first element is the parameter name, the second element is its
      // value.
      if (parameter[0] == this.MethodSignatures_.FRAME_RATE) {
        var frameRate = parseFloat(parameter[1]);
        this.speedometer_.updateMeterNamed(FlockingGeese.MeterNames.NACL,
                                           frameRate);
        this.updateSpeedDifference_();
      }
    }
  } else {
    var error = 'Unrecognized NaCl message: ' + messageEvent.data;
    console.log(error);
    throw new Error(error);
  }
}

/**
 * The run() method starts and 'runs' the FlockingGeese app.  An <EMBED> tag is
 * injected into the <DIV> element |opt_viewDivName| which causes the NaCl
 * module to be loaded.  Once loaded, the moduleDidLoad() method is called via
 * a 'load' event handler that is attached to the <DIV> element.
 * @param {?String} opt_viewDivName The id of a DOM element in which to
 *     embed the NaCl module.  If unspecified, defaults to DomIds.VIEW.  The
 *     DOM element must exist.
 */
FlockingGeese.prototype.run = function(opt_viewDivName) {
  var viewDivName = opt_viewDivName || FlockingGeese.DomIds.NACL_VIEW;
  var viewDiv = document.getElementById(viewDivName);
  this.assert(viewDiv, "Missing DOM element '" + viewDivName + "'");

  this.initializeApplication();

  // A small handler for the 'load' event.  It stops propagation of the 'load'
  // event and then calls moduleDidLoad().  The handler is bound to |this| so
  // that the calling context of the closure makes |this| point to this
  // instance of the Applicaiton object.
  var loadEventHandler = function(loadEvent) {
    this.moduleDidLoad(FlockingGeese.DomIds.NACL_MODULE);
  }

  // Note that the <EMBED> element is wrapped inside a <DIV>, which has a 'load'
  // event listener attached.  This method is used instead of attaching the
  // 'load' event listener directly to the <EMBED> element to ensure that the
  // listener is active before the NaCl module 'load' event fires.
  viewDiv.addEventListener('load', goog.bind(loadEventHandler, this), true);
  // Attach a listener for messages coming from the NaCl module.
  viewDiv.addEventListener('message',
                           goog.bind(this.handleNaClMessage, this),
                           true);
  // Handle the load progress.
  viewDiv.addEventListener(
      'progress',
      goog.bind(this.progressBar_.handleProgressEvent, this.progressBar_),
      true);

  var viewSize = goog.style.getSize(viewDiv);
  viewDiv.appendChild(this.progressBar_.createDom());
  var naclDom = goog.dom.createDom('embed', {
      'id': FlockingGeese.DomIds.NACL_MODULE,
      'class': 'autosize-view',
      'width': viewSize.width,
      'height': viewSize.height,
      'src': 'flocking_geese.nmf',
      'type': 'application/x-nacl',
      'style': 'visibility: inherit'
  });
  viewDiv.appendChild(naclDom);
  this.startJavaScriptSimulation_();
}

/**
 * Run one tick of the simulation and then render the new flock.  Update the
 * speedometer with new timing values.
 */
FlockingGeese.prototype.simulationTick = function() {
  var flockingCanvas =
      document.getElementById(FlockingGeese.DomIds.GOOSE_CANVAS);
  // Clear the canvas
  var context2d = flockingCanvas.getContext('2d');
  var flockBox = new goog.math.Rect(0,
                                    0,
                                    flockingCanvas.width,
                                    flockingCanvas.height);
  context2d.fillStyle = 'white';
  context2d.fillRect(flockBox.top,
                     flockBox.left,
                     flockBox.width,
                     flockBox.height);
  var frameRate = this.flock_.flock(flockBox);
  this.flock_.render(flockingCanvas);
  this.speedometer_.updateMeterNamed(FlockingGeese.MeterNames.JAVASCRIPT,
                                     frameRate);
  this.updateSpeedDifference_();
  this.startJavaScriptSimulation_();
}

/**
 * Shut down the application instance.  This unhooks all the event listeners
 * and deletes the objects created in moduleDidLoad().
 */
FlockingGeese.prototype.terminate = function() {
  goog.events.removeAll();
  this.stopJavaScriptSimulation_();
}

/**
 * Start up the JavaScript implementation of the simulation by enabling the
 * simulation timer.
 * @private
 */
FlockingGeese.prototype.startJavaScriptSimulation_ = function() {
  // Start up the JavaScript implementation of the simulation.
  this.flockingSimTimer_ = setTimeout(goog.bind(this.simulationTick, this),
                                      this.SIMULATION_TICK);
}

/**
 * Stop the JavaScript implementation of the simulation by clearing the
 * simulation timer.
 * @private
 */
FlockingGeese.prototype.stopJavaScriptSimulation_ = function() {
  // Stop the JavaScript implementation of the simulation.
  if (this.flockingSimTimer_) {
    clearTimeout(this.flockingSimTimer_);
    this.flockingSimTimer_ = null;
  }
}

/**
 * Recompute the speed difference between JavaScript and Native Client.
 * Color the result depending on which is faster.
 * @private
 */
FlockingGeese.prototype.updateSpeedDifference_ = function() {
  var speedDifferenceLabel =
      document.getElementById(FlockingGeese.DomIds.SPEED_DIFFERENCE_LABEL);
  var naclSpeed = this.speedometer_.valueForMeterNamed(
      FlockingGeese.MeterNames.NACL,
      Speedometer.Attributes.ODOMETER_DISPLAY_VALUE);
  var jsSpeed = this.speedometer_.valueForMeterNamed(
      FlockingGeese.MeterNames.JAVASCRIPT,
      Speedometer.Attributes.ODOMETER_DISPLAY_VALUE);
  if (naclSpeed == 0 || jsSpeed == 0) {
    speedDifferenceLabel.innerHTML = 'Not available&hellip;';
    speedDifferenceLabel.style.fontStyle = 'italic';
    speedDifferenceLabel.style.color = '#949596';
  } else {
    var diff = naclSpeed / jsSpeed;
    speedDifferenceLabel.innerHTML = diff.toFixed(1) + 'X';
    speedDifferenceLabel.style.fontStyle = 'normal';
    speedDifferenceLabel.style.color = diff > 1.0 ? '#009933' : '#FF0033';
  }
}

/**
 * Find the center point of the simulation canvas.  If the a canvas element
 * with id |GOOSE_CANVAS| doesn't exist, return (0, 0).
 * @return {goog.math.Vec2} The center of the simulation canvas, measured in
 *     pixels.
 * @private
 */
FlockingGeese.prototype.getCanvasCenter_ = function() {
  var flockingCanvas =
      document.getElementById(FlockingGeese.DomIds.GOOSE_CANVAS);
  if (flockingCanvas)
    return new goog.math.Vec2(flockingCanvas.width / 2,
                              flockingCanvas.height / 2);
  else
    return new goog.math.Vec2(0, 0);
}
