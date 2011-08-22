// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_thread.h"
#include "debugger/core/debug_api.h"
#include <signal.h>

namespace debug {
DebuggeeThread::DebuggeeThread(int id, DebugAPI* debug_api)
    : id_(id),
      state_(RUNNING),
      debug_api_(debug_api) {
}

DebugAPI& DebuggeeThread::debug_api() {
  return *debug_api_;
}

bool DebuggeeThread::Continue() {
  if (RUNNING == state_)
    return false;

  int signal_to_pass = last_debug_event_.signal_no_;
  if (SIGTRAP == signal_to_pass)  // TODO: shall we pass 0 for SIGSTOP too?
    signal_to_pass = 0;

  bool res = debug_api().ContinueDebugEvent(id_, signal_to_pass);
  if (res)
    state_ = RUNNING;
  return res;
}

void DebuggeeThread::OnDebugEvent(const DebugEvent& debug_event) {
  last_debug_event_ = debug_event;
  state_ = debug_event.process_state_;
}

}  // namespace debug

