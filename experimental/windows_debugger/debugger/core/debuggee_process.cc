// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debug_api.h"
#include "debugger/core/debug_breakpoint.h"
#include "debugger/core/debug_event.h"

namespace debug {
DebuggeeProcess::DebuggeeProcess(int id,
                                 HANDLE handle,
                                 HANDLE file_handle,
                                 DebugAPI& debug_api)
  : state_(kRunning),
    id_(id),
    handle_(handle),
    file_handle_(file_handle),
    debug_api_(debug_api),
    nexe_mem_base_(NULL),
    nexe_entry_point_(NULL) {
}

DebuggeeProcess::~DebuggeeProcess() {
  DeleteThreads();
  if (NULL != file_handle_) {
    debug_api().CloseHandle(file_handle_);
    file_handle_ = NULL;
  }
  // Delete breakpoints.
  std::map<void*, Breakpoint*>::const_iterator it = breakpoints_.begin();
  while (it != breakpoints_.end()) {
    delete it->second;
    ++it;
  }
  breakpoints_.clear();
}

/// Implementation relies on the fact that 32 bit debugger cannot run
/// 64 bit debuggee process (::CreateProcess with DEBUG_PROCESS fails).
/// Also, 32 bit debugger is not able to attach to 64 bit process.
///
/// Debugger x debuggee matrix:
/// a) 32 x 32 -> _WIN64 not defined, not WoW, returns 32
/// b) 32 x 64 -> ::CreateProcess fails, impossible to get here
/// c) 64 x 64 -> _WIN64 is defined, not WoW, returns 64
/// d) 64 x 32 -> _WIN64 is defined, WoW, returns 32
int DebuggeeProcess::GetWordSizeInBits() {
#ifndef _WIN64
  // Not 64-bit debugger, so must be a 32-bit debugger and debuggee.
  return 32;
#else
  if (IsWoW())
    return 32;
  return 64;
#endif
}

bool DebuggeeProcess::IsWoW() {
  BOOL is_wow = FALSE;
  if (!debug_api().IsWoW64Process(handle_, &is_wow))
    return false;
  return is_wow ? true : false;
}

void DebuggeeProcess::Continue() {
  ContinueHaltedThread(DebuggeeThread::kContinue);
}

void DebuggeeProcess::ContinueAndPassExceptionToDebuggee() {
  ContinueHaltedThread(DebuggeeThread::kContinueAndPassException);
}

void DebuggeeProcess::SingleStep() {
  ContinueHaltedThread(DebuggeeThread::kSingleStep);
}

void DebuggeeProcess::Break() {
  debug_api().DebugBreakProcess(handle_);
}

void DebuggeeProcess::Kill() {
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    ++it;
    if (NULL != thread)
      thread->Kill();
  }
  Continue();
}

void DebuggeeProcess::Detach() {
  debug_api().DebugActiveProcessStop(id());
  DeleteThreads();
  state_ = kDead;
}

DebuggeeThread* DebuggeeProcess::GetThread(int id) {
  std::deque<DebuggeeThread*>::iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (thread->id() == id)
      return thread;
    ++it;
  }
  return NULL;
}

DebuggeeThread* DebuggeeProcess::GetHaltedThread() {
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it++;
    if (thread->IsHalted())
      return thread;
  }
  return NULL;
}

void DebuggeeProcess::GetThreadIds(std::deque<int>* threads) const {
  if (NULL == threads)
    return;
  threads->clear();
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it++;
    threads->push_back(thread->id());
  }
}

bool DebuggeeProcess::ReadMemory(const void* addr,
                                 size_t size,
                                 void* destination) {
  // There's no need to change memory protection, because debugger
  // has full access to the debuggee memory.
  // The function fails if the requested read operation crosses into an area
  // of the process that is inaccessible.
  SIZE_T rd = 0;
  if (!debug_api().ReadProcessMemory(handle_, addr, destination, size, &rd)) {
    return false;
  }
  return true;
}

