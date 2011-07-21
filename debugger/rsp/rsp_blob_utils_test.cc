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

#define EXPECT_BLOBEQ(x, y) EXPECT_STREQ(x, y.ToString().c_str()); y.Clear()

// Unit tests start here.
TEST_F(BlobUtilsTest, PopInt8FromFront) {
  LoadBlob("10abcd");
  uint8_t v = 0;
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0x10, v);
  EXPECT_STREQ("abcd", blob_.ToString().c_str());

  LoadBlob("f");
  v = 0;
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0xf, v);
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("kaka");
  v = 0;
  EXPECT_FALSE(rsp::PopIntFromFront(&blob_, &v));

  LoadBlob("f=xxx");
  v = 0;
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0xf, v);
  EXPECT_STREQ("=xxx", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, PopInt32FromFront) {
  LoadBlob("a");
  uint32_t v = 0;
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0xa, v);
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("10abcd");
  v = 0;
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0x10abcd, v);
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("1234567890abcdef");
  EXPECT_STREQ("1234567890abcdef", blob_.ToString().c_str());
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0x12345678, v);
  EXPECT_STREQ("90abcdef", blob_.ToString().c_str());

  LoadBlob("bcd=0");
  v = 0;
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0xbcd, v);
  EXPECT_STREQ("=0", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, PopInt64FromFront) {
  LoadBlob("a");
  uint64_t v = 0;
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0xa, v);
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("10abcd");
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0x10abcd, v);
  EXPECT_STREQ("", blob_.ToString().c_str());

  LoadBlob("1234567890abcdef000");
  EXPECT_TRUE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(0x1234567890abcdef, v);
  EXPECT_STREQ("000", blob_.ToString().c_str());
}

TEST_F(BlobUtilsTest, GarbageInZeroesOut) {
  LoadBlob("klmnopqwerty");
  uint32_t v = 666;
  EXPECT_FALSE(rsp::PopIntFromFront(&blob_, &v));
  EXPECT_EQ(666, v);
  EXPECT_STREQ("klmnopqwerty", blob_.ToString().c_str());
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
  EXPECT_BLOBEQ("[12345-1234]", blob_);
}

TEST_F(BlobUtilsTest, PushIntToBack) {
  uint32_t x = 0;
  debug::Blob blob;
  EXPECT_BLOBEQ("0", rsp::PushIntToBack(x, &blob));

  x = 0x21;
  EXPECT_BLOBEQ("21", rsp::PushIntToBack(x, &blob));

  x = 0x1234;
  EXPECT_BLOBEQ("1234", rsp::PushIntToBack(x, &blob));

  x = 0x0234;
  EXPECT_BLOBEQ("234", rsp::PushIntToBack(x, &blob));

  uint64_t y = 0;
  EXPECT_BLOBEQ("0", rsp::PushIntToBack(y, &blob));

  y = 0x21;
  EXPECT_BLOBEQ("21", rsp::PushIntToBack(y, &blob));

  y = 0x1234;
  EXPECT_BLOBEQ("1234", rsp::PushIntToBack(y, &blob));

  y = 0x0234;
  EXPECT_BLOBEQ("234", rsp::PushIntToBack(y, &blob));

  y = 0x1234567890abcdef;
  EXPECT_BLOBEQ("1234567890abcdef", rsp::PushIntToBack(y, &blob));

  y = 0x234567890abcdef;
  EXPECT_BLOBEQ("234567890abcdef", rsp::PushIntToBack(y, &blob));

  y = 0x34567890abcdef;
  EXPECT_BLOBEQ("34567890abcdef", rsp::PushIntToBack(y, &blob));
}

}  // namespace

