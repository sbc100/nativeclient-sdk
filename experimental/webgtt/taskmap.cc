// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "webgtt/taskmap.h"

#include <boost/bind.hpp>

#include <vector>

#include "webgtt/graph.h"
#include "webgtt/webgtt.h"

namespace webgtt {
TaskMap::TaskMap(const graph::Graph& inputGraph, const std::vector<int>& args) {
  assert(static_cast<int>(args.size()) >= kMaxArgs);
  // taskID 0 : Graph coloring : no arguments needed.
  FunctionInfo coloringFunctionInfo;
  coloringFunctionInfo.numberOfArguments = 0;
  coloringFunctionInfo.functionToCall = boost::bind(getColoring, inputGraph);
  task_map_.push_back(coloringFunctionInfo);
}

std::vector<FunctionInfo> TaskMap::getTaskMap(void) {
  return task_map_;
}
}  // namespace webgtt
