// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the JavaScript required for the WebGTT
 * application, specifically, the implementation of the Vertex class. This
 * implementation provides functions for creating, selecting, coloring, and
 * drawing a vertex.
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * This constant holds the size used to draw a vertex.
 * @type {number}
 * @const
 */
var VERTEX_SIZE = 6;

/**
 * This class is used to represent a vertex of the graph.
 *
 * @param {number} x The x-coordinate of the center of the vertex relative to
 *     the canvas.
 * @param {number} y The y-coordinate of the center of the vertex relative to
 *     the canvas.
 * @constructor
 */
Vertex = function (x, y) {
  this.x = x;
  this.y = y;

  /**
   * This boolean variable indicates whether the vertex is selected or not.
   */
  this.isSelected = false;

  /**
   * This variable stores the color used while drawing the vertex on the canvas.
   */
  this.color = 'black';
};

/**
 * This function sets the value of Vertex.isSelected.
 *
 * @param {boolean} isSelected The value to be assigned to Vertex.isSelected.
 */
Vertex.prototype.setSelected = function (isSelected) {
  this.isSelected = isSelected;
};

/**
 * This function sets the value of Vertex.color.
 *
 * @param {string} color The value to be assigned to Vertex.color.
 */
Vertex.prototype.setColor = function (color) {
    this.color = color;
};

/**
 * This function draws the vertex on the canvas, with a specified color and
 * radius.
 *
 * @param {object} drawingContext The handle to the drawing surface of the
 *     canvas.
 * @param {string} color The color with which the vertex is to be drawn.
 * @param {number} radius The radius with which the vertex is to be drawn.
 */
Vertex.prototype.drawSpecific = function (drawingContext, color, radius) {
  drawingContext.fillStyle = color;
  drawingContext.beginPath();
  drawingContext.arc(this.x, this.y, radius, 0, Math.PI * 2, false);
  drawingContext.closePath();
  drawingContext.fill();
};

/**
 * This function draws the vertex on the canvas.
 *
 * The vertex is drawn as a solid circle centered at (x,y) with radius
 * specified by VERTEX_SIZE. If isSelected is true, a bigger red circle is
 * also drawn, to indicate that the vertex is selected.
 *
 * @param {object} drawingContext The handle to the drawing surface of the
 *     canvas.
 */
Vertex.prototype.draw = function (drawingContext) {
  this.drawSpecific(drawingContext, this.color, VERTEX_SIZE);
  if (this.isSelected) {
    this.drawSpecific(drawingContext, 'red', 3*VERTEX_SIZE/2);
  }
};
