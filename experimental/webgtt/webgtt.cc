// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/// @fileoverview This file contains code for a simple prototype of the WebGTT
/// project. It demonstrates loading, running and scripting a simple NaCl
/// module, which, when given the adjacency matrix of a graph by the browser,
/// returns a valid vertex coloring of the graph. To load the NaCl module, the
/// browser first looks for the CreateModule() factory method. It calls
/// CreateModule() once to load the module code from the .nexe. After the .nexe
/// code is loaded, CreateModule() is not called again. Once the .nexe code is
/// loaded, the browser then calls the CreateInstance() method on the object
/// returned by CreateModule(). It calls CreateInstance() each time it
/// encounters an <embed> tag that references the NaCl module.
///
/// @author ragad@google.com (Raga Gopalakrishnan)

#include <boost/concept_check.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/type_traits.hpp>
#include <cmath>
#include <cstdio>
#include <limits>
#include <map>
#include <string>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

namespace {
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

/// This function is a wrapper for the Boost coloring function above. This is
/// used to abstract away the Boost usage details from HandleMessage()
///
/// @param[in] numberOfVertices The number of vertices in the graph.
/// @param[in] adjacencyMatrix The adjacency matrix of the graph.
/// @return A vector containing a valid coloring of the graph.
std::vector<int> getColoring(int numberOfVertices,
                             std::vector< std::vector<int> > adjacencyMatrix) {
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
}  // namespace

/// The Instance class. One of these exists for each instance of the NaCl module
/// on the web page. The browser will ask the Module object to create a new
/// Instance for each occurence of the <embed> tag that has these attributes:
///     type="application/x-nacl"
///     src="webgtt.nmf"
/// To communicate with the browser, the HandleMessage() method is overridden
/// for receiving messages from the browser. The PostMessage() method is used to
/// send messages back to the browser. This interface is entirely asynchronous.
class WebgttInstance : public pp::Instance {
 public:
  /// The constructor creates the plugin-side instance.
  ///
  /// @param[in] instance The handle to the browser-side plugin instance.
  /// @constructor
  explicit WebgttInstance(PP_Instance instance) : pp::Instance(instance) { }
  virtual ~WebgttInstance() { }

  /// This function handles messages coming in from the browser via
  /// postMessage().
  ///
  /// The @a var_message can contain anything: a JSON string; a string that
  /// encodes method names and arguments, etc.
  ///
  /// @param[in] var_message The message posted by the browser.
  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_string()) {
      return;
    }
    std::string message = var_message.AsString();

    // Use the knowledge of how the incoming message is formatted.
    int numberOfVertices = sqrt((message.length()+1)/2);

    std::vector< std::vector<int> > adjacencyMatrix(numberOfVertices,
        std::vector<int>(numberOfVertices));
    int i = 0;
    for (int counter_i = 0 ; counter_i < numberOfVertices ; ++counter_i) {
      for (int counter_j = 0 ; counter_j < numberOfVertices ; ++counter_j) {
        adjacencyMatrix[counter_i][counter_j] = ((message[i] == '0') ? 0 : 1);
        i += 2;
      }
    }

    // Get the coloring
    std::vector<int> vertexColors = getColoring(numberOfVertices,
                                                adjacencyMatrix);
    // Format the message to be sent back to the browser.
    std::ostringstream answer;
    for (int i = 0; i < numberOfVertices; ++i) {
      answer << vertexColors[i];
      if (i != numberOfVertices - 1) {
        answer << ',';
      }
    }
    pp::Var var_reply = pp::Var(answer.str());
    PostMessage(var_reply);
  }
};

/// The Module class. The browser calls the CreateInstance() method to create
/// an instance of the NaCl module on the web page. The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class WebgttModule : public pp::Module {
 public:
  WebgttModule() : pp::Module() { }
  virtual ~WebgttModule() { }

  /// This function creates and returns a WebgttInstance object.
  ///
  /// @param[in] instance The browser-side instance.
  /// @return The plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new WebgttInstance(instance);
  }
};

namespace pp {
  /// This is the factory function called by the browser when the module is
  /// first loaded. The browser keeps a singleton of this module. It calls the
  /// CreateInstance() method on the object that is returned to make instances.
  /// There is one instance per <embed> tag on the page. This is the main
  /// binding point for the NaCl module with the browser.
  Module* CreateModule() {
    return new WebgttModule();
  }
}  // namespace pp
