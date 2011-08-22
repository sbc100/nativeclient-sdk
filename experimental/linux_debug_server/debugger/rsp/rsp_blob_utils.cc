// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_blob_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include "debugger/rsp/rsp_packets.h"

namespace rsp {
void RemoveSpacesFromBothEnds(debug::Blob* blob) {
  while (blob->size() != 0) {
    char c = blob->Front();
    if (isspace(c))
      blob->PopFront();
    else
      break;
  }
  while (blob->size() != 0) {
    char c = blob->Back();
    if (isspace(c))
      blob->PopBack();
    else
      break;
  }
}

void RemoveSpacesFromBothEnds(std::deque<debug::Blob>* blobs) {
  std::deque<debug::Blob>::iterator it = blobs->begin();
  while (it != blobs->end()) {
    debug::Blob& blob = *it++;
    RemoveSpacesFromBothEnds(&blob);
  }
}

debug::Blob& Format(debug::Blob* blob, const char* fmt, ...) {
  va_list marker;
  va_start(marker, fmt);
  char buff[rsp::kMaxRspPacketSize * 2];
#ifdef _WIN32
  signed int res = _vsnprintf_s(buff, sizeof(buff) - 1, fmt, marker);
#else
	signed int res = vsnprintf(buff, sizeof(buff) - 1, fmt, marker);
#endif
  if (-1 != res) {
    buff[sizeof(buff) - 1] = 0;
    buff[res] = 0;
    blob->FromString(buff);
  }
  return *blob;
}
}  // namespace rsp

