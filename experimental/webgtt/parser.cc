// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "webgtt/parser.h"

#include <boost/bind.hpp>

#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include "webgtt/graph.h"
#include "webgtt/taskmap.h"
#include "webgtt/webgtt.h"

namespace webgtt {

Parser::Parser(const std::string& message)
    : message_(message),
      is_valid_(false),
      task_ID_(kInvalidValue) { }

/// This function decodes the message. A valid message is of the form:
/// "kSentinel<adjacency_matrix_>kSentinel<task_ID_>kSentinel<args_>kSentinel"
/// If at any stage during the decoding process, the message is found to be
/// invalid (not conforming to the above format), parsing is aborted, and the
/// is_valid_ bit contains false. Upon successful completion of the decoding
/// process, the is_valid_ bit is set to true.
bool Parser::DecodeMessage() {
  assert(!is_valid_);

  const size_t message_length = message_.size();
  // We start parsing from the beginning of the message.
  int parse_position = 0;

  // The shortest valid message is for a coloring request on a graph with
  // exactly one vertex, which would look like:
  // "<kSentinel>0<kSentinel>0<kSentinel>"
  if (message_length < 2 + (3 * kSentinel.size())) {
    return false;
  }
  // Look for the first sentinel.
  if (message_.substr(0, kSentinel.size()).compare(kSentinel) != 0) {
    return false;
  }
  // Skip over the sentinel that we just found, and continue parsing.
  parse_position += kSentinel.size();

  std::string adjacency_matrix = GetNextChunk(message_, &parse_position);
  if (adjacency_matrix.compare(kSentinel) == 0) {
    return false;
  }
  // adjacency_matrix should be in CSV format; obtain the positions of commas.
  std::vector<int> comma_positions = GetCommaPositions(adjacency_matrix);
  // There should be a total of (number_of_vertices)^2 - 1 commas, since
  // adjacency_matrix is a square matrix.
  int number_of_vertices = sqrt(comma_positions.size() + 1);
  if (static_cast<int>(comma_positions.size()) !=
      (number_of_vertices * number_of_vertices) - 1) {
    return false;
  }
  // Decode the adjacency matrix
  int adj_position = 0;
  for (int i = 1; i <= number_of_vertices; ++i) {
    std::vector<int> row = DecodeCSV(adjacency_matrix.substr(adj_position,
        comma_positions[(i * number_of_vertices) - 1] - adj_position));
    for (size_t j = 0; j < row.size(); ++j) {
      if (row[j] == kInvalidValue) {
        return false;
      }
    }
    adjacency_matrix_.push_back(row);
    adj_position = comma_positions[(i * number_of_vertices) - 1] + 1;
  }
  comma_positions.clear();
  // Construct the graph
  graph::Graph input_graph(number_of_vertices, adjacency_matrix_);

  std::string task_ID = GetNextChunk(message_, &parse_position);
  if (task_ID.compare(kSentinel) == 0) {
    return false;
  }
  if (StringToInteger(task_ID) == kInvalidValue) {
    return false;
  }
  task_ID_ = StringToInteger(task_ID);

  std::string args = "";
  if (parse_position != static_cast<int>(message_length)) {
    // There may be some arguments.
    args = GetNextChunk(message_, &parse_position);
    if (args.compare(kSentinel) == 0) {
      return false;
    }
  }
  if (args.size() != 0) {
    // args should be in CSV format; decode it.
    args_ = DecodeCSV(args);
    for (size_t i = 0; i < args_.size(); ++i) {
      if (args_[i] == kInvalidValue) {
        return false;
      }
    }
  }
  int number_of_arguments = static_cast<int>(args_.size());
  if (number_of_arguments < kMaxArgs) {
    // Append enough dummy elements (zeros) to the end of args_.
    args_.insert(args_.end(), kMaxArgs - number_of_arguments, 0);
  }

  // Obtain the task_map_ lookup.
  TaskMap task_map_object(input_graph, args_);
  task_map_ = task_map_object.task_map();

  // Perform final validation checks.
  if (task_ID_ >= static_cast<int>(task_map_.size())) {
    return false;
  }
  if (number_of_arguments != task_map_[task_ID_].number_of_arguments) {
    return false;
  }

  is_valid_ = true;
  return true;
}

std::string Parser::GetResponse() const {
  assert(is_valid_);
  return task_map_[task_ID_].function_to_call();
}

std::vector<int> DecodeCSV(const std::string& message) {
  std::vector<int> answer;
  std::vector<int> comma_positions = GetCommaPositions(message);
  int parse_position = 0;
  for (size_t i = 0; i < comma_positions.size() + 1; ++i) {
    int next_comma_position;
    // There is no comma at the end of the list.
    if (i == comma_positions.size()) {
      next_comma_position = message.size();
    } else {
      next_comma_position = comma_positions[i];
    }
    std::string entry = message.substr(parse_position,
                                       next_comma_position - parse_position);
    answer.push_back(StringToInteger(entry));
    parse_position = next_comma_position + 1;
  }
  return answer;
}

std::string GetNextChunk(const std::string& message, int* parse_position) {
  assert(*parse_position < static_cast<int>(message.size()));
  unsigned int next_sentinel_position = message.find(kSentinel,
                                                     *parse_position);
  if (next_sentinel_position == std::string::npos) {
    return kSentinel;  // Return the sentinel string itself indicating error.
  }
  std::string next_chunk = message.substr(*parse_position,
      next_sentinel_position - *parse_position);
  // Update the position to continue parsing from (skip over the sentinel).
  *parse_position = next_sentinel_position + kSentinel.size();
  return next_chunk;
}

std::vector<int> GetCommaPositions(const std::string& message) {
  std::vector<int> comma_positions;
  for (std::string temp = message; temp.find(',') != std::string::npos;
       temp.replace(temp.find(','), 1, 1, '.')) {
    comma_positions.push_back(temp.find(','));
  }
  return comma_positions;
}

int StringToInteger(const std::string& message) {
  if (message.size() == 0) {
    return kInvalidValue;
  }
  if (!isdigit(message[0])) {
    return kInvalidValue;
  }
  return atoi(message.c_str());
}

}  // namespace webgtt
