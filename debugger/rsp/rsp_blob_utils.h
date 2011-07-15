// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_BLOB_UTILS_H_
#define DEBUGGER_RSP_RSP_BLOB_UTILS_H_

#include <deque>
#include "debugger/base/debug_blob.h"

namespace rsp {
  /// These functions are used to parse RSP packets.
  /// http://sources.redhat.com/gdb/current/onlinedocs/gdb.html#Remote-Protocol

  /// Removes bytes from the front of the |blob|, converts it to integer
  /// Assumes hex test representation is in the blob.
  /// @param blob[in,out] blob to perform operation on
  /// @param result[out] destination for the popped integer
  /// @return false if |blob| has no valid hex characters in the front,
  /// example: PopIntFromFront("kaka", &v) -> false
  template <class T>
  bool PopIntFromFront(debug::Blob* blob, T* result) {
    if (NULL == result)
      return false;

    // 2 chars per 1 encoded byte
    size_t max_bytes_to_pop = sizeof(*result) * 2;
    size_t i = 0;
    for (; i < max_bytes_to_pop; i++) {
      if (0 == blob->size())
        break;

      unsigned int dig = 0;
      if (!debug::Blob::HexCharToInt(blob->Front(), &dig))
        break;  // Stop on first no-hex character.

      blob->PopFront();
      if (0 == i)
        *result = dig;
      else
        *result = (*result << 4) + dig;
    }
    return (i > 0);
  }

  /// removes space characters from front and from back of the blob.
  /// @param blob blob to perform operation on
  void RemoveSpacesFromBothEnds(debug::Blob* blob);

  /// removes space characters from front and from back of the blobs.
  /// @param blobs blobs to perform operation on
  void RemoveSpacesFromBothEnds(std::deque<debug::Blob>* blobs);

  /// Writes formatted data to Blob.
  /// @param blob blob to perform operation on
  /// @param[in] fmt string that contains the text to be written to the Blob.
  /// @return reference to |blob|
  debug::Blob& Format(debug::Blob* blob, const char* fmt, ...);

}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_BLOB_UTILS_H_

