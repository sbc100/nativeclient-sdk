// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_RSP_PACKETIZER_H_
#define SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_RSP_PACKETIZER_H_

#include <deque>
#include <string>
#include "debugger/base/debug_blob.h"
#include "debugger/rsp/rsp_packet_util.h"

namespace rsp {
class PacketConsumer {
 public:
  PacketConsumer() {}
  virtual ~PacketConsumer() {}
  virtual void OnPacket(const debug::Blob& body, bool valid_checksum) = 0;
  virtual void OnUnexpectedChar(char unexpected_char) = 0;
  virtual void OnBreak() = 0;
};

class Packetizer {
 public:
  Packetizer();
  virtual ~Packetizer();

  virtual void SetPacketConsumer(PacketConsumer* consumer);
  virtual void OnData(const void* data, size_t data_length);
  virtual void OnData(const debug::Blob& data);
  virtual void OnData(const char* data_str);  // From zero-terminated string.
  virtual void Reset();
  virtual bool IsIdle() const;

 private:
  enum State {IDLE, BODY, END, CHECKSUM, ESCAPE, RUNLEN};

  virtual void OnChar(unsigned char c);
  bool HexCharToint(unsigned char c, unsigned int* result);
  virtual void AddToChecksum(unsigned char c);
  virtual void AddCharToBody(unsigned char c);
  virtual void AddRepeatedChars(size_t n);

  State state_;
  PacketConsumer* consumer_;
  debug::Blob body_;
  unsigned int calculated_checksum_;
  unsigned int recv_checksum_;
};

}  // namespace rsp

#endif  // SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_RSP_PACKETIZER_H_

