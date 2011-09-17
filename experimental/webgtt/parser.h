// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef EXPERIMENTAL_WEBGTT_PARSER_H_
#define EXPERIMENTAL_WEBGTT_PARSER_H_

/// @fileoverview This file provides a helper class with functions that parse
/// and validate the message received by the NaCl module, decode the same, and
/// obtain the appropriate response to be sent back to the browser.
///
/// @author ragad@google.com (Raga Gopalakrishnan)

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <string>
#include <vector>

#include "webgtt/graph.h"
#include "webgtt/taskmap.h"

namespace webgtt {
/// The sentinel/delimiter string that is used in the message format.
const std::string kSentinel = "::";

/// The integer value that represents an invalid value.
const int kInvalidValue = -1;

/// The Parser class. This class provides an interface for validating a given
/// message the provided message and parse it to decode the message into its
/// constituent entities. In addition, it also provides a function that obtains
/// the appropriate response to be sent back to the browser.
class Parser {
 public:
  /// The constructor takes in a message, and sets default values to its class
  /// members.
  ///
  /// @param[in] message The input message to be parsed/validated.
  /// @constructor
  explicit Parser(const std::string& message);

  /// This function starts decoding the message into its constituent components
  /// (adjacency matrix, task ID, list of arguments). During this process, if
  /// the message is found to be invalid, parsing is aborted, and the is_valid_
  /// bit would contain false. Upon successful completion, the is_valid_ bit is
  /// set to true.
  ///
  /// @return false if an error was encountered, true otherwise.
  bool decodeMessage(void);

  /// This function returns the response string to be sent back to the browser.
  ///
  /// This function should be used only when is_valid_ is true.
  ///
  /// @return The response string to be sent back to the browser.
  std::string getResponse(void);

 private:
  std::string message_;
  bool is_valid_;
  std::vector< std::vector<int> > adjacency_matrix_;
  int task_ID_;
  std::vector<int> args_;

  /// The information about the function to be called, corresponding to a given
  /// task ID.
  std::vector<FunctionInfo> task_map_;
};

/// This helper function converts a string in CSV format into a vector of the
/// integer equivalents (using strtoi below) of its component elements.
///
/// @param[in] message The input string in CSV format.
/// @return The vector of integer equivalents of the component elements.
std::vector<int> decodeCSV(const std::string& message);

/// This helper function returns the next chunk of the message to be processed,
/// starting from parsePosition, until the sentinel is encountered. It also
/// updates the position to continue parsing from.
///
/// @param[in] message The message to take the next chunk out of.
/// @param[in,out] parsePosition The starting position of the chunk.
/// @return The substring of message starting at parsePosition, until the
///         sentinel is encountered. Returns the sentinel itself to indicate an
///         error if the sentinel is not encountered.
std::string getNextChunk(const std::string& message, int* parsePosition);

/// This helper function returns the positions where a comma occurs in the given
/// message.
///
/// @param[in] message The message to look for commas.
/// @return A vector of positions where a comma occurs in the message.
std::vector<int> getCommaPositions(const std::string& message);


/// This helper function converts a string to an integer, internally using atoi.
///
/// @param[in] message The string to be converted.
/// @return -1 if the string is empty or doesn't begin with a digit. The integer
///         value that would be returned by atoi otherwise.
int strtoi(const std::string& message);
}  // namespace webgtt

#endif  // EXPERIMENTAL_WEBGTT_PARSER_H_
