// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_DEBUG_COMMAND_LINE_H_
#define SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_DEBUG_COMMAND_LINE_H_

#include <string>

namespace debug {

// This class works with command lines.
// Switches can optionally have a value attached as in "--switch value".

class CommandLine {
 public:
  CommandLine(int argc, char* argv[]);  // Initialize from argv vector.
  ~CommandLine();

  // Returns the value associated with the given switch.
  std::string GetStringSwitch(const std::string& name,
                              const std::string& default_value) const;
  int GetIntSwitch(const std::string& name, int default_value) const;
  bool HasSwitch(const std::string& name) const;

 private:
  int argc_;
  char** argv_;
};
}  // namespace debug
#endif  // SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_DEBUG_COMMAND_LINE_H_


