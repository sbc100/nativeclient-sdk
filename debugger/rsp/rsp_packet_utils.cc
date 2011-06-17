// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "debugger/rsp/rsp_packet_utils.h"
#include "debugger/rsp/rsp_packetizer.h"

namespace {
/// Replaces characters '}', '#', '$', '*' and characters > 126 with escape
/// sequences: '}' + c ^ 0x20, per RSP protocol spec.
/// @param[in] blob_in blob to be escaped
/// @param[out] blob_out destination for transformed blob
void Escape(const debug::Blob& blob_in, debug::Blob* blob_out) {
  blob_out->Clear();
  unsigned char prev_c = 0;
  for (size_t i = 0; i < blob_in.size(); i++) {
    unsigned char c = blob_in[i];
    if (((('$' == c) || ('#' == c) || ('*' == c) || ('}' == c) || (3 == c)) &&
        (prev_c != '*')) || (c > 126)) {
      // escape it by '}'
      blob_out->PushBack('}');
      c = c ^ 0x20;
    }
    blob_out->PushBack(c);
    prev_c = blob_in[i];
  }
}

class LocalPacketConsumer : public rsp::PacketConsumerInterface {
 public:
  explicit LocalPacketConsumer(debug::Blob* packet)
      : packet_(packet),
        success_(false) {
  }
  virtual void OnPacket(const debug::Blob& body, bool valid_checksum) {
    *packet_ = body;
    success_ = valid_checksum;
  }
  virtual void OnUnexpectedByte(uint8_t unexpected_byte) {}
  virtual void OnBreak() {}

  debug::Blob* packet_;
  bool success_;
};
}  // namespace

namespace rsp {
void PacketUtils::AddEnvelope(const debug::Blob& blob_in,
                              debug::Blob* blob_out) {
  blob_out->Clear();
  Escape(blob_in, blob_out);
  unsigned int checksum = 0;
  for (size_t i = 0; i < blob_out->size(); i++) {
    checksum += (*blob_out)[i];
    checksum %= 256;
  }

  blob_out->PushFront('$');
  blob_out->PushBack('#');
  blob_out->PushBack(debug::Blob::GetHexDigit(checksum, 1));
  blob_out->PushBack(debug::Blob::GetHexDigit(checksum, 0));
}

bool PacketUtils::RemoveEnvelope(const debug::Blob& blob_in,
                                 debug::Blob* blob_out) {
  blob_out->Clear();

  // RSP packet consumer, copies received RSP message to |blob_out|.
  LocalPacketConsumer consumer(blob_out);

  // Associate |consumer| with packetizer.
  // Packetizer will call consumer->OnPacket when it receives complete
  // RSP packet.
  Packetizer pktz;
  pktz.SetPacketConsumer(&consumer);

  // Packetizer removes envelope and calls LocalPacketConsumer::OnPacket,
  // OnPacket copies unpacked message to |blob_out|.
  pktz.OnData(blob_in);

  // LocalPacketConsumer::OnPacket sets |consumer.success_| to true
  // if message was successfully unpacked (and checksum is correct).
  return consumer.success_;
}
}  // namespace rsp

