// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/base/debug_command_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#pragma warning(disable : 4996)  // Disable sscanf warning.
#endif

namespace {
/// Prepends '-' if |name| has one '-' at the front.
std::string NormalizeSwitchName(const std::string& name) {
  std::string result = name;
  if (name.size() > 1)
    if ((name[0] == '-') && (name[1] != '-'))
      result = std::string("-") + result;
  return result;
}

/// Prepends '-' one at a time until the name has two -- at the front.
std::string CreateSwitchName(const std::string& name) {
  std::string result = name;
  while (0 != strncmp(result.c_str(), "--", 2)) {
    result = std::string("-") + result;
  }
  return result;
}

bool HasWhiteSpace(const std::string& str) {
  for (size_t i = 0; i < str.size(); i++) {
    if (isspace(str[i]))
      return true;
  }
  return false;
}
}  // namespace

namespace debug {
CommandLine::CommandLine(int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    std::string value;
    if (NULL != argv[i])
      value = argv[i];
    argv_.push_back(value);
  }
}

CommandLine::CommandLine(const std::string& command_line) {
  std::string token;
  bool in_quote = false;
  for (size_t i = 0; i < command_line.size(); i++) {
    char c = command_line[i];
    if (in_quote) {
      if ('"' == c) {
        in_quote = false;
        argv_.push_back(token);
        token.clear();
      } else {
        token.push_back(c);
      }
    } else if (isspace(c)) {
      if (token.size() > 0)
        argv_.push_back(token);
      token.clear();
    } else if (('"' == c) && (token.size() == 0)) {
      in_quote = true;
    } else {
      token.push_back(c);
    }
  }
  if (token.size() > 0)
    argv_.push_back(token);
}

CommandLine::~CommandLine() {}

size_t CommandLine::GetParametersNum() const {
  return argv_.size() > 0 ? argv_.size() - 1 : 0;
}

std::string CommandLine::GetParameter(size_t pos) const {
  if (argv_.size() > (pos + 1))
    return argv_[pos + 1];
  return "";
}

std::string CommandLine::GetProgramName() const {
  if (argv_.size() > 0)
    return argv_[0];
  return "";
}

std::string CommandLine::ToString() const {
  std::string str;
  for (size_t i = 0; i < argv_.size(); i++) {
    if (i > 0)
      str += " ";
    if (HasWhiteSpace(argv_[i])) {
      str += "\"";
      str += argv_[i];
      str += "\"";
    } else {
      str += argv_[i];
    }
  }
  return str;
}

std::string CommandLine::GetStringSwitch(
    const std::string& name,
    const std::string& default_value) const {
  for (size_t i = 1; (i + 1) < argv_.size(); i++)
    if (NormalizeSwitchName(argv_[i]) == CreateSwitchName(name))
      return argv_[i + 1];
  return default_value;
}

int CommandLine::GetIntSwitch(const std::string& name,
                              int default_value) const {
  std::string str = GetStringSwitch(name, "");
  if (0 != str.size())
    return atoi(str.c_str());
  return default_value;
}

void* CommandLine::GetAddrSwitch(const std::string& name) const {
  void* ptr = NULL;
  std::string str = GetStringSwitch(name, "");
  sscanf(str.c_str(), "%p", &ptr);  // NOLINT
  return ptr;
}

bool CommandLine::HasSwitch(const std::string& name) const {
  for (size_t i = 1; i < argv_.size(); i++)
    if (NormalizeSwitchName(argv_[i]) == CreateSwitchName(name))
      return true;
  return false;
}
}  // namespace debug


