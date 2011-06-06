// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_process_mock.h"

namespace debug {
DebuggeeProcessMock::DebuggeeProcessMock(DebugAPI* debug_api)
    : debug_api_(debug_api),
      nexe_mem_base_(NULL),
      nexe_entry_point_(NULL),
      state_(kHalted) {
  memset(buff_, kFillChar, sizeof(buff_));
}

bool DebuggeeProcessMock::ReadMemory(const void* addr,
                                     size_t size,
                                     void* destination) {
  intptr_t offset = reinterpret_cast<int>(addr);
  if ((offset < sizeof(buff_)) &&
     ((offset + size) <= sizeof(buff_))) {
    memcpy(destination, &buff_[offset], size);
    return true;
  }
  return false;
}

bool DebuggeeProcessMock::WriteMemory(const void* addr,
                                      size_t size,
                                      const void* source) {
  intptr_t offset = reinterpret_cast<int>(addr);
  if ((offset < sizeof(buff_)) &&
     ((offset + size) <= sizeof(buff_))) {
    memcpy(&buff_[offset], source, size);
    return true;
  }
  return false;
}
}  // namespace debug

