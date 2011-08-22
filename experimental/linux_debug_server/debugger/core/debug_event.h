// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_EVENT_H_
#define DEBUGGER_CORE_DEBUG_EVENT_H_

namespace debug {
enum ProcessState {
  RUNNING,
  PROCESS_STOPPED,
  PROCESS_TERMINATED,
  PROCESS_EXITED
};

/// Struct that receives information about the debugging event.
struct DebugEvent {
  int pid_;
  ProcessState process_state_;
  int signal_no_;
  int exit_code_;
};

}  // namespace debug

#endif  // DEBUGGER_CORE_DEBUG_EVENT_H_

