// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "debugger/rsp/rsp_packet_util.h"
#include "debugger/rsp/rsp_packetizer.h"

// Description of GDB RSP protocol:
// http://sources.redhat.com/gdb/current/onlinedocs/gdb.html#Remote-Protocol

namespace {
class LocalPacketConsumer : public rsp::PacketConsumer {
 public:
  explicit LocalPacketConsumer(debug::Blob* packet)
      : packet_(packet), success_(false) {}
  virtual void OnPacket(const debug::Blob& body, bool valid_checksum) {
    *packet_ = body;
    success_ = true;
  }
  virtual void OnUnexpectedChar(char unexpected_char) {}
  virtual void OnBreak() {}

  debug::Blob* packet_;
  bool success_;
};

void Escape(const debug::Blob& blob_in, debug::Blob* blob_out);
}  // namespace

namespace rsp {
void PacketUtil::AddEnvelope(const debug::Blob& blob_in,
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

bool PacketUtil::RemoveEnvelope(const debug::Blob& blob_in,
                                 debug::Blob* blob_out) {
  blob_out->Clear();
  Packetizer pktz;
  LocalPacketConsumer consumer(blob_out);
  pktz.SetPacketConsumer(&consumer);
  pktz.OnData(blob_in);
  return consumer.success_;
}
}  // namespace rsp

namespace {
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
}  // namespace

