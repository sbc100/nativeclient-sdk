// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_PACKETIZER_H_
#define DEBUGGER_RSP_RSP_PACKETIZER_H_

#include <deque>
#include <string>
#include "debugger/base/debug_blob.h"

namespace rsp {
class PacketConsumerInterface;

/// This class provides ability to convert byte stream into RSP messages.
///
/// Byte stream contain messages in RSP wire format, see
/// http://sources.redhat.com/gdb/current/onlinedocs/gdb.html#Remote-Protocol
/// Packetizer removes RSP 'envelope', i.e. start character, stop character,
/// checksum, converts escaped characters, expands run-encoded data.
///
/// Example:
///
/// class MyPacketConsumer : public rsp::PacketConsumerInterface {
/// ...
/// MyPacketConsumer consumer;
/// rsp::Packetizer packetizer;
/// packetizer.SetPacketConsumer(&consumer);
/// ...
/// char buff[100];
/// size_t rd_bytes = 0;
/// while (ReadConnection(buff, &rd_bytes))
///   packetizer.OnData(buff, &rd_bytes);
///
class Packetizer {
 public:
  Packetizer();
  virtual ~Packetizer();

  /// Associate |consumer| with packetizer.
  /// Packetizer will call consumer->OnPacket when it receives complete
  /// RSP packet.
  /// @param[in] consumer externally created and maintained packet consumer.
  /// Note: |consumer| is not owned by Packetizer, it's a weak reference.
  /// |consumer| shall exist for the duration of Packetizer object.
  virtual void SetPacketConsumer(PacketConsumerInterface* consumer);

  /// Consumes incoming RSP messages in GDB RSP wire format.
  /// @param[in] data buffer with incoming bytes
  /// @param[in] data_length size of |data|
  /// Note: |OnData| will directly call one of these methods:
  /// consumer_->OnPacket
  /// consumer_->OnUnexpectedByte
  /// consumer_->OnBreak
  virtual void OnData(const void* data, size_t data_length);

  /// Same as previous member.
  /// @param[in] data buffer with incoming bytes
  virtual void OnData(const debug::Blob& data);

  /// Resets state, dropping all received data.
  virtual void Reset();

 private:
  enum State {
      IDLE,  // in between packets
      BODY,  // packet start symbol is received ('$'), normal RSP packet body
      END,  // packet end symbol is received ('#'), expect checksum next
      CHECKSUM,  // first byte of checksum is received, expect one more
      ESCAPE,  // received an escape symbol
      RUNLEN  // received a run-length encoding symbol, expect count byte next
      };

  virtual void OnByte(uint8_t c);
  virtual void AddByteToChecksum(uint8_t c);
  virtual void AddByteToBody(uint8_t c);

  /// Adds |n| copies of last byte in |body_|.
  virtual void AddRepeatedBytes(size_t n);

  State state_;
  PacketConsumerInterface* consumer_;
  debug::Blob body_;
  unsigned int calculated_checksum_;
  unsigned int recv_checksum_;
};

/// This class represents interface to RSP packet consumer.
class PacketConsumerInterface {
 public:
  PacketConsumerInterface() {}
  virtual ~PacketConsumerInterface() {}

  /// Handler for received RSP packet.
  /// @param[in] body body of RSP packet, with 'envelope' removed
  /// @param[in] valid_checksum true if checksum for the packet is correct
  virtual void OnPacket(const debug::Blob& body, bool valid_checksum) = 0;

  /// This method get called when Packetizer encounter unexpected byte
  virtual void OnUnexpectedByte(uint8_t unexpected_byte) = 0;

  /// This method get called when Packetizer encounter Ctl-C code,
  /// meaning a "break" RSP command.
  virtual void OnBreak() = 0;
};

}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_PACKETIZER_H_

