// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwarf_reader/dwarf_reader_UnitTestProject/base_fixture.h"

// In this file, we test the basic setup functions provided in our
// BaseFixtures base class.
namespace dwarf_reader_tests {

// Unit tests start here.
TEST_F(BaseFixture, IsVSXRootSet) {
  ASSERT_TRUE(InitializeNaClVSXRoot());
}

TEST_F(BaseFixture, DoesLoopNexeLoad) {
  ASSERT_TRUE(InitializeElfObject());
}

}  // namespace dwarf_reader_tests

