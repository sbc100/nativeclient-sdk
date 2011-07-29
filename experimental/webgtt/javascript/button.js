// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the JavaScript required for the WebGTT
 * application, specifically, the implementation of the ColoringButton class.
 * This implementation provides functions for manipulating a coloring button,
 * and adding/managing event handlers for a coloring button.
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * This constant holds the default text for the coloring button.
 * @type {string}
 * @const
 */
BUTTON_TEXT = 'Get a valid coloring';

/**
 * This class is used to represent a coloring button.
 *
 * @param coloringButton Handle to the DOM object representing the button.
 * @param {Graph} graph1 Reference to the graph associated with the button.
 * @constructor
 */
ColoringButton = function (coloringButton, graph1) {
  this.coloringButton = coloringButton;
  this.graph1 = graph1;

  this.coloringButton.disabled = true;
};

/**
 * This function disables/enables the coloring button.
 *
 * @param {boolean} disabled The value to be assigned to
 *     coloringButton.disabled.
 */
ColoringButton.prototype.setDisabled = function (isDisabled) {
  this.coloringButton.disabled = isDisabled;
};

/**
 * This function sets the text on the coloring button.
 *
 * @param {string} buttonText The text to be displayed on the coloring button.
 */
ColoringButton.prototype.setText = function (buttonText) {
  this.coloringButton.innerHTML = buttonText;
};

/**
 * This function handles the click event on the coloring button. Note that the
 * event object passed to this function is of no use here.
 *
 * @param e The Event object containing information about the click event.
 */
ColoringButton.prototype.handleColoringButtonClick = function (e) {
  canvas1.setEditMode(false);
  this.setDisabled(true);
  this.setText('Please wait...');
  naclModule1.postMessage(this.graph1.getAdjacencyMatrix().toString());
};
