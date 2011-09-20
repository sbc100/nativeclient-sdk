// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef EXPERIMENTAL_WEBGTT_WEBGTT_H_
#define EXPERIMENTAL_WEBGTT_WEBGTT_H_

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

#endif  // EXPERIMENTAL_WEBGTT_WEBGTT_H_
