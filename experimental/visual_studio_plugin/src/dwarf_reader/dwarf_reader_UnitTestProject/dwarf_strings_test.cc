// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwarf_reader/dwarf_strings.h"
#include "gtest/gtest.h"

namespace {
// Unit tests start here.
// TODO(mlinck) replace this trival sample test with tests for new code as it
// is being written.
TEST(DwarfStringsTest, DwarfAttributeName) {
  const char * name = dwarf_reader::DwarfAttributeName(
      dwarf2reader::DW_AT_location);
  EXPECT_STREQ("location", name);
}
}  // namespace

