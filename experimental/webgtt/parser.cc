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
/// isValid bit contains false. Upon successful completion of the decoding
/// process, the isValid bit is set to true.
bool Parser::decodeMessage(void) {
  assert(!is_valid_);

  int messageLength = message_.size();
  // We start parsing from the beginning of the message.
  int parsePosition = 0;

  // The shortest valid message is for a coloring request on a graph with
  // exactly one vertex, which would look like:
  // "<kSentinel>0<kSentinel>0<kSentinel>"
  if (messageLength < 2 + (3 * static_cast<int>(kSentinel.size()))) {
    return false;
  }
  // Look for the first sentinel.
  if (message_.substr(0, kSentinel.size()).compare(kSentinel) != 0) {
    return false;
  }
  // Skip over the sentinel that we just found, and continue parsing.
  parsePosition += kSentinel.size();

  std::string adjacencyMatrix = getNextChunk(message_, &parsePosition);
  if (adjacencyMatrix.compare(kSentinel) == 0) {
    return false;
  }
  // adjacencyMatrix should be in CSV format; obtain the positions of commas.
  std::vector<int> commaPositions = getCommaPositions(adjacencyMatrix);
  // There should be a total of (numberOfVertices)^2 - 1 commas, since
  // adjacencyMatrix is a square matrix.
  int numberOfVertices = sqrt(commaPositions.size() + 1);
  if (static_cast<int>(commaPositions.size()) !=
      (numberOfVertices * numberOfVertices) - 1) {
    return false;
  }
  // Decode the adjacency matrix
  int adjPosition = 0;
  for (int i = 1; i <= numberOfVertices; ++i) {
    std::vector<int> row = decodeCSV(adjacencyMatrix.substr(adjPosition,
        commaPositions[(i * numberOfVertices) - 1] - adjPosition));
    for (int j = 0; j < static_cast<int>(row.size()); ++j) {
      if (row[j] == kInvalidValue) {
        return false;
      }
    }
    adjacency_matrix_.push_back(row);
    adjPosition = commaPositions[(i * numberOfVertices) - 1] + 1;
  }
  commaPositions.clear();
  // Construct the graph
  graph::Graph inputGraph(numberOfVertices, adjacency_matrix_);

  std::string taskID = getNextChunk(message_, &parsePosition);
  if (taskID.compare(kSentinel) == 0) {
    return false;
  }
  if (strtoi(taskID) == kInvalidValue) {
    return false;
  }
  task_ID_ = strtoi(taskID);

  std::string args = "";
  if (parsePosition != messageLength) {
    // There may be some arguments.
    args = getNextChunk(message_, &parsePosition);
    if (args.compare(kSentinel) == 0) {
      return false;
    }
  }
  if (args.size() != 0) {
    // args should be in CSV format; decode it.
    args_ = decodeCSV(args);
    for (int i = 0; i < static_cast<int>(args_.size()); ++i) {
      if (args_[i] == kInvalidValue) {
        return false;
      }
    }
  }
  int numberOfArguments = static_cast<int>(args_.size());
  if (numberOfArguments < kMaxArgs) {
    // Append enough dummy elements (zeros) to the end of args_.
    args_.insert(args_.end(), kMaxArgs - numberOfArguments, 0);
  }

  // Obtain the task_map_ lookup.
  TaskMap taskMapObject(inputGraph, args_);
  task_map_ = taskMapObject.getTaskMap();

  // Perform final validation checks.
  if (task_ID_ >= static_cast<int>(task_map_.size())) {
    return false;
  }
  if (numberOfArguments != task_map_[task_ID_].numberOfArguments) {
    return false;
  }

  is_valid_ = true;
  return true;
}

std::string Parser::getResponse(void) {
  assert(is_valid_);
  return task_map_[task_ID_].functionToCall();
}

std::vector<int> decodeCSV(const std::string& message) {
  std::vector<int> answer;
  std::vector<int> commaPositions = getCommaPositions(message);
  int parsePosition = 0;
  for (int i = 0; i < static_cast<int>(commaPositions.size()) + 1; ++i) {
    int nextCommaPosition;
    // There is no comma at the end of the list.
    if (i == static_cast<int>(commaPositions.size())) {
      nextCommaPosition = message.size();
    } else {
      nextCommaPosition = commaPositions[i];
    }
    std::string entry = message.substr(parsePosition,
                                       nextCommaPosition - parsePosition);
    answer.push_back(strtoi(entry));
    parsePosition = nextCommaPosition + 1;
  }
  return answer;
}

std::string getNextChunk(const std::string& message, int* parsePosition) {
  assert(*parsePosition < static_cast<int>(message.size()));
  unsigned int nextSentinelPosition = message.find(kSentinel, *parsePosition);
  if (nextSentinelPosition == std::string::npos) {
    return kSentinel;  // Return the sentinel string itself indicating error.
  }
  std::string nextChunk = message.substr(*parsePosition,
                                         nextSentinelPosition - *parsePosition);
  // Update the position to continue parsing from. Move it to skip over the
  // sentinel.
  *parsePosition = nextSentinelPosition + kSentinel.size();
  return nextChunk;
}

std::vector<int> getCommaPositions(const std::string& message) {
  std::vector<int> commaPositions;
  for (std::string temp = message; temp.find(',') != std::string::npos;
       temp.replace(temp.find(','), 1, 1, '.')) {
    commaPositions.push_back(temp.find(','));
  }
  return commaPositions;
}

int strtoi(const std::string& message) {
  if (message.size() == 0) {
    return kInvalidValue;
  }
  if (!isdigit(message[0])) {
    return kInvalidValue;
  }
  return atoi(message.c_str());
}
}  // namespace webgtt
