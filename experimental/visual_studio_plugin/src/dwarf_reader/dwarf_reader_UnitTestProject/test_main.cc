// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "gmock/gmock.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  int res = 0;
  ::testing::InitGoogleTest(&argc, argv);
  // The following line must be executed to initialize Google Mock
  // (and Google Test) before running the tests.
  ::testing::InitGoogleMock(&argc, argv);
  res = RUN_ALL_TESTS();

  return res;
}
