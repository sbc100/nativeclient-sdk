// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_packetizer.h"
#include "gtest/gtest.h"

namespace {
#define EXPECT_RES_TYPE(type) \
EXPECT_EQ(TokenizerTestConsumer::type, consumer_.result_type_)

#define EXPECT_RESULT(type, str, checksum_valid) \
EXPECT_RES_TYPE(type);\
if (TokenizerTestConsumer::PACKET == TokenizerTestConsumer::type)\
  EXPECT_TRUE(debug::Blob().FromString(str) == consumer_.packet_body_);\
EXPECT_EQ(checksum_valid, consumer_.packet_checksum_is_valid_)

class TokenizerTestConsumer : public rsp::PacketConsumerInterface {
 public:
  enum ResultType {NONE, PACKET, INVALID_BYTE, BREAK};

  TokenizerTestConsumer()
      : result_type_(NONE),
        packet_checksum_is_valid_(true) {
  }

  void OnPacket(const debug::Blob& body, bool valid_checksum) {
    result_type_ = PACKET;
    packet_body_ = body;
    packet_checksum_is_valid_ = valid_checksum;
  }

  void OnUnexpectedByte(uint8_t unexpected_byte) {
    result_type_ = INVALID_BYTE;
  }

  void OnBreak() {
    result_type_ = BREAK;
  }

  ResultType result_type_;
  debug::Blob packet_body_;
  bool packet_checksum_is_valid_;
};

// debug::Packetizer test fixture.
class PacketizerTest : public ::testing::Test {
 public:
  PacketizerTest() { packetizer_.SetPacketConsumer(&consumer_); }

  void OnData(const char* str) {
    packetizer_.OnData(debug::Blob().FromString(str));
  }

  rsp::Packetizer packetizer_;
  TokenizerTestConsumer consumer_;
};

// Unit tests start here.
TEST_F(PacketizerTest, Break) {
  OnData("\x03");
  EXPECT_RES_TYPE(BREAK);
}

TEST_F(PacketizerTest, EmptyPacket) {
  OnData("$#00");
  EXPECT_RESULT(PACKET, "", true);
}

TEST_F(PacketizerTest, EmptyBadPacket) {
  OnData("$#02");
  EXPECT_RESULT(PACKET, "", false);
}

TEST_F(PacketizerTest, SimplePacket) {
  OnData("$  #40");
  EXPECT_RESULT(PACKET, "  ", true);
}

TEST_F(PacketizerTest, SimplePacket2) {
  OnData("$123456#35");
  EXPECT_RESULT(PACKET, "123456", true);
}

TEST_F(PacketizerTest, SimplePacketWithEscapedChar) {
  OnData("$}]aa#9C");
  EXPECT_RESULT(PACKET, "}aa", true);
}

TEST_F(PacketizerTest, SimplePacketWithRLE) {
  // 0*# means the character (#) after the * is the number of times (-29) that
  // the value before the * is repeated. Since # is ascii 35, 35-29=6, '0' is
  // repeated six times plus we have the original 0.
  OnData("$0*##7D");
  EXPECT_RESULT(PACKET, "0000000", true);
}

TEST_F(PacketizerTest, InvalidByte) {
  OnData("#");
  EXPECT_RESULT(INVALID_BYTE, "#", true);
}
}  // namespace

