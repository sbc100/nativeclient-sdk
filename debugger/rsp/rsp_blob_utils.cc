// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
#include "debugger/rsp/rsp_blob_utils.h"
#include <stdarg.h>
#include "debugger/rsp/rsp_packet.h"

namespace rsp {
unsigned int PopInt8FromFront(debug::Blob* blob) {
  if (0 == blob->size())
    return 0;

  unsigned int dig1 = 0;
  debug::Blob::HexCharToInt(blob->PopFront(), &dig1);
  if (0 == blob->size())
    return dig1;

  unsigned int dig2 = 0;
  debug::Blob::HexCharToInt(blob->PopFront(), &dig2);
  return (dig1 * 16) + dig2;
}

uint32_t PopInt32FromFront(debug::Blob* blob) {
  uint32_t res = 0;
  size_t nibbles_to_copy = sizeof(res) * 2;
  for (size_t i = 0; i < nibbles_to_copy; i++) {
    if (0 == blob->size())
      break;
    unsigned int dig = 0;
    debug::Blob::HexCharToInt(blob->PopFront(), &dig);
    res = (res << 4) + dig;
  }
  return res;
}

uint64_t PopInt64FromFront(debug::Blob* blob) {
  uint64_t res = 0;
  size_t nibbles_to_copy = sizeof(res) * 2;
  for (size_t i = 0; i < nibbles_to_copy; i++) {
    if (0 == blob->size())
      break;
    unsigned int dig = 0;
    debug::Blob::HexCharToInt(blob->PopFront(), &dig);
    res = (res << 4) + dig;
  }
  return res;
}

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

void RemoveSpacesFromBothEnds(std::deque<debug::Blob>* tokens) {
  std::deque<debug::Blob>::iterator it = tokens->begin();
  while (it != tokens->end()) {
    debug::Blob& token = *it++;
    RemoveSpacesFromBothEnds(&token);
  }
}

void Format(debug::Blob* blob, const char* fmt, ...) {
  va_list marker;
  va_start(marker, fmt);
  char buff[rsp::kMaxRspPacketSize * 2];
  signed int res = _vsnprintf_s(buff, sizeof(buff) - 1, fmt, marker);
  if (-1 != res) {
    buff[sizeof(buff) - 1] = 0;
    buff[res] = 0;
    blob->FromString(buff);
  }
}
}  // namespace rsp

