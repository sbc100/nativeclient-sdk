// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ENV_VAR_H_
#define BASE_ENV_VAR_H_

#include <string>

#include "base/basictypes.h"

namespace base {

// These are used to derive mocks for unittests.
class EnvVarGetter {
 public:
  virtual ~EnvVarGetter() {}
  // Gets an environment variable's value and stores it in |result|.
  // Returns false if the key is unset.
  virtual bool GetEnv(const char* variable_name, std::string* result) = 0;

  // Syntactic sugar for GetEnv(variable_name, NULL);
  virtual bool HasEnv(const char* variable_name) {
    return GetEnv(variable_name, NULL);
  }

  // Create an instance of EnvVarGetter
  static EnvVarGetter* Create();
};

}  // namespace base

#endif  // BASE_ENV_VAR_H_
