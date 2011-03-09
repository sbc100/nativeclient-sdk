// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/env_var.h"

#if defined(OS_POSIX)
#include <stdlib.h>
#elif defined(OS_WIN)
#include <windows.h>
#endif

#include "base/string_util.h"

#if defined(OS_WIN)
#include "base/scoped_ptr.h"
#include "base/utf_string_conversions.h"
#endif

namespace {

class EnvVarGetterImpl
    : public base::EnvVarGetter {
 public:
  virtual bool GetEnv(const char* variable_name, std::string* result) {
    if (GetEnvImpl(variable_name, result))
      return true;

    // Some commonly used variable names are uppercase while others
    // are lowercase, which is inconsistent. Let's try to be helpful
    // and look for a variable name with the reverse case.
    // I.e. HTTP_PROXY may be http_proxy for some users/systems.
    char first_char = variable_name[0];
    std::string alternate_case_var;
    if (first_char >= 'a' && first_char <= 'z')
      alternate_case_var = StringToUpperASCII(std::string(variable_name));
    else if (first_char >= 'A' && first_char <= 'Z')
      alternate_case_var = StringToLowerASCII(std::string(variable_name));
    else
      return false;
    return GetEnvImpl(alternate_case_var.c_str(), result);
  }
 private:
  bool GetEnvImpl(const char* variable_name, std::string* result) {
#if defined(OS_POSIX)
    const char* env_value = getenv(variable_name);
    if (!env_value)
      return false;
    // Note that the variable may be defined but empty.
    if (result)
      *result = env_value;
    return true;
#elif defined(OS_WIN)
    DWORD value_length = ::GetEnvironmentVariable(
        UTF8ToWide(variable_name).c_str(), NULL, 0);
    if (value_length == 0)
      return false;
    if (result) {
      scoped_array<wchar_t> value(new wchar_t[value_length]);
      ::GetEnvironmentVariable(UTF8ToWide(variable_name).c_str(), value.get(),
                               value_length);
      *result = WideToUTF8(value.get());
    }
    return true;
#else
#error need to port
#endif
  }
};

}  // namespace

namespace base {

// static
EnvVarGetter* EnvVarGetter::Create() {
  return new EnvVarGetterImpl();
}

}  // namespace base
