// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the JavaScript required for the WebGTT
 * application, specifically, the implementation of the Edge class. This
 * implementation provides functions for creating, selecting, coloring, and
 * drawing an edge.
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * This constant holds the size used to draw an edge.
 * @type {number}
 * @const
 */
var EDGE_SIZE = 4;

/**
 * This class is used to represent an edge of the graph.
 *
 * @param {Vertex} start Reference to the object representing the start vertex.
 * @param {Vertex} end Reference to the object representing the end vertex.
 * @constructor
 */
Edge = function (start, end) {
  this.start = start;
  this.end = end;

  /**
   * This boolean variable indicates whether the edge is selected or not.
   */
  this.isSelected = false;

  /**
   * This variable stores the color used while drawing the edge on the canvas.
   */
  this.color = 'black';
};

/**
 * This function sets the value of Edge.isSelected.
 *
 * @param {boolean} isSelected The value to be assigned to Edge.isSelected.
 */
Edge.prototype.setSelected = function (isSelected) {
  this.isSelected = isSelected;
};

/**
 * This function sets the value of Edge.color.
 *
 * @param {string} color The value to be assigned to Edge.color.
 */
Edge.prototype.setColor = function (color) {
    this.color = color;
};

/**
 * This function draws the edge on the canvas, with a specified color and
 * line width.
 *
 * @param drawingContext The handle to the drawing surface of the canvas.
 * @param {string} color The color with which the edge is to be drawn.
 * @param {number} lineWidth The width with which the edge is to be drawn.
 */
Edge.prototype.drawSpecific = function (drawingContext, color, lineWidth) {
  drawingContext.strokeStyle = color;
  drawingContext.lineWidth = lineWidth;
  drawingContext.beginPath();
  drawingContext.moveTo(this.start.x, this.start.y);
  drawingContext.lineTo(this.end.x, this.end.y);
  drawingContext.closePath();
  drawingContext.stroke();
};

/**
 * This function draws the edge on the canvas.
 *
 * The edge is drawn as a line from (start.x,start.y) to (end.x,end.y) with
 * size (thickness) specified by EDGE_SIZE. If isSelected is true, a thicker
 * red line is also drawn, to indicate that the edge is selected.
 *
 * @param drawingContext The handle to the drawing surface of the canvas.
 */
Edge.prototype.draw = function (drawingContext) {
  this.drawSpecific(drawingContext, this.color, EDGE_SIZE);
  if (this.isSelected) {
    this.drawSpecific(drawingContext, 'red', 3*EDGE_SIZE/2);
  }
};
