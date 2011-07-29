// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the JavaScript required for the WebGTT
 * application, specifically, the implementation of the Graph class. This
 * implementation provides functions for creating, directing and drawing a
 * graph, adding/deleting vertices/edges, checking whether an edge exists,
 * obtaining the vertex that is nearest to a given point on the canvas, and
 * obtaining the adjacency matrix.
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * This class is used to represent a graph.
 * @constructor
 */
Graph = function () {
  /**
   * These variables are arrays that hold references to the vertices and edges
   * that make up the graph.
   */
  this.listOfVertices = new Array();
  this.listOfEdges = new Array();

  /**
   * This boolean variable indicates whether the graph is directed or not.
   */
  this.isDirected = false;
};

/**
 * This function sets the value of Graph.isDirected.
 *
 * If the graph is being converted from a directed to an undirected graph, then
 * possible duplicate edges should be deleted.
 *
 * @param {boolean} isDirected The value to be assigned to Graph.isDirected.
 */
Graph.prototype.setDirected = function (isDirected) {
  if (isDirected) {
    this.isDirected = true;
  } else if(this.isDirected) {
    this.isDirected = false;
    for (var i = 0 ; i < this.listOfEdges.length - 1 ; i++) {
      for (var j = i+1 ; j < this.listOfEdges.length ; j++) {
        if (this.listOfEdges[i].start == this.listOfEdges[j].end &&
            this.listOfEdges[i].end == this.listOfEdges[j].start) {
          this.deleteEdgeByIndex(j);
          break;
        }
      }
    }
  }
};

/**
 * This function adds a vertex to the graph.
 *
 * @param {Vertex} newVertex Reference to the new vertex to be added.
 */
Graph.prototype.addVertex = function (newVertex) {
  this.listOfVertices.push(newVertex);
};

/**
 * This function deletes a vertex from the graph, given its index in
 * Graph.listOfVertices.
 *
 * @param {number} vertexIndex The index of the vertex to be deleted.
 */
Graph.prototype.deleteVertexByIndex = function (vertexIndex) {
  // First, delete all edges from/to this vertex
  for (var i = 0 ; i < this.listOfEdges.length ; i++) {
    if (this.listOfEdges[i].start == this.listOfVertices[vertexIndex] ||
        this.listOfEdges[i].end == this.listOfVertices[vertexIndex]) {
      this.deleteEdgeByIndex(i);
      i--;
    }
  }

  for (var i = vertexIndex ; i < this.listOfVertices.length - 1 ; i++) {
    this.listOfVertices[i] = this.listOfVertices[i+1];
  }
  this.listOfVertices.pop();
};

/**
 * This function returns the vertex nearest to the given coordinates.
 *
 * @param {number} x The given x-coordinate.
 * @param {number} y The given y-coordinate.
 * @return {Vertex} Reference to the nearest vertex (undefined if none).
 */
Graph.prototype.getNearestVertex = function (x, y) {
  var closestVertex = undefined;
  var closestDistance = 0;
  if (graph1.listOfVertices.length != 0) {
    closestVertex = graph1.listOfVertices[0];
    closestDistance = Math.sqrt(Math.pow(x-closestVertex.x,2) +
                                Math.pow(y-closestVertex.y,2));
    for (var i = 1 ; i < graph1.listOfVertices.length ; i++) {
      var tempVertex = graph1.listOfVertices[i];
      var tempDistance = Math.sqrt(Math.pow(x-tempVertex.x,2) +
                                   Math.pow(y-tempVertex.y,2));
      if (tempDistance < closestDistance) {
        closestVertex = tempVertex;
        closestDistance = tempDistance;
      }
    }
  }
  return closestVertex;
};

/**
 * This function adds an edge to the graph.
 *
 * @param {Edge} newEdge Reference to the new edge to be added.
 */
Graph.prototype.addEdge = function (newEdge) {
  this.listOfEdges.push(newEdge);
};

/**
 * This function deletes an edge from the graph, given its index in
 * Graph.listOfEdges.
 *
 * @param {number} edgeIndex The index of the edge to be deleted.
 */
Graph.prototype.deleteEdgeByIndex = function (edgeIndex) {
  for (var i = edgeIndex ; i < this.listOfEdges.length - 1 ; i++) {
    this.listOfEdges[i] = this.listOfEdges[i+1];
  }
  this.listOfEdges.pop();
};

/**
 * This function returns a reference to the edge that is adjacent to the given
 * vertices.
 *
 * @param {Vertex} start Reference to the start vertex.
 * @param {Vertex} end Reference to the end vertex.
 * @return {Edge} Reference to the adjacent edge (undefined if none).
 */
Graph.prototype.getEdge = function (start, end) {
  var returnEdge = undefined;

  // This is slightly complex, since the graph could be directed or undirected.
  for (var i = 0 ; i < this.listOfEdges.length ; i++) {
    if (this.listOfEdges[i].start == start && this.listOfEdges[i].end == end) {
      returnEdge = this.listOfEdges[i];
      break;
    }
    if (this.listOfEdges[i].start == end && this.listOfEdges[i].end == start) {
      if (!this.isDirected) {
        returnEdge = this.listOfEdges[i];
        break;
      }
    }
  }
  return returnEdge;
};

/**
 * This function draws the given list of elements (vertices or edges) on the
 * canvas.
 *
 * @param drawingContext The handle to the drawing surface of the canvas.
 * @param {Array.<Vertex/Edge>} listOfElements The list of elements (vertices
 *     or edges) to be drawn on to the canvas.
 */
Graph.prototype.drawSpecific = function (drawingContext, listOfElements) {
  for (var i = 0 ; i < listOfElements.length ; i++) {
    listOfElements[i].draw(drawingContext);
  }
};

/**
 * This function draws the graph on the canvas.
 *
 * @param drawingContext The handle to the drawing surface of the canvas.
 */
Graph.prototype.draw = function (drawingContext) {
  drawingContext.globalCompositeOperation = 'destination-over';
  this.drawSpecific(drawingContext, this.listOfVertices);
  this.drawSpecific(drawingContext, this.listOfEdges);
};

/** This function returns the adjacency matrix of the graph.
 *
 * @return {Array.<Array.<number>>} The adjacency matrix of the graph.
 */
Graph.prototype.getAdjacencyMatrix = function () {
  var adjacencyMatrix = new Array();
  for (var i = 0 ; i < this.listOfVertices.length ; i++) {
    adjacencyMatrix.push(new Array());
  }

  for (var i = 0 ; i < this.listOfVertices.length ; i++) {
    for (var j = 0 ; j < this.listOfVertices.length ; j++) {
      adjacencyMatrix[i][j] = 0;
    }
  }

  for (var i = 0 ; i < this.listOfEdges.length ; i++) {
    var indexOfStart = this.listOfVertices.indexOf(this.listOfEdges[i].start);
    var indexOfEnd = this.listOfVertices.indexOf(this.listOfEdges[i].end);
    adjacencyMatrix[indexOfStart][indexOfEnd] = 1;
    if (!this.isDirected) {
      adjacencyMatrix[indexOfEnd][indexOfStart] = 1;
    }
  }

  return adjacencyMatrix;
};
