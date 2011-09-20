// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "webgtt/taskmap.h"

#include <boost/bind.hpp>

#include <vector>

#include "webgtt/graph.h"
#include "webgtt/webgtt.h"

namespace webgtt {

TaskMap::TaskMap(const graph::Graph& input_graph,
                 const std::vector<int>& args) {
  assert(static_cast<int>(args.size()) >= kMaxArgs);
  // taskID 0 : Graph coloring : no arguments needed.
  FunctionInfo coloring_function_info;
  coloring_function_info.number_of_arguments = 0;
  coloring_function_info.function_to_call = boost::bind(GetColoring,
                                                        input_graph);
  task_map_.push_back(coloring_function_info);
}

}  // namespace webgtt
