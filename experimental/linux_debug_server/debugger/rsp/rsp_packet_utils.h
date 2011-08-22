// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_PACKET_UTILS_H_
#define DEBUGGER_RSP_RSP_PACKET_UTILS_H_

#include "debugger/base/debug_blob.h"

namespace rsp {
/// This class provides ability to add or strip RSP package 'envelope' i.e.
/// start character, stop character, checksum etc.
///
/// http://sources.redhat.com/gdb/current/onlinedocs/gdb.html#Remote-Protocol
class PacketUtils {
 public:
  /// Adds RSP envelope.
  /// @param[in] blob_in message that has to go inside envelope
  /// @param[out] blob_out destination for the 'enveloped' message
  static void AddEnvelope(const debug::Blob& blob_in, debug::Blob* blob_out);

  /// Removes RSP envelope.
  /// @param[in] blob_in message in envelope
  /// @param[out] blob_out destination for the message
  /// @return true if |blob_in| is valid 'enveloped' RSP packet
  static bool RemoveEnvelope(const debug::Blob& blob_in, debug::Blob* blob_out);
};
}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_PACKET_UTILS_H_


