// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef EXPERIMENTAL_LINUX_OOP_DEBUGGER_DEBUG_API_LINUX_H_
#define EXPERIMENTAL_LINUX_OOP_DEBUGGER_DEBUG_API_LINUX_H_

#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>

#include <deque>
#include <string>

// some usefull string functions
void Split(const char* str_in,
           const char* delimiters,
           std::deque<std::string>* out);
std::string GetAppPathOutOfCmdLine(const char* cmd_line);
std::string GetAppNameOutOfCmdLine(const char* cmd_line);

namespace debug {
class DebugEvent;

class DebugApi {
 public:
  DebugApi() {}
  virtual ~DebugApi() {}

  virtual bool StartProcess(const char* cmd_line,
                            bool trace,
                            pid_t* child_pid_out);
  virtual bool SetupProc(pid_t pid);
  virtual bool GetNewChildPid(pid_t pid, pid_t* child_pid_out);

  virtual bool DebugBreak(pid_t pid);
  virtual bool SingleStep(pid_t pid);

  virtual bool EnableSingleStep(pid_t pid, bool enable);
  virtual bool PostASignal(pid_t pid, int signo, int sig_value);
  virtual bool PostASignal(pid_t pid, int signo, void* sig_value);

  virtual bool WaitForDebugEvent(DebugEvent* de);  //TODO: add optional pid
  virtual bool ContinueDebugEvent(pid_t process_id, int signo);

  virtual bool ReadDebugString(DebugEvent* de, std::string* string);

  virtual bool ReadProcessMemory(pid_t pid,
                                 void* addr,
                                 void* dest,
                                 size_t size,
                                 size_t* readed_bytes_out);

  virtual bool WriteProcessMemory(pid_t pid,
                                  void* addr,
                                  void* src,
                                  size_t size,
                                  size_t* written_bytes_out);

  virtual bool ReadThreadContext(pid_t pid, user_regs_struct* context);
  virtual bool WriteThreadContext(pid_t pid, user_regs_struct* context);
  virtual void PrintThreadContext(const user_regs_struct& context);

  virtual bool GetIp(pid_t pid, char** ip);
  virtual bool SetIp(pid_t pid, char* ip);
  virtual bool GetRax(pid_t pid, char** ip);
};

static const int EVENT_FORK = (SIGTRAP | (PTRACE_EVENT_FORK << 8));
static const int EVENT_VFORK = (SIGTRAP | (PTRACE_EVENT_VFORK << 8));
static const int EVENT_CLONE = (SIGTRAP | (PTRACE_EVENT_CLONE << 8));
static const int EVENT_EXEC = (SIGTRAP | (PTRACE_EVENT_EXEC << 8));
static const int EVENT_VFORK_DONE = (SIGTRAP | (PTRACE_EVENT_VFORK_DONE << 8));
static const int EVENT_EXIT = (SIGTRAP | (PTRACE_EVENT_EXIT << 8));

class DebugEvent {
 public:
  enum EventCode {UNKNOWN,
                  HIT_BREAKPOINT,
                  SINGLE_STEP_TRAP,
                  OUTPUT_DEBUG_STRING,
                  PROCESS_TERMINATED,  // child process terminated due
                  // to the receipt of a signal that was not caught.
                  PROCESS_EXITED,
                  PROCESS_STOPPED,
                  PROCESS_CONTINUED_WITH_SIGCONT};

  DebugEvent() { Reset(); }

  void Reset();
  void Print();
  bool IsProcessDied() { return event_code_ == PROCESS_EXITED; }

  pid_t process_id_;
  EventCode event_code_;
  int signal_no_;
  int signal_code_;
  sigval signal_value_;
  int exit_code_;
  char* ip_;
  char* addr_;
};

}  // namespace debug
#endif  // EXPERIMENTAL_LINUX_OOP_DEBUGGER_DEBUG_API_LINUX_H_

