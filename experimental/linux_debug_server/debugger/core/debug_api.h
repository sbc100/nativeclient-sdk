// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_API_H_
#define DEBUGGER_CORE_DEBUG_API_H_
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <string>

#include "debugger/base/debug_blob.h"
#include "debugger/core/debug_event.h"

namespace debug {
/// This class is a layer on top of linux ptrace() syscall.
class DebugAPI {
 public:
  DebugAPI() {}

  bool StartProcess(const char* cmd_line,	bool trace,	pid_t* child_pid_out);
  bool SetupProc(pid_t pid);

  bool WaitForDebugEvent(DebugEvent* de);

  bool ContinueDebugEvent(pid_t pid, int signo);
  bool SingleStep(pid_t pid);
  bool PostSignal(pid_t pid, int signo);
  bool DebugBreak(pid_t pid);

  bool ReadMemory(pid_t pid,
                  uint64_t addr,
                  void* dest,
                  size_t size,
                  size_t* readed_bytes_out);

  bool WriteMemory(pid_t pid,
                   uint64_t addr,
                   void* src,
                   size_t size,
                   size_t* written_bytes_out);

  bool ReadThreadContext(pid_t pid, user_regs_struct* context);
  bool WriteThreadContext(pid_t pid, user_regs_struct* context);

  bool ReadDebugString(const DebugEvent& de, std::string* string);

 private:
  DebugAPI(const DebugAPI&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const DebugAPI&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUG_API_H_

