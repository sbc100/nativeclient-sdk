// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api.h"
#include <algorithm>
#include "debugger/core/debug_execution_engine.h"
#include "debugger/core/debug_logger.h"
#include "debugger/core/debuggee_process.h"

namespace {
/// Timeout for exiting processes. If nothing happens for
/// kWaitOnExitMs milliseconds, ExecutionEngine quits waiting
/// for debug events.
int kWaitOnExitMs = 300;

bool not_dead_proc(debug::IDebuggeeProcess* proc) {
  return debug::IDebuggeeProcess::kDead != proc->state();
}
void delete_proc(debug::IDebuggeeProcess* proc) { delete proc; }
void detach_proc(debug::IDebuggeeProcess* proc) {
  proc->Detach();
}
void kill_proc(debug::IDebuggeeProcess* proc) {
  proc->Kill();
}
}  // namespace

namespace debug {

ExecutionEngine::ExecutionEngine(DebugAPI* debug_api)
  : debug_api_(*debug_api) {
}

ExecutionEngine::~ExecutionEngine() {
  Stop(kWaitOnExitMs);
  std::for_each(processes_.begin(), processes_.end(), delete_proc);
  processes_.clear();
}

IDebuggeeProcess* ExecutionEngine::CreateDebuggeeProcess(int pid,
                                                         HANDLE handle,
                                                         HANDLE file_handle) {
  return new DebuggeeProcess(pid, handle, file_handle, &debug_api_);
}

bool ExecutionEngine::StartProcess(const char* cmd, const char* work_dir) {
  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  char* cmd_dup = _strdup(cmd);
  if (NULL == cmd_dup) {
    DBG_LOG("TR01.00", "Memory allocation error.");
    return false;
  }
  BOOL res = debug_api_.CreateProcess(NULL,
                                      cmd_dup,
                                      NULL,
                                      NULL,
                                      FALSE,
                                      DEBUG_PROCESS | CREATE_NEW_CONSOLE,
                                      NULL,
                                      work_dir,
                                      &si,
                                      &pi);
  free(cmd_dup);
  if (!res)
    return false;

  debug_api_.CloseHandle(pi.hThread);
  debug_api_.CloseHandle(pi.hProcess);
  return true;
}

bool ExecutionEngine::AttachToProcess(int pid) {
  return (TRUE == debug_api_.DebugActiveProcess(pid)) ? true : false;
}

void ExecutionEngine::DetachAll() {
  std::for_each(processes_.begin(), processes_.end(), detach_proc);
  std::for_each(processes_.begin(), processes_.end(), delete_proc);
  processes_.clear();
}

IDebuggeeProcess* ExecutionEngine::GetProcess(int pid) {
  ProcessConstIter it = processes_.begin();
  while (it != processes_.end()) {
    IDebuggeeProcess* proc = *it;
    ++it;
    if (pid == proc->id())
      return proc;
  }
  return NULL;
}

void ExecutionEngine::GetProcessIds(std::deque<int>* processes) const {
  processes->clear();
  ProcessConstIter it = processes_.begin();
  while (it != processes_.end()) {
    processes->push_back((*it)->id());
    ++it;
  }
}

bool ExecutionEngine::HasAliveDebuggee() {
  RemoveDeadProcesses();
  std::deque<int> processes;
  GetProcessIds(&processes);
  return (processes.size() > 0);
}

void ExecutionEngine::RemoveDeadProcesses() {
  ProcessIter it = std::partition(processes_.begin(),
                                  processes_.end(),
                                  not_dead_proc);
  std::for_each(it, processes_.end(), delete_proc);
  processes_.erase(it, processes_.end());
}

bool ExecutionEngine::WaitForDebugEventAndDispatchIt(int wait_ms,
                                                     int* halted_pid) {
  RemoveDeadProcesses();
  DEBUG_EVENT de;
  if (debug_api_.WaitForDebugEvent(&de, wait_ms)) {
    int pid = OnDebugEvent(de);
    if (NULL != halted_pid)
      *halted_pid = pid;
    return true;
  }
  return false;
}

int ExecutionEngine::OnDebugEvent(const DEBUG_EVENT& debug_event) {
  debug_event_.set_windows_debug_event(debug_event);
  debug_event_.set_nacl_debug_event_code(DebugEvent::kNotNaClDebugEvent);

  IDebuggeeProcess* process = GetProcess(debug_event.dwProcessId);

  if (CREATE_PROCESS_DEBUG_EVENT == debug_event.dwDebugEventCode) {
    process = CreateDebuggeeProcess(debug_event.dwProcessId,
                                    debug_event.u.CreateProcessInfo.hProcess,
                                    debug_event.u.CreateProcessInfo.hFile);
    if (NULL != process)
      processes_.push_back(process);
  }

  char tmp[1000];
  debug_event_.ToString(tmp, sizeof(tmp));
  DBG_LOG("TR01.01", "msg='ExecutionEngine::OnDebugEvent' event='%s'", tmp);

  if (NULL != process) {
    process->OnDebugEvent(&debug_event_);
    if (process->IsHalted())
      return process->id();
  }
  return 0;
}

void ExecutionEngine::Stop(int wait_ms) {
  std::for_each(processes_.begin(), processes_.end(), kill_proc);
  while (processes_.size() > 0) {
    int halted_pid = 0;
    WaitForDebugEventAndDispatchIt(wait_ms, &halted_pid);
    if (0 != halted_pid) {
      IDebuggeeProcess* proc = GetProcess(halted_pid);
      proc->ContinueAndPassExceptionToDebuggee();
    } else {
      break;  // Timed-out, stop waiting for processes to shut down.
    }
  }
}
}  // namespace debug

