// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_blob_utils.h"
#include "gtest/gtest.h"

namespace {
// BlobUtilsTest test fixture.
class BlobUtilsTest : public ::testing::Test {
 public:
  BlobUtilsTest() {}
  void LoadBlob(const char* str) { blob_.FromString(str); }

  debug::Blob blob_;
};

// Unit tests start here.
TEST_F(BlobUtilsTest, PopInt8FromFront) {
  LoadBlob("10abcd");
  EXPECT_EQ(0x10, rsp::PopInt8FromFront(&blob_));
  EXPECT_STREQ("abcd", blob_.ToString().c_str());

  LoadBlob("f");
  EXPECT_EQ(0xf, rsp::PopInt8FromFront(&blob_));
  EXPECT_STREQ("", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, PopInt32FromFront) {
  LoadBlob("a");
  EXPECT_EQ(0xa, rsp::PopInt32FromFront(&blob_));
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("10abcd");
  EXPECT_EQ(0x10abcd, rsp::PopInt32FromFront(&blob_));
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("1234567890abcdef");
  EXPECT_STREQ("1234567890abcdef", blob_.ToString().c_str());
  EXPECT_EQ(0x12345678, rsp::PopInt32FromFront(&blob_));
  EXPECT_STREQ("90abcdef", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, PopInt64FromFront) {
  LoadBlob("a");
  EXPECT_EQ(0xa, rsp::PopInt64FromFront(&blob_));
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("10abcd");
  EXPECT_EQ(0x10abcd, rsp::PopInt64FromFront(&blob_));
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("1234567890abcdef000");
  EXPECT_EQ(0x1234567890abcdef, rsp::PopInt64FromFront(&blob_));
  EXPECT_STREQ("000", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, GarbageInZeroesOut) {
  LoadBlob("klmnopqwerty");
  EXPECT_EQ(0x0, rsp::PopInt32FromFront(&blob_));
  EXPECT_STREQ("erty", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, RemoveSpacesFromBothEnds) {
  LoadBlob("  \tabc-12 \n\r");
  rsp::RemoveSpacesFromBothEnds(&blob_);
  EXPECT_STREQ("abc-12", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, RemoveSpacesFromBothEnds2) {
  std::deque<debug::Blob> blobs;
  blobs.push_back(debug::Blob().FromString("  \tabc-12 \n\r"));
  blobs.push_back(debug::Blob().FromString("12345 "));
  blobs.push_back(debug::Blob().FromString("  ubiq13 "));
  blobs.push_back(debug::Blob().FromString("drkl;zctmo"));

  rsp::RemoveSpacesFromBothEnds(&blobs);
  EXPECT_STREQ("abc-12", blobs[0].ToString().c_str());
  EXPECT_STREQ("12345", blobs[1].ToString().c_str());
  EXPECT_STREQ("ubiq13", blobs[2].ToString().c_str());
  EXPECT_STREQ("drkl;zctmo", blobs[3].ToString().c_str());
}

TEST_F(BlobUtilsTest, Format) {
  rsp::Format(&blob_, "[%s]", "abcd");
  EXPECT_STREQ("[abcd]", blob_.ToString().c_str());

  rsp::Format(&blob_, "[%d]", 12345);
  EXPECT_STREQ("[12345]", blob_.ToString().c_str());

  rsp::Format(&blob_, "[%d-%x]", 12345, 0x1234);
  EXPECT_STREQ("[12345-1234]", blob_.ToString().c_str());
}

}  // namespace

