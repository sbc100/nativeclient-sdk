// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef EXPERIMENTAL_WEBGTT_TASKMAP_H_
#define EXPERIMENTAL_WEBGTT_TASKMAP_H_

/// @fileoverview This file provides a utility class which instantiates a lookup
/// vector, each element of which provides information about the function to be
/// called, and the number of arguments needed for that function call, for a
/// specific taskID (that corresponds to a specific graph algorithm).
///
/// @author ragad@google.com (Raga Gopalakrishnan)

#include <boost/function.hpp>

#include <string>
#include <vector>

namespace graph {
class Graph;
}

namespace webgtt {

/// This constant specifies the maximum number of arguments that a graph
/// algorithm will ever need.
const int kMaxArgs = 10;

/// This structure stores the information required to call a function that
/// implements a particular graph algorithm.
struct FunctionInfo {
  /// The number of arguments that the graph algorithm would need to work.
  int number_of_arguments;
  /// The function that obtains the response string containing the formatted
  /// output of the graph algorithm.
  boost::function<std::string()> function_to_call;
};

/// The TaskMap class. This class initializes a vector of instances of the
/// FunctionInfo structure - one instance per task ID. The function call for a
/// particular task ID needs to be customized based on the input graph, and the
/// arguments, so the constructor takes these as parameters.
class TaskMap {
 public:
  /// The constructor takes in an input graph and a vector of arguments, and
  /// initializes the lookup vector of FunctionInfo instances.
  ///
  /// @param[in] input_graph The input graph to be used in the function
  ///     bindings.
  /// @param[in] args The vector of arguments from which the arguments to be
  ///     used in the function bindings are obtained.
  /// @constructor
  TaskMap(const graph::Graph& input_graph, const std::vector<int>& args);

  /// Accessor function for task_map.
  const std::vector<FunctionInfo>& task_map() const { return task_map_; }

 private:
  std::vector<FunctionInfo> task_map_;
  /// This disallows usage of copy and assignment constructors.
  TaskMap(const TaskMap&);
  void operator=(const TaskMap&);
};

}  // namespace webgtt

#endif  // EXPERIMENTAL_WEBGTT_TASKMAP_H_
