// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/test_utils/sys_utils.h"

#pragma warning(disable : 4996)  // Disable getenv warning.

namespace sys_utils {
std::string GetStringEnvVar(const std::string& name,
                            const std::string& default_value) {
  char* value = getenv(name.c_str());
  if (NULL != value)
    return value;
  return default_value;
}

int GetIntEnvVar(const std::string& name,
                 int default_value) {
  char* value = getenv(name.c_str());
  if (NULL != value)
    return atoi(value);
  return default_value;
}

std::string DirFromPath(const std::string& command_line) {
  int last_dash_pos = 0;
  for (size_t i = 0; i < command_line.size(); i++)
    if (command_line[i] == '\\')
      last_dash_pos = i;

  std::string dir = command_line.substr(0, last_dash_pos);
  return dir;
}
}  // namespace sys_utils

