// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_packet_utils.h"
#include "gtest/gtest.h"

namespace {
// PacketUtilsTest test fixture.
class PacketUtilsTest : public ::testing::Test {
 public:
  debug::Blob blob_in_;
  debug::Blob blob_out_;
};

// Unit tests start here.
TEST_F(PacketUtilsTest, AddEnvelope) {
  blob_in_.FromString("}aa");
  rsp::PacketUtils::AddEnvelope(blob_in_, &blob_out_);
  EXPECT_STREQ("$}]aa#9c", blob_out_.ToString().c_str());

  blob_in_.FromString("0000000");
  rsp::PacketUtils::AddEnvelope(blob_in_, &blob_out_);
  EXPECT_STREQ("$0000000#50", blob_out_.ToString().c_str());
}

TEST_F(PacketUtilsTest, RemoveEnvelope) {
  blob_in_.FromString("$}]aa#9c");
  rsp::PacketUtils::RemoveEnvelope(blob_in_, &blob_out_);
  EXPECT_STREQ("}aa", blob_out_.ToString().c_str());

  // 0*# means the character (#) after the * is the number of times (-29) that
  // the value before the * is repeated. Since # is ascii 35, 35-29=6, '0' is
  // repeated six times plus we have the original 0.
  blob_in_.FromString("$0*##7d");
  rsp::PacketUtils::RemoveEnvelope(blob_in_, &blob_out_);
  EXPECT_STREQ("0000000", blob_out_.ToString().c_str());
}
}  // namespace

