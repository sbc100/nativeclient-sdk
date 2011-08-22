// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_process_mock.h"
#include "debugger/core/debuggee_thread.h"

namespace debug {
DebuggeeProcessMock::DebuggeeProcessMock(DebugAPI* debug_api)
    : debug_api_(debug_api),
      nexe_mem_base_(NULL),
      nexe_entry_point_(NULL),
      state_(kHalted),
      compatibility_mode_(false) {
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

bool DebuggeeProcessMock::ReadDebugString(std::string* debug_string) {
  DEBUG_EVENT de = last_debug_event().windows_debug_event();
  if (OUTPUT_DEBUG_STRING_EVENT != de.dwDebugEventCode)
    return false;

  // We don't support UNICODE debug strings yet.
  if (0 != de.u.DebugString.fUnicode)
    return false;

  // Read string from debuggee address space.
  char buff[2000];
  size_t str_len = min(sizeof(buff) - 1, de.u.DebugString.nDebugStringLength);
  if (ReadMemory(de.u.DebugString.lpDebugStringData, str_len, buff))
    buff[str_len - 1] = 0;

  if (NULL != debug_string)
    *debug_string = buff;
  return true;
}

void DebuggeeProcessMock::OnDebugEvent(DebugEvent* debug_event) {
  last_debug_event_ = *debug_event;
  DEBUG_EVENT wde = debug_event->windows_debug_event();
  debug::DebuggeeThread* thread = GetThread(wde.dwThreadId);
  if (NULL != thread) {
    thread->OnDebugEvent(debug_event);
    if (thread->IsHalted())
      state_ = kHalted;
  }
}

DebuggeeThread* DebuggeeProcessMock::GetThread(int id) {
  std::deque<DebuggeeThread*>::iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (thread->id() == id)
      return thread;
    ++it;
  }
  return NULL;
}

DebuggeeThread* DebuggeeProcessMock::AddThread(int id, HANDLE handle) {
  DebuggeeThread* thread = new DebuggeeThread(id, handle, this);
  threads_.push_back(thread);
  return thread;
}

}  // namespace debug

