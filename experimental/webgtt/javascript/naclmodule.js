// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the JavaScript required for the WebGTT
 * application, specifically, the implementation of the NaClModule class. This
 * implementation provides functions for manipulating a NaCl module, and
 * adding/managing event handlers for a NaCl module.
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * This constant holds the color palette used to color the vertices.
 * @type {Array.<string>}
 * @const
 */
var COLOR_PALETTE = [
  'red',
  'blue',
  'green',
  'yellow',
  'orange',
  'magenta',
  'chocolate',
  'black',
  'greenyellow',
  'grey',
  'pink',
  'yellowgreen'
];

/**
 * This class is used to represent a NaCl module.
 *
 * @param naclModule Handle to the DOM object representing the NaCl module.
 * @param {Graph} graph1 Reference to the graph associated with the NaCl module.
 * @constructor
 */
NaClModule = function (naclModule, graph1) {
  this.naclModule = naclModule;
  this.graph1 = graph1;
  coloringButton1.setText(BUTTON_TEXT);
  if (canvas1.graph1.listOfVertices.length > 0) {
    coloringButton1.setDisabled(false);
  }
};

/**
 * This function invokes the postMessage function on the NaCl module.
 *
 * @param {string} message The argument for postMessage.
 */
NaClModule.prototype.postMessage = function (message) {
  this.naclModule.postMessage(message);
};

/**
 * This function handles the 'message' event on the NaCl module.
 *
 * This handler is fired when the NaCl module posts a message to the browser.
 * Here, we color the graph according to the coloring that is received from the
 * NaCl module.
 *
 * @param messageEvent The Event object containing information about the
 *     'message' event.
 */
NaClModule.prototype.handleMessage = function (messageEvent) {
  var tempString = '';
  var vertexIndex = 0;

  for (var i = 0 ; i < messageEvent.data.length ; i++) {
    if(messageEvent.data[i] != ',') {
      tempString += messageEvent.data[i];
      continue;
    }
    this.graph1.listOfVertices[vertexIndex].setColor(COLOR_PALETTE[parseInt(
        tempString)]);
    tempString = '';
    vertexIndex ++;
  }
  this.graph1.listOfVertices[vertexIndex].setColor(COLOR_PALETTE[parseInt(
      tempString)]);

  canvas1.redrawCanvas();
  alert('A coloring is displayed!');

  for (var i = 0 ; i < ((messageEvent.data.length)+1)/2 ; i++) {
    this.graph1.listOfVertices[i].setColor('black');
  }

  canvas1.redrawCanvas();

  canvas1.setEditMode(true);
  coloringButton1.setDisabled(false);
  coloringButton1.setText(BUTTON_TEXT);
};
