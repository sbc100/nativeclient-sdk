// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef EXPERIMENTAL_WEBGTT_GRAPH_H_
#define EXPERIMENTAL_WEBGTT_GRAPH_H_

/// @fileoverview This file provides an implementation of a Graph data
/// structure, which constructs a graph given the number of vertices and the
/// adjacency matrix. It implements the graph algorithms required by webgtt.cc,
/// which handles browser communication. Basically, this data structure can be
/// thought of as a wrapper for Boost's Graph interface (we internally use
/// Boost's graph representation and algorithms). At the moment, only
/// undirected, unweighted graphs are supported.
///
/// @author ragad@google.com (Raga Gopalakrishnan)

#include <boost/graph/graph_traits.hpp>
#include <vector>

namespace graph {
/// This function extends the Boost library of graph algorithms. It finds a
/// coloring of the graph, given the adjacency matrix.
///
/// Definition: A valid coloring of a graph consists of a color for each
/// vertex of the graph, such that no two vertices that are connected by an
/// edge share the same color. A minimum coloring is a valid coloring that
/// uses as few colors as possible.
/// Algorithm Choice: Finding a minimum coloring is an NP-complete problem,
/// and therefore, in this algorithm, we focus on finding a valid coloring.
/// Of course, assigning each vertex a different color is a trivial valid
/// coloring! But we want to do something better, that would use far fewer
/// colors in most cases. We use a greedy coloring technique, and the Boost
/// implementation of this technique, outlined at
/// http://www.boost.org/doc/libs/1_47_0/libs/graph/doc/constructing_algorithms.html
///
/// @param[in] graph The graph object.
/// @param[in] order A property map object that maps vertex indices to their
///                  corresponding vertex descriptors.
/// @param[in] color A property map object that maps vertex descriptors to
///                  their corresponding vertex colors.
/// @return The number of colors used to color the graph.
template <class VertexListGraph, class Order, class Color>
typename boost::graph_traits<VertexListGraph>::vertices_size_type
sequential_vertex_coloring(const VertexListGraph& graph,
                           Order order, const Color& color);

/// The Graph class. This class is a wrapper for Boost's graph representation
/// and algorithms.
class Graph {
 public:
  /// The constructor creates a Graph object given the number of vertices and
  /// the adjacency matrix.
  ///
  /// @param[in] numberOfVertices The number of vertices in the graph.
  /// @param[in] adjacencyMatrix The adjacency matrix of the graph.
  /// @constructor
  explicit Graph(int numberOfVertices,
                 std::vector< std::vector<int> > adjacencyMatrix);

  /// This function returns a greedy vertex coloring of the graph. Internally,
  /// it uses sequential_vertex_coloring, an algorithm provided here as an
  /// extension of Boost's graph algorithms.
  ///
  /// @param[in] numberOfVertices The number of vertices in the graph.
  /// @param[in] adjacencyMatrix The adjacency matrix of the graph.
  /// @return A vector containing a valid coloring of the graph.
  std::vector<int> getColoring(void);

 private:
  int numberOfVertices;
  std::vector< std::vector<int> > adjacencyMatrix;
};
}  // namespace graph

#endif  // EXPERIMENTAL_WEBGTT_GRAPH_H_
