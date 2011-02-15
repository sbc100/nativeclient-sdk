// Copyright (c) 2010 The Ginsu Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "c_salt/integration_tests/test_module.h"

#include <cstdio>

// Return the TestModule.
namespace c_salt {
Module* CreateModule() {
  std::printf("Creating the Module singleton.\n");
  return new TestModule();
}
}  // namespace c_salt
