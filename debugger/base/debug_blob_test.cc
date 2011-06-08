// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/base/debug_blob.h"
#include "gtest/gtest.h"

namespace {
char* kBlobStr = "1234567890abcdef";

// debug::Blob test fixture.
class BlobTest : public ::testing::Test {
 public:
  BlobTest() { blob_.FromHexString(kBlobStr); }

  debug::Blob blob_;
};

// Unit tests start here.
TEST_F(BlobTest, Empty) {
  debug::Blob blob;
  EXPECT_EQ(0, blob.size());
  std::string str = blob.ToHexStringNoLeadingZeroes();
  EXPECT_STREQ("", str.c_str());
}

TEST_F(BlobTest, ToHexString) {
  unsigned char buff[] = {0x3, 0xAF, 0xF0};
  debug::Blob blob(buff, sizeof(buff));
  EXPECT_EQ(sizeof(buff), blob.size());
  EXPECT_STREQ("3aff0", blob.ToHexStringNoLeadingZeroes().c_str());
  EXPECT_STREQ("03aff0", blob.ToHexString().c_str());
}

TEST_F(BlobTest, FromHexString) {
  // Blob::FromHexString is called in BlobTest constructor.
  EXPECT_STREQ(kBlobStr, blob_.ToHexString().c_str());
}

TEST_F(BlobTest, CopyConstructor) {
  debug::Blob blob(blob_);
  EXPECT_EQ(true, (blob == blob_));
  EXPECT_EQ(blob_.size(), blob.size());
  EXPECT_STREQ(kBlobStr, blob.ToHexString().c_str());
}

TEST_F(BlobTest, ConstructorFromString) {
  debug::Blob blob;
  blob.FromString("1234");
  EXPECT_STREQ("31323334", blob.ToHexString().c_str());
}

TEST_F(BlobTest, AssignmentOperator) {
  debug::Blob blob;
  blob = blob_;
  EXPECT_EQ(true, (blob == blob_));
  EXPECT_EQ(blob_.size(), blob.size());
  EXPECT_STREQ(kBlobStr, blob.ToHexString().c_str());
}

TEST_F(BlobTest, Accessors) {
  EXPECT_EQ(8, blob_.size());
  EXPECT_EQ(0x12, blob_[0]);
  EXPECT_EQ(0x12, blob_.GetAt(0));
  EXPECT_EQ(0x12, blob_.Front());
  EXPECT_EQ(0xab, blob_[5]);
  EXPECT_EQ(0xab, blob_.GetAt(5));
  EXPECT_EQ(0xef, blob_.Back());
}

TEST_F(BlobTest, PopFront) {
  EXPECT_EQ(0x12, blob_.PopFront());
  EXPECT_EQ(0x34, blob_[0]);
  EXPECT_EQ(7, blob_.size());

  EXPECT_EQ(0x34, blob_.PopFront());
  EXPECT_EQ(0x56, blob_[0]);
  EXPECT_EQ(6, blob_.size());
}

TEST_F(BlobTest, PopBack) {
  EXPECT_EQ(0xef, blob_.PopBack());
  EXPECT_EQ(0xcd, blob_[6]);
  EXPECT_EQ(7, blob_.size());

  EXPECT_EQ(0xcd, blob_.PopBack());
  EXPECT_EQ(0xab, blob_[5]);
  EXPECT_EQ(6, blob_.size());
}

TEST_F(BlobTest, PushFront) {
  blob_.PushFront(0x88);
  EXPECT_EQ(9, blob_.size());
  EXPECT_EQ(0x88, blob_[0]);
  EXPECT_EQ(0x12, blob_[1]);
  EXPECT_EQ(0xab, blob_[6]);

  blob_.PushFront(0xaa);
  EXPECT_EQ(10, blob_.size());
  EXPECT_EQ(0xaa, blob_[0]);
  EXPECT_EQ(0x88, blob_[1]);
  EXPECT_EQ(0x12, blob_[2]);
  EXPECT_EQ(0xab, blob_[7]);
}

TEST_F(BlobTest, PushBack) {
  blob_.PushBack(0x88);
  EXPECT_EQ(9, blob_.size());
  EXPECT_EQ(0x12, blob_[0]);
  EXPECT_EQ(0xab, blob_[5]);
  EXPECT_EQ(0x88, blob_[8]);

  blob_.PushBack(0xaa);
  EXPECT_EQ(10, blob_.size());
  EXPECT_EQ(0x12, blob_[0]);
  EXPECT_EQ(0xab, blob_[5]);
  EXPECT_EQ(0x88, blob_[8]);
  EXPECT_EQ(0xaa, blob_[9]);
}

TEST_F(BlobTest, AppendBlob) {
  debug::Blob blob1(debug::Blob().FromString("1234"));
  debug::Blob blob2(debug::Blob().FromString("06"));
  blob1.Append(blob2);
  EXPECT_STREQ("313233343036", blob1.ToHexString().c_str());
}

TEST_F(BlobTest, Clear) {
  EXPECT_EQ(8, blob_.size());
  blob_.Clear();
  EXPECT_EQ(0, blob_.size());
}

TEST_F(BlobTest, Peek) {
  unsigned char buff[3];
  memset(buff, 0xcc, sizeof(buff));
  EXPECT_EQ(3, blob_.Peek(0, buff, sizeof(buff)));
  EXPECT_EQ(0x12, buff[0]);
  EXPECT_EQ(0x34, buff[1]);
  EXPECT_EQ(0x56, buff[2]);

  memset(buff, 0xcc, sizeof(buff));
  EXPECT_EQ(2, blob_.Peek(6, buff, sizeof(buff)));
  EXPECT_EQ(0xcd, buff[0]);
  EXPECT_EQ(0xef, buff[1]);
  EXPECT_EQ(0xcc, buff[2]);
}

TEST_F(BlobTest, ToString) {
  debug::Blob blob(debug::Blob().FromString(kBlobStr));
  EXPECT_STREQ(kBlobStr, blob.ToString().c_str());
}

TEST_F(BlobTest, Reverse) {
  blob_.Reverse();
  EXPECT_EQ(0x12, blob_[7]);
  EXPECT_EQ(0x34, blob_[6]);
  EXPECT_EQ(0x56, blob_[5]);
  EXPECT_EQ(0x78, blob_[4]);
  EXPECT_EQ(0x90, blob_[3]);
  EXPECT_EQ(0xab, blob_[2]);
  EXPECT_EQ(0xcd, blob_[1]);
  EXPECT_EQ(0xef, blob_[0]);
}

TEST_F(BlobTest, Split) {
  blob_.FromString("some:stuff,goes:here");
  std::deque<debug::Blob> tokens;
  blob_.Split(debug::Blob().FromString(",:"), &tokens);
  ASSERT_EQ(4, tokens.size());
  EXPECT_STREQ("some", tokens[0].ToString().c_str());
  EXPECT_STREQ("stuff", tokens[1].ToString().c_str());
  EXPECT_STREQ("goes", tokens[2].ToString().c_str());
  EXPECT_STREQ("here", tokens[3].ToString().c_str());
}

TEST_F(BlobTest, HexCharToInt) {
  unsigned int result = 0;
  EXPECT_TRUE(debug::Blob::HexCharToInt('c', &result));
  EXPECT_EQ(12, result);
  EXPECT_FALSE(debug::Blob::HexCharToInt('g', &result));
  EXPECT_TRUE(debug::Blob::HexCharToInt('1', &result));
  EXPECT_EQ(1, result);
}

TEST_F(BlobTest, PopBlobFromFrontUntilBytes) {
  blob_.FromString("some:stuff,goes:here");
  debug::Blob token =
    blob_.PopBlobFromFrontUntilBytes(debug::Blob().FromString(",:"));
  EXPECT_STREQ("some", token.ToString().c_str());
}

TEST_F(BlobTest, PopMatchingBytesFromFront) {
  blob_.FromString("   ,some:stuff,goes:here");
  blob_.PopMatchingBytesFromFront(debug::Blob().FromString(", "));
  EXPECT_STREQ("some:stuff,goes:here", blob_.ToString().c_str());
}

}  // namespace