bool DebuggeeProcess::WriteMemory(const void* addr,
                                  size_t size,
                                  const void* source) {
  // There's no need to change memory protection, because debugger
  // has full access to the debuggee memory.
  // The function fails if the requested write operation crosses into an area
  // of the process that is inaccessible.
  SIZE_T wr = 0;
  BOOL res = debug_api().WriteProcessMemory(handle_,
                                            const_cast<void*>(addr),
                                            source,
                                            size,
                                            &wr);
  if (!res) {
    return false;
  }
  // Flushes the instruction cache for the debuggee process.
  // The CPU cannot detect the change, and may execute the old code it cached.
  res = debug_api().FlushInstructionCache(handle_, addr, size);
  if (!res) {
    return false;
  }
  return true;
}

bool DebuggeeProcess::SetBreakpoint(void* addr) {
  if (kHalted != state())
    return false;

  if (NULL != GetBreakpoint(addr))
    return false;

  Breakpoint* br = new Breakpoint(addr, this);
  if (br->Init()) {
    breakpoints_[addr] = br;
    return true;
  }
  delete br;
  return false;
}

Breakpoint* DebuggeeProcess::GetBreakpoint(void* addr) {
  std::map<void*, Breakpoint*>::iterator it = breakpoints_.find(addr);
  if (breakpoints_.end() == it)
    return NULL;
  return it->second;
}

bool DebuggeeProcess::RemoveBreakpoint(void* addr) {
  if (kHalted != state())
    return false;
  std::map<void*, Breakpoint*>::iterator it = breakpoints_.find(addr);
  if (breakpoints_.end() != it) {
    Breakpoint* br = it->second;
    br->RecoverCodeAtBreakpoint();
    delete br;
    breakpoints_.erase(it);
  }
  return true;
}

void DebuggeeProcess::GetBreakpoints(std::deque<Breakpoint*>* breakpoints) {
  if (NULL == breakpoints)
    return;
  breakpoints->clear();
  std::map<void*, Breakpoint*>::const_iterator it = breakpoints_.begin();
  while (it != breakpoints_.end()) {
    breakpoints->push_back(it->second);
    ++it;
  }
}

void DebuggeeProcess::OnDebugEvent(DebugEvent* debug_event) {
  switch (debug_event->windows_debug_event_.dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT: {
      AddThread(
          debug_event->windows_debug_event_.dwThreadId,
          debug_event->windows_debug_event_.u.CreateProcessInfo.hThread);
      break;
    }
    case CREATE_THREAD_DEBUG_EVENT: {
      AddThread(
          debug_event->windows_debug_event_.dwThreadId,
          debug_event->windows_debug_event_.u.CreateThread.hThread);
      break;
    }
    case EXIT_PROCESS_DEBUG_EVENT: {
      state_ = kDead;
      return;
    }
  }
  DebuggeeThread* thread =
      GetThread(debug_event->windows_debug_event_.dwThreadId);
  if (NULL != thread) {
    thread->OnDebugEvent(debug_event);
    if (thread->IsHalted())
      state_ = kHalted;
  }
}

void DebuggeeProcess::ContinueHaltedThread(
    DebuggeeThread::ContinueOption option) {
  DebuggeeThread* halted_thread = GetHaltedThread();

  if (NULL != halted_thread) {
    halted_thread->Continue(option);
    if (halted_thread->state() == DebuggeeThread::kDead)
      RemoveThread(halted_thread->id());
  }
}

DebuggeeThread* DebuggeeProcess::AddThread(int id, HANDLE handle) {
  DebuggeeThread* thread = GetThread(id);
  if (NULL == thread) {
    thread = new DebuggeeThread(id, handle, this);
    threads_.push_back(thread);
  } else {
    printf("");
  }
  return thread;
}

void DebuggeeProcess::RemoveThread(int id) {
  std::deque<DebuggeeThread*>::iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (thread->id() == id) {
      threads_.erase(it);
      delete thread;
      break;
    }
    ++it;
  }
}

void DebuggeeProcess::DeleteThreads() {
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    delete thread;
    ++it;
  }
  threads_.clear();
}
}  // namespace debug

