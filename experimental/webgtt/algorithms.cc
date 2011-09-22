// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "webgtt/algorithms.h"

#include <sstream>
#include <vector>

#include "webgtt/graph.h"

namespace webgtt {

std::string GetColoring(const graph::Graph& input_graph) {
  std::vector<int> vertex_colors = input_graph.GetColoring();
  // Format the message to be sent back to the browser.
  std::ostringstream answer;
  for (size_t i = 0; i < vertex_colors.size(); ++i) {
    answer << vertex_colors[i];
    if (i != vertex_colors.size() - 1) {
      answer << ',';
    }
  }
  return answer.str();
}

}  // namespace webgtt
