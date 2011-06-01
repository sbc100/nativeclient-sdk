// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_EVENT_H_
#define DEBUGGER_CORE_DEBUG_EVENT_H_
#include <windows.h>
#include <string>

namespace debug {
/// Class that receives information about the debugging event.
/// It's basically an extension to Windows DEBUG_EVENT, with
/// NaCl events added.
/// For example, Windows OUTPUT_DEBUG_STRING_EVENT event can
/// carry information about NaCl untrusted thread to be started.
///
/// Link to Windows documentation:
/// http://msdn.microsoft.com/en-us/library/ms681423%28v=VS.85%29.aspx
class DebugEvent {
 public:
  enum NaClDebugEventCode {
    kNotNaClDebugEvent = 0,
    kThreadIsAboutToStart = 1
  };

  DebugEvent();

  bool IsBreakpoint() const;
  bool IsSingleStep() const;
  int GetExceptionCode() const;
  void ToString(char* buff, size_t size) const;

  DEBUG_EVENT windows_debug_event() const { return windows_debug_event_; }
  NaClDebugEventCode nacl_debug_event_code() const {
    return nacl_debug_event_code_;
  }
  void set_windows_debug_event(const DEBUG_EVENT& debug_event) {
    windows_debug_event_ = debug_event;
  }
  void set_nacl_debug_event_code(NaClDebugEventCode event_code) {
    nacl_debug_event_code_ = event_code;
  }

 private:
  DEBUG_EVENT windows_debug_event_;
  NaClDebugEventCode nacl_debug_event_code_;
};
}  // namespace debug

#endif  // DEBUGGER_CORE_DEBUG_EVENT_H_

