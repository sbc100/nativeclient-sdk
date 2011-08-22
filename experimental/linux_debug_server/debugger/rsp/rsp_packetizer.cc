// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_packetizer.h"

// Description of GDB RSP protocol:
// http://sources.redhat.com/gdb/current/onlinedocs/gdb.html#Remote-Protocol

namespace {
const int kMaxCharValue = 126;
const int kEscapeXor = 0x20;
}  // namespace

namespace rsp {

Packetizer::Packetizer()
  : state_(IDLE),
    consumer_(NULL),
    calculated_checksum_(0),
    recv_checksum_(0) {
  Reset();
}

Packetizer::~Packetizer() {
}

void Packetizer::SetPacketConsumer(PacketConsumerInterface* consumer) {
  consumer_ = consumer;
}

void Packetizer::Reset() {
  state_ = IDLE;
  body_.Clear();
  calculated_checksum_ = 0;
  recv_checksum_ = 0;
}

void Packetizer::OnData(const debug::Blob& data) {
  for (size_t i = 0; i < data.size(); i++)
    OnByte(data[i]);
}

void Packetizer::OnData(const void* data, size_t data_length) {
  const uint8_t* cdata = static_cast<const uint8_t*>(data);
  if (NULL == data)
    return;
  for (size_t i = 0; i < data_length; i++) {
    uint8_t c = *cdata++;
    OnByte(c);
  }
}

void Packetizer::OnByte(uint8_t c) {
  if (NULL == consumer_)
    return;

  if (((BODY == state_) && ('#' != c)) ||
      (ESCAPE == state_) || (RUNLEN == state_))
    AddByteToChecksum(c);

  switch (state_) {
    case IDLE: {
      if (('+' == c) || ('-' == c))
        return;
      if ('$' == c)
        state_ = BODY;
      else if (3 == c)  // RSP is using Ctrl-C as a break message.
        consumer_->OnBreak();
      else
        consumer_->OnUnexpectedByte(c);
      break;
    }
    case BODY: {
      if ('}' == c)
        state_ = ESCAPE;
      else if ('#' == c)
        state_ = END;
      else if ('*' == c)
        state_ = RUNLEN;
      else if (c > kMaxCharValue)
        consumer_->OnUnexpectedByte(c);
      else
        AddByteToBody(c);
      break;
    }
    case ESCAPE: {
      c = c ^ kEscapeXor;
      AddByteToBody(c);
      state_ = BODY;
      break;
    }
    case RUNLEN: {
      int n = c - 29;
      AddRepeatedBytes(n);
      state_ = BODY;
      break;
    }
    case END: {
      if (debug::Blob::HexCharToInt(c, &recv_checksum_)) {
        recv_checksum_ <<= 4;
        state_ = CHECKSUM;
      } else {
        consumer_->OnUnexpectedByte(c);
      }
      break;
    }
    case CHECKSUM: {
      unsigned int digit = 0;
      if (debug::Blob::HexCharToInt(c, &digit)) {
        recv_checksum_ += digit;
        bool checksum_valid = (recv_checksum_ == calculated_checksum_);
        consumer_->OnPacket(body_, checksum_valid);
        Reset();
      } else {
        consumer_->OnUnexpectedByte(c);
      }
      break;
    }
  }
}

void Packetizer::AddByteToChecksum(uint8_t c) {
  calculated_checksum_ += c;
  calculated_checksum_ %= 256;
}

void Packetizer::AddByteToBody(uint8_t c) {
  body_.PushBack(c);
}

void Packetizer::AddRepeatedBytes(size_t n) {
  size_t len = body_.size();
  if (0 != len) {
    uint8_t c = body_[len - 1];
    for (size_t i = 0; i < n; i++)
      AddByteToBody(c);
  }
}
}  // namespace rsp

