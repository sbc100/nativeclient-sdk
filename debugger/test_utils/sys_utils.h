// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_TEST_UTILS_SYS_UTILS_H_
#define DEBUGGER_TEST_UTILS_SYS_UTILS_H_

#include <string>

namespace sys_utils {
/// @param name[in] name of the environment variable
/// @param default_value[in] value to return if environment variable is not set
/// @return value of environment variable, or |default_value| if
/// there's no |name| environment variable.
std::string GetStringEnvVar(const std::string& name,
                            const std::string& default_value);

/// @param name[in] name of the environment variable
/// @param default_value[in] value to return if environment variable is not set
/// @return value of environment variable, or |default_value| if
/// there's no |name| environment variable.
int GetIntEnvVar(const std::string& name, int default_value);

/// @return directory part of the |path|
std::string DirFromPath(const std::string& path);
}  // namespace sys_utils

#endif  // DEBUGGER_TEST_UTILS_SYS_UTILS_H_

