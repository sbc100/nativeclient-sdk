// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the top level JavaScript required for the
 * WebGTT application. The two main functions are fired when the page loads and
 * when the NaCl module loads, respectively. We handle these load events
 * separately since the order in which these events are fired is much less
 * predictable than other standard DOM elements (for which we assume that they
 * are loaded when the page is loaded).
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * Each of these variables is an object corresponding to a DOM element. Since
 * event handlers for one DOM element may need to modify properties of or
 * perform actions on another DOM element, these objects have global scope.
 */
var naclModule1 = undefined; // NaClModule object. See naclmodule.js
var graph1 = new Graph(); // Graph object. See graph.js
var canvas1 = undefined; // Canvas object. See canvas.js
var coloringButton1 = undefined; // ColoringButton object. See button.js

/**
 * This constant holds the default status message.
 * @type {string}
 * @const
 */
STATUS_TEXT = 'NO-STATUS';

/**
 * This function is triggered when the page has finished loading, and contains
 * starter code (initializing objects, etc.).
 *
 * The NaCl module could still be loading while the page has finished loading.
 * So, the status must be updated accordingly, and no NaClModule object is
 * created yet. New objects of the Canvas and ColoringButton classes are
 * created, and the appropriate event handlers are attached.
 */
function main1 () {
  if (naclModule1 == undefined) {
    updateStatus('LOADING...');
  } else {
    updateStatus();
  }

  canvas1 = new Canvas(document.getElementById('mainCanvas'), graph1);
  canvas1.drawingCanvas.addEventListener(
      'click', function (e) {canvas1.handleCanvasClick(e);}, false);
  window.addEventListener(
      'keydown', function (e) {canvas1.handleCanvasKeydown(e);}, true);

  coloringButton1 = new ColoringButton(document.getElementById('coloring'),
                                       graph1);
  coloringButton1.coloringButton.addEventListener(
      'click', function (e) {coloringButton1.handleColoringButtonClick(e);},
      false);
}

/**
 * This function is triggered when the NaCl module has finished loading. It
 * creates a new NaClModule object, attaches an event handler to it, and
 * indcates to the user that the NaCl module has loaded successfully.
 */
function main2() {
  naclModule1 = new NaClModule(document.getElementById('webgtt'), graph1);
  naclModule1.naclModule.addEventListener(
      'message', function (e) {naclModule1.handleMessage(e);}, false);
  updateStatus('SUCCESS');
}

/**
 * This function sets the global status message.
 *
 * If the element with id 'statusField' exists, then its HTML is set to the
 * status message as well. opt_message is the message text. If this is null or
 * undefined, then the element with id 'statusField' is set to STATUS_TEXT.
 *
 * @param {string} opt_message The status message to be displayed.
 */
function updateStatus(opt_message) {
  var statusText = STATUS_TEXT;
  if (opt_message) {
    statusText = opt_message;
  }
  var statusField = document.getElementById('status_field');
  if (statusField) {
    statusField.innerHTML = statusText;
  }
}
