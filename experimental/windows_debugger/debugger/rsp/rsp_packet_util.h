// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_RSP_PACKET_UTIL_H_
#define SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_RSP_PACKET_UTIL_H_

#include "debugger/base/debug_blob.h"

namespace rsp {
class PacketUtil {
 public:
  static void AddEnvelope(const debug::Blob& blob_in, debug::Blob* blob_out);
  static bool RemoveEnvelope(const debug::Blob& blob_in, debug::Blob* blob_out);
};
}  // namespace rsp

#endif  // SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_RSP_PACKET_UTIL_H_



