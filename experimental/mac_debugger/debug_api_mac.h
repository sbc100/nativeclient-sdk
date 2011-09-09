// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef EXPERIMENTAL_MAC_OOP_DEBUGGER_DEBUG_API_MAC_H_
#define EXPERIMENTAL_MAC_OOP_DEBUGGER_DEBUG_API_MAC_H_

#ifndef WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <mach/mach_vm.h>
#include <mach/mach.h>
#else
#include "mac_sys_mock.h"
#endif

#include <deque>
#include <string>

namespace debug {
enum ProcessState {
  PROCESS_RUNNING,
  PROCESS_STOPPED,
  PROCESS_TERMINATED,
  PROCESS_EXITED
};

// mach exceptions:
#define MACH_EXCEPTIONS_START_CODE 2000
#define MACH_BAD_ACCESS MACH_EXCEPTIONS_START_CODE + EXC_BAD_ACCESS
#define MACH_BAD_INSTRUCTION MACH_EXCEPTIONS_START_CODE + EXC_BAD_INSTRUCTION
#define MACH_ARITHMETIC MACH_EXCEPTIONS_START_CODE + EXC_ARITHMETIC
#define MACH_BREAKPOINT MACH_EXCEPTIONS_START_CODE + EXC_BREAKPOINT

/// Struct that receives information about the debugging event.
struct DebugEvent {
  DebugEvent()
      : pid_(0),
        process_state_(PROCESS_STOPPED),
        signal_no_(0),
        exit_code_(0),
        //child_pid_(0),
        thread_(0),
        task_(0) {}
        
  void Print();        

  int pid_;
  ProcessState process_state_;
  int signal_no_;  // Unix signals + mach exception codes
  int exit_code_;
  //int child_pid_;  // Relevent only for SIGCHLD signal
  
  // mach stuff
  mach_port_t task_;
  mach_port_t thread_;  // can be used as tid
};

class DebugAPI {
 public:
  DebugAPI();
  
  bool StartProcess(const char* cmd_line,
                            bool trace,
                            pid_t* child_pid_out);
                            
  bool WaitForDebugEvent(int wait_ms, DebugEvent* de);
  bool ContinueDebugEvent(DebugEvent de, int signo);

  bool SingleStep(DebugEvent de);
  bool PostSignal(pid_t pid, int signo);
  bool DebugBreak(pid_t pid);

  bool GetThreadList(pid_t pid, std::deque<int>* list);

  bool ReadProcessMemory(pid_t pid,
                         uint64_t addr,
                         void* dest,
                         size_t size,
                         size_t* readed_bytes_out);

  bool WriteProcessMemory(pid_t pid,
                          uint64_t addr,
                          void* src,
                          size_t size,
                          size_t* written_bytes_out);

  bool ReadThreadContext(int tid, x86_thread_state* context);
  bool WriteThreadContext(int tid, const x86_thread_state& context);

  bool ReadDebugString(const DebugEvent& de, std::string* string);

  bool ReadIP(int tid, unsigned int* ip);
  bool WriteIP(int tid, unsigned int ip);

  bool EnableSingleStep(int pid, bool enable);
  
  // br_no should be 0..3
  bool SetHwBreakpoint(int tid, uint64_t addr, int br_no);

  // Setups debuggee process to send mach exceptions our way.
  // Shall be called only once.
  bool HookupDebugeeProcess(pid_t pid);
    
 private:
  DebugAPI(const DebugAPI&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const DebugAPI&);

  bool WaitForMachException(int wait_ms, DebugEvent* de);
  
  size_t page_size_;
  mach_port_t exception_port_;  // Exception port on which we will receive
                                // exceptions from debugee processes. 
};

}  // namespace debug
#endif  // EXPERIMENTAL_MAC_OOP_DEBUGGER_DEBUG_API_MAC_H_

