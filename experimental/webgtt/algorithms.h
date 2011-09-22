// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef EXPERIMENTAL_WEBGTT_ALGORITHMS_H_
#define EXPERIMENTAL_WEBGTT_ALGORITHMS_H_

/// @fileoverview This file contains standardized functions (all of which return
/// an element of std::string type) that act as a wrapper for the graph
/// algorithms defined in graph.h The additional tasks done by these functions
/// include formatting the result obtained from the graph algorithm into the
/// response string to be sent back to the browser.
///
/// @author ragad@google.com (Raga Gopalakrishnan)

#include <string>

namespace graph {
class Graph;
}

namespace webgtt {

/// This function obtains a vertex coloring and formats it into the response
/// string to be sent back to the browser.
///
/// @param[in] input_graph A pointer to the input graph object that is used to
///     obtain the vertex coloring.
/// @return The response string to be sent back to the browser.
std::string GetColoring(const graph::Graph& input_graph);

}  // namespace webgtt

#endif  // EXPERIMENTAL_WEBGTT_ALGORITHMS_H_
