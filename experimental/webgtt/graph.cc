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
SequentialVertexColoring(const VertexListGraph& graph,
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
  const size_type number_of_vertices = boost::num_vertices(graph);
  std::vector<size_type> mark(number_of_vertices,
                              std::numeric_limits<size_type>::max());

  typename GraphTraits::vertex_iterator vertex, last_vertex;
  for (tie(vertex, last_vertex) = boost::vertices(graph); vertex != last_vertex;
       ++vertex) {
    color[*vertex] = number_of_vertices - 1;  // which means "not colored".
  }

  for (size_type i = 0; i < number_of_vertices; ++i) {
    vertex_descriptor current_vertex = order[i];
    // Mark all the colors of the adjacent vertices.
    typename GraphTraits::adjacency_iterator adjacent_vertex,
                                             last_adjacent_vertex;
    for (tie(adjacent_vertex, last_adjacent_vertex) =
         boost::adjacent_vertices(current_vertex, graph);
         adjacent_vertex != last_adjacent_vertex; ++adjacent_vertex) {
      mark[color[*adjacent_vertex]] = i;
    }
    // Find the smallest color unused by the adjacent vertices.
    size_type smallest_color = 0;
    while (smallest_color < max_color && mark[smallest_color] == i) {
      ++smallest_color;
    }
    // If all the colors are used up, increase the number of colors.
    if (smallest_color == max_color) {
      ++max_color;
    }
    color[current_vertex] = smallest_color;
  }
  return max_color;
}

Graph::Graph(int number_of_vertices,
             std::vector< std::vector<int> > adjacency_matrix)
    : number_of_vertices_(number_of_vertices),
      adjacency_matrix_(adjacency_matrix) { }

std::vector<int> Graph::GetColoring() const {
  // Construct the boost graph.
  typedef boost::adjacency_matrix<boost::undirectedS> UndirectedGraph;
  typedef boost::graph_traits<UndirectedGraph> GraphTraits;
  typedef GraphTraits::vertex_descriptor vertex_descriptor;
  typedef GraphTraits::vertex_iterator vertex_iterator;
  UndirectedGraph undirected_graph(number_of_vertices_);
  for (int i = 0; i < number_of_vertices_; ++i) {
    for (int j = i + 1; j < number_of_vertices_; ++j) {
      if (adjacency_matrix_[i][j] == 1) {
        boost::add_edge(i, j, undirected_graph);
      }
    }
  }
  // Create order as an externally stored property.
  boost::scoped_array<vertex_descriptor>
      order(new vertex_descriptor[number_of_vertices_]);
  std::pair<vertex_iterator, vertex_iterator> first_last_vertex =
      boost::vertices(undirected_graph);
  for (int i = 0; i < number_of_vertices_; ++i) {
    order[i] = first_last_vertex.first[i];
  }
  // Create color as an externally stored property.
  std::map<vertex_descriptor, int> color;
  for (int i = 0; i < number_of_vertices_; ++i) {
    color.insert(std::make_pair(vertex_descriptor(first_last_vertex.first[i]),
                                0));
  }
  // Get the coloring.
  SequentialVertexColoring(undirected_graph, order.get(),
                           boost::make_assoc_property_map(color));
  // Return the coloring.
  std::vector<int> vertex_colors(number_of_vertices_);
  for (int i = 0; i < number_of_vertices_; ++i) {
    vertex_colors[i] = color[first_last_vertex.first[i]];
  }
  return vertex_colors;
}

}  // namespace graph
