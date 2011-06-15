// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_BASE_DEBUG_COMMAND_LINE_H_
#define DEBUGGER_BASE_DEBUG_COMMAND_LINE_H_

#include <string>
#include <vector>

namespace debug {
/// This class allows to get value of command-line switches.
/// CommandLine constructor takes the same arguments as a C/C++ main function:
/// http://wiki.answers.com/Q/What_is_main_function_on_C_plus_plus
///
/// It also provides conversion to/from string.
/// Switches can optionally have a value attached as in "--switch value".
/// Switch names shall be prefixed by one or two dashes.
class CommandLine {
 public:
  CommandLine() {}

  /// Initialize from |argv| vector. Content of the |argv| is copied.
  /// @param[in] argc number of the program's command-line arguments
  /// @param[in] argv argument vector
  CommandLine(int argc, char* argv[]);

  /// Parses command line string.
  /// @param[in] command_line command line string
  explicit CommandLine(const std::string& command_line);

  ~CommandLine();

  /// @return number of parameters
  size_t GetParametersNum() const;

  /// @param[in] pos parameter position, starting from 0
  /// @return the parameter, whether it's a switch or a value.
  /// Example: CommandLine cl("cmd.exe --a 1 --b 2");
  /// cl.GetParameter(0) -> "--aa"
  /// cl.GetParameter(1) -> "1"
  /// cl.GetParametersNum() -> 4
  std::string GetParameter(size_t pos) const;

  /// @return program name, passed in argv[0]
  std::string GetProgramName() const;

  /// @return command line string
  std::string ToString() const;

  /// @param[in] name name of the switch, with two, one or no dashes.
  /// Example: names "--host", "-host" or "host" should match switches with
  /// actual names "--host" and "-host".
  /// @param[in] default_value value to return if specified switch is not found
  /// @return the value associated with the given switch, or |default_value|.
  std::string GetStringSwitch(const std::string& name,
                              const std::string& default_value) const;

  /// @param[in] name name of the switch, with two, one or no dashes.
  /// Example: names "--port", "-port" or "port" should match switches with
  /// actual names "--port" and "-port".
  /// @param[in] default_value value to return if specified switch is not found
  /// @return the value associated with the given switch, or |default_value|.
  int GetIntSwitch(const std::string& name, int default_value) const;

  /// @param[in] name name of the switch, with two, one or no dashes.
  /// Example: names "--flag", "-flag" or "flag" should match switches with
  /// actual names "--flag" and "-flag".
  /// @return true if switch is found
  bool HasSwitch(const std::string& name) const;

 private:
  std::vector<std::string> argv_;
};
}  // namespace debug
#endif  // DEBUGGER_BASE_DEBUG_COMMAND_LINE_H_


