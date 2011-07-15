// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/base/debug_command_line.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  debug::CommandLine command_line(argc, argv);

  // |repeat_number| specifies how many times unit tests run without breaking.
  // Used to detect flaky tests. Default is one.
  int repeat_number = command_line.GetIntSwitch("-repeat", 1);

  int res = 0;
  for (int i = 0; i < repeat_number; i++) {
    ::testing::InitGoogleTest(&argc, argv);
    res = RUN_ALL_TESTS();
    if (res != 0)
      break;
  }
  return res;
}
