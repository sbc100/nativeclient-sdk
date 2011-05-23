// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_event.h"
#include <string>

namespace debug {
DebugEvent::DebugEvent()
  : nacl_debug_event_code_(kNotNaClDebugEvent) {
  memset(&windows_debug_event_, 0, sizeof(windows_debug_event_));
}

bool DebugEvent::IsBreakpoint() const {
  return EXCEPTION_BREAKPOINT == GetExceptionCode();
}

bool DebugEvent::IsSingleStep() const {
  return EXCEPTION_SINGLE_STEP == GetExceptionCode();
}

int DebugEvent::GetExceptionCode() const {
  if (EXCEPTION_DEBUG_EVENT != windows_debug_event_.dwDebugEventCode)
    return 0;
  return windows_debug_event_.u.Exception.ExceptionRecord.ExceptionCode;
}

void DebugEvent::ToString(char* buff, size_t size) const {
  _snprintf_s(
      buff,
      size - 1,
      _TRUNCATE,
      "dwDebugEventCode=0x%X dwProcessId=0x%X dwThreadId=0x%X NaClEventId=%d",
      windows_debug_event_.dwDebugEventCode,
      windows_debug_event_.dwProcessId,
      windows_debug_event_.dwThreadId,
      nacl_debug_event_code_);
  buff[size - 1] = 0;
}
}  // namespace debug

