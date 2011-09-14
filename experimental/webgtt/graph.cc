// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "webgtt/graph.h"
#include <boost/concept_check.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/type_traits.hpp>
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace graph {
template <class VertexListGraph, class Order, class Color>
typename boost::graph_traits<VertexListGraph>::vertices_size_type
sequential_vertex_coloring(const VertexListGraph& graph,
                           Order order, const Color& color) {
  typedef boost::graph_traits<VertexListGraph> GraphTraits;
  typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
  typedef typename GraphTraits::vertices_size_type size_type;
  typedef typename boost::property_traits<Color>::value_type ColorType;
  typedef typename boost::property_traits<Order>::value_type OrderType;

  boost::function_requires< boost::VertexListGraphConcept<VertexListGraph> >();
  boost::function_requires< boost::ReadWritePropertyMapConcept<Color,
      vertex_descriptor> >();
  boost::function_requires< boost::IntegerConcept<ColorType> >();
  boost::function_requires< boost::ReadablePropertyMapConcept<Order,
      size_type> >();
  BOOST_STATIC_ASSERT((boost::is_same<OrderType, vertex_descriptor>::value));

  size_type max_color = 0;
  const size_type numberOfVertices = boost::num_vertices(graph);
  std::vector<size_type> mark(numberOfVertices,
                              std::numeric_limits<size_type>::max());

  typename GraphTraits::vertex_iterator vertex, lastVertex;
  for (tie(vertex, lastVertex) = boost::vertices(graph); vertex != lastVertex;
       ++vertex) {
    color[*vertex] = numberOfVertices - 1;  // which means "not colored"
  }

  for (size_type i = 0; i < numberOfVertices; ++i) {
    vertex_descriptor currentVertex = order[i];
    // mark all the colors of the adjacent vertices
    typename GraphTraits::adjacency_iterator adjacentVertex, lastAdjacentVertex;
    for (tie(adjacentVertex, lastAdjacentVertex) =
         boost::adjacent_vertices(currentVertex, graph);
         adjacentVertex != lastAdjacentVertex; ++adjacentVertex) {
      mark[color[*adjacentVertex]] = i;
    }
    // find the smallest color unused by the adjacent vertices
    size_type smallest_color = 0;
    while (smallest_color < max_color && mark[smallest_color] == i) {
      ++smallest_color;
    }
    // if all the colors are used up, increase the number of colors
    if (smallest_color == max_color) {
      ++max_color;
    }
    color[currentVertex] = smallest_color;
  }
  return max_color;
}

Graph::Graph(int numberOfVertices,
             std::vector< std::vector<int> > adjacencyMatrix) {
  this->numberOfVertices = numberOfVertices;
  this->adjacencyMatrix = adjacencyMatrix;
}

std::vector<int> Graph::getColoring(void) {
  // Construct boost graph
  typedef boost::adjacency_matrix<boost::undirectedS> UndirectedGraph;
  typedef boost::graph_traits<UndirectedGraph> GraphTraits;
  typedef GraphTraits::vertex_descriptor vertex_descriptor;
  typedef GraphTraits::vertex_iterator vertex_iterator;
  UndirectedGraph undirectedGraph(numberOfVertices);
  for (int i = 0; i < numberOfVertices; ++i) {
    for (int j = i + 1; j < numberOfVertices; ++j) {
      if (adjacencyMatrix[i][j] == 1) {
        boost::add_edge(i, j, undirectedGraph);
      }
    }
  }
  // Create order as an externally stored property
  boost::scoped_array<vertex_descriptor>
      order(new vertex_descriptor[numberOfVertices]);
  std::pair<vertex_iterator, vertex_iterator> firstLastVertex =
      boost::vertices(undirectedGraph);
  for (int i = 0; i < numberOfVertices; ++i) {
    order[i] = firstLastVertex.first[i];
  }
  // Create color as an externally stored property
  std::map<vertex_descriptor, int> color;
  for (int i = 0; i < numberOfVertices; ++i) {
    color.insert(std::make_pair(vertex_descriptor(firstLastVertex.first[i]),
                                0));
  }
  // Get the coloring
  sequential_vertex_coloring(undirectedGraph, order.get(),
                             boost::make_assoc_property_map(color));
  // Return the coloring
  std::vector<int> vertexColors(numberOfVertices);
  for (int i = 0; i < numberOfVertices; ++i) {
    vertexColors[i] = color[firstLastVertex.first[i]];
  }
  return vertexColors;
}
}  // namespace graph
