// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_THREAD_H_
#define DEBUGGER_CORE_DEBUGGEE_THREAD_H_

#include "debugger/core/debug_event.h"

namespace debug {
class DebugAPI;
class DebugEvent;

/// This class represents a NaClApp thread in the debugged process.

class DebuggeeThread {
 public:
  /// Creates a DebuggeeThread object with specified thread |id|,
  DebuggeeThread(int id, DebugAPI* debug_api);

  int id() const { return id_; }
  ProcessState state() const { return state_; }
  DebugEvent last_debug_event() const { return last_debug_event_; }

  void OnDebugEvent(const DebugEvent& debug_event);

  /// Allows thread execution to continue (i.e. it calls
  /// ContinueDebugEvent()).
  bool Continue();

 protected:
  DebugAPI& debug_api();

 private:
  int id_;
  ProcessState state_;
  DebugEvent last_debug_event_;
  DebugAPI* debug_api_;

  DebuggeeThread(const DebuggeeThread&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const DebuggeeThread&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUGGEE_THREAD_H_

