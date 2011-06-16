// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_BLOB_UTILS_H_
#define DEBUGGER_RSP_RSP_BLOB_UTILS_H_

#include "debugger/base/debug_blob.h"

namespace rsp {

  /// Removes 2 bytes from the front of this blob, converts it to integer
  /// Assumes hex test representation is in the blob.
  /// example: {0x31, 0x30, 0x33} -> 0x10 + {0x33}
  /// @param blob blob to perform operation on
  /// @return popped integer
  unsigned int PopInt8FromFront(debug::Blob* blob);

  /// Removes 4 bytes from the front of this blob, converts it to 32bit integer
  /// Assumes hex test representation is in the blob.
  /// example: {0x31, 0x30, 0x33, 0x39} -> 0x1039 + {}
  /// @param blob blob to perform operation on
  /// @return poped integer
  uint32_t PopInt32FromFront(debug::Blob* blob);

  /// Removes 8 bytes from the front of this blob, converts it to 64bit integer
  /// Assumes hex test representation is in the blob. Example:
  /// {0x31, 0x30, 0x33, 0x39, 0x34, 0x35, 0x36, 0x37} -> 0x10394567 + {}
  /// @param blob blob to perform operation on
  /// @return poped integer
  uint64_t PopInt64FromFront(debug::Blob* blob);

  /// removes space characters from front and from back of the blob.
  /// @param blob blob to perform operation on
  void RemoveSpacesFromBothEnds(debug::Blob* blob);

  /// removes space characters from front and from back of the blobs.
  /// @param blobs blobs to perform operation on
  void RemoveSpacesFromBothEnds(std::deque<debug::Blob>* blobs);

  /// Writes formatted data to Blob.
  /// @param blob blob to perform operation on
  /// @param[in] fmt string that contains the text to be written to the Blob.
  void Format(debug::Blob* blob, const char* fmt, ...);

}  // namespace rsp

#endif DEBUGGER_RSP_RSP_BLOB_UTILS_H_

