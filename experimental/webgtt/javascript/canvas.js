// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the JavaScript required for the WebGTT
 * application, specifically, the implementation of the Canvas class. This
 * implementation provides functions for manipulating a canvas, and
 * adding/managing event handlers for a canvas.
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * This constant holds the thickness of the canvas border.
 * @type {number}
 * @const
 */
var BORDER_THICKNESS = 5;

/**
 * This class is used to represent a canvas.
 *
 * @param drawingCanvas Handle to the DOM object representing the canvas.
 * @param {Graph} graph1 Reference to the graph associated with the canvas.
 * @constructor
 */
Canvas = function (drawingCanvas, graph1) {
  this.drawingCanvas = drawingCanvas;
  this.graph1 = graph1;

  /**
   * This variable is the handle to the drawing surface of the canvas.
   */
  this.drawingContext = this.drawingCanvas.getContext('2d');

  /**
   * This boolean variable indicates whether the canvas/graph is editable or
   * not.
   */
  this.editMode = true;

  this.redrawCanvas();
};

/**
 * This function (re)draws the canvas.
 */
Canvas.prototype.redrawCanvas = function () {
  // The easiest way to reset the canvas is to set one of its properties.
  this.drawingCanvas.width = this.drawingCanvas.width;

  this.drawingContext.lineWidth = BORDER_THICKNESS;
  this.drawingContext.strokeRect(0, 0, this.drawingCanvas.width,
                                 this.drawingCanvas.height);
  this.graph1.draw(this.drawingContext);
};

/**
 * This function sets the value of Canvas.editMode.
 *
 * @param {boolean} editMode The value to be assigned to Canvas.editMode.
 */
Canvas.prototype.setEditMode = function (editMode) {
  this.editMode = editMode;
};

/**
 * This function handles the click event on the canvas.
 *
 * When the user clicks on the canvas, there are several possibilities to be
 * handled. If the user clicked sufficiently far away from any existing vertex,
 * then a new vertex should be created there. If the user clicked too close to
 * an existing vertex, that vertex should be selected/deselected. If at this
 * point, two vertices are selected, and an edge doesn't exist between them, a
 * new edge should be created. If an edge does exist, then that edge should be
 * selected/deselected. Finally, the control panel (the coloring button) should
 * be enabled only if there is at least one vertex on the canvas.
 *
 * @param clickEvent The Event object containing information about the click
 *     event.
 */
Canvas.prototype.handleCanvasClick = function (clickEvent) {
  if (this.editMode) {
    // The origin of the coordinate system is at the top left. The x-coordinate
    // increases while moving horizontally right, and the y-coordinate increases
    // while moving vertically down.
    var x;
    var y;
    if (clickEvent.pageX || clickEvent.pageY) {
      // pageX and pageY give the coordinates relative to the document
      x = clickEvent.pageX;
      y = clickEvent.pageY;
    } else {
      // Our old friend, Internet Explorer does not support pageX and pageY
      // properties, so we calculate it manually
      x = clickEvent.clientX + document.body.scrollLeft +
          document.documentElement.scrollLeft;
      y = clickEvent.clientY + document.body.scrollTop +
          document.documentElement.scrollTop;
    }
    x -= this.drawingCanvas.offsetLeft;
    y -= this.drawingCanvas.offsetTop;

    var shouldRedraw = false;

    var closestVertex = this.graph1.getNearestVertex(x,y);
    var closestDistance = undefined;
    if (closestVertex != undefined) {
      closestDistance = Math.sqrt(Math.pow(x-closestVertex.x,2) +
                                  Math.pow(y-closestVertex.y,2));
    }

    if (closestVertex == undefined || closestDistance > VERTEX_SIZE * 4) {
      this.graph1.addVertex(new Vertex(x, y));
      shouldRedraw = true;
    } else if (closestDistance < VERTEX_SIZE+3) {
      closestVertex.setSelected(!closestVertex.isSelected);
      shouldRedraw = true;
      if (closestVertex.isSelected) {
        // Check if another vertex is selected.
        for (var i = 0 ; i < this.graph1.listOfVertices.length ; i++) {
          if (this.graph1.listOfVertices[i] != closestVertex &&
             this.graph1.listOfVertices[i].isSelected) {
            var tempEdge = this.graph1.getEdge(this.graph1.listOfVertices[i],
                                               closestVertex);
            if (tempEdge != undefined) {
              tempEdge.setSelected(!tempEdge.isSelected);
            } else {
              this.graph1.addEdge(new Edge(this.graph1.listOfVertices[i],
                                           closestVertex));
            }
            this.graph1.listOfVertices[i].setSelected(false);
            closestVertex.setSelected(false);
            break;
          }
        }
      }
    }

    if (shouldRedraw) {
      this.redrawCanvas();
    }

    if ((this.graph1.listOfVertices.length > 0) && (naclModule1 != undefined)) {
      coloringButton1.setDisabled(false);
    } else {
      coloringButton1.setDisabled(true);
    }
  }
};

/**
 * This function handles the keydown event on the window.
 *
 * When the user presses any key on the keyboard, this function first checks if
 * the key that was pressed was a 'Delete' key, and if the canvas/graph is
 * editable. If both these conditions are satisfied, then the selected vertices
 * and/or edges are deleted. Finally, the control panel (the coloring button)
 * should be enabled only if there is at least one vertex on the canvas.
 *
 * @param keydownEvent The Event object containing information about the
 *     keydown event.
 */
Canvas.prototype.handleCanvasKeydown = function (keydownEvent) {
  if (keydownEvent.keyCode == 46 && this.editMode) {
    var shouldredraw = false;
    this.setEditMode(false);

    for (var i = 0 ; i < this.graph1.listOfVertices.length ; i++) {
      if (this.graph1.listOfVertices[i].isSelected) {
        this.graph1.deleteVertexByIndex(i);
        // Deleting a vertex moves the next vertex in the array back by a
        // position, so the counter has to be decremented to account for that.
        i--;
        shouldRedraw = true;
      }
    }

    for (var i = 0 ; i < this.graph1.listOfEdges.length ; i++) {
      if (this.graph1.listOfEdges[i].isSelected) {
        this.graph1.deleteEdgeByIndex(i);
        // Deleting an edge moves the next edge in the array back by a position,
        // so the counter has to be decremented to account for that.
        i--;
        shouldRedraw = true;
      }
    }

    this.setEditMode(true);

    if (shouldRedraw) {
      this.redrawCanvas();
      if (this.graph1.listOfVertices.length > 0) {
        coloringButton1.setDisabled(false);
      } else {
        coloringButton1.setDisabled(true);
      }
    } else {
      alert("Nothing to delete!");
    }
  }
};
