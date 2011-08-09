// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  int res = 0;
  ::testing::InitGoogleTest(&argc, argv);
  res = RUN_ALL_TESTS();

  return res;
}
