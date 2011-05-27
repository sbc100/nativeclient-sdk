// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_process.h"
#include <assert.h>
#include "debugger/core/debug_api.h"
#include "debugger/core/debug_breakpoint.h"
#include "debugger/core/debug_event.h"

namespace debug {
DebuggeeProcess::DebuggeeProcess(int id,
                                 HANDLE handle,
                                 HANDLE file_handle,
                                 DebugAPI* debug_api)
  : id_(id),
    handle_(handle),
    file_handle_(file_handle),
    state_(kRunning),
    exit_code_(0),
    debug_api_(*debug_api),
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
#ifndef _WIN64
  return false;
#else
  BOOL is_wow = FALSE;
  if (!debug_api().IsWoW64Process(handle_, &is_wow))
    return false;
  return is_wow ? true : false;
#endif
}

void* DebuggeeProcess::FromNexeToFlatAddress(void* addr) const {
#ifndef _WIN64
  addr = reinterpret_cast<char*>(addr) +
      reinterpret_cast<size_t>(nexe_mem_base_);
#endif
  return addr;
}

bool DebuggeeProcess::Continue() {
  return ContinueHaltedThread(DebuggeeThread::kContinue);
}

bool DebuggeeProcess::ContinueAndPassExceptionToDebuggee() {
  return ContinueHaltedThread(DebuggeeThread::kContinueAndPassException);
}

bool DebuggeeProcess::SingleStep() {
  return ContinueHaltedThread(DebuggeeThread::kSingleStep);
}

bool DebuggeeProcess::Break() {
  return (FALSE != debug_api().DebugBreakProcess(handle_));
}

bool DebuggeeProcess::Kill() {
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    ++it;
    if (NULL != thread)
      thread->Kill();
  }
  return Continue();
}

bool DebuggeeProcess::Detach() {
  BOOL res = debug_api().DebugActiveProcessStop(id());
  DeleteThreads();
  state_ = kDead;
  return (FALSE != res);
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
  if (!debug_api().ReadProcessMemory(handle_, addr, destination, size, NULL)) {
    return false;
  }
  return true;
}

bool DebuggeeProcess::WriteMemory(const void* addr,
                                  size_t size,
                                  const void* source) {
  if (!IsHalted())
    return false;
  // There's no need to change memory protection, because debugger
  // has full access to the debuggee memory.
  // The function fails if the requested write operation crosses into an area
  // of the process that is inaccessible.
  BOOL res = debug_api().WriteProcessMemory(handle_,
                                            const_cast<void*>(addr),
                                            source,
                                            size,
                                            NULL);
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
  if (!IsHalted())
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
  if (!IsHalted())
    return false;

  bool result = true;
  std::map<void*, Breakpoint*>::iterator it = breakpoints_.find(addr);
  if (breakpoints_.end() != it) {
    Breakpoint* br = it->second;
    if (!br->RecoverCodeAtBreakpoint())
      result = false;
    delete br;
    breakpoints_.erase(it);
  }
  return result;
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
  last_debug_event_ = *debug_event;
  DEBUG_EVENT wde = debug_event->windows_debug_event();

  switch (wde.dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT: {
      AddThread(
          wde.dwThreadId,
          wde.u.CreateProcessInfo.hThread);
      break;
    }
    case CREATE_THREAD_DEBUG_EVENT: {
      AddThread(
          wde.dwThreadId,
          wde.u.CreateThread.hThread);
      break;
    }
    case EXIT_PROCESS_DEBUG_EVENT: {
      exit_code_ = wde.u.ExitProcess.dwExitCode;
      break;
    }
  }
  DebuggeeThread* thread = GetThread(wde.dwThreadId);
  if (NULL != thread) {
    thread->OnDebugEvent(debug_event);
    if (thread->IsHalted())
      state_ = kHalted;
  } else {
    /// To prevent halting the process in case we lost the thread
    /// object somehow.
    debug_api().ContinueDebugEvent(id(), wde.dwThreadId, DBG_CONTINUE);
  }
}

bool DebuggeeProcess::ContinueHaltedThread(
    DebuggeeThread::ContinueOption option) {
  if (state_ != kHalted)
    return false;

  DebuggeeThread* halted_thread = GetHaltedThread();
  assert(NULL != halted_thread);

  if (NULL != halted_thread) {
    bool res = halted_thread->Continue(option);
    if (halted_thread->state() == DebuggeeThread::kDead)
      RemoveThread(halted_thread->id());

    int last_debug_event_id =
      last_debug_event_.windows_debug_event().dwDebugEventCode;
    if (EXIT_PROCESS_DEBUG_EVENT == last_debug_event_id)
      state_ = kDead;
    else
      state_ = kRunning;
    return (TRUE == res);
  }
  return false;
}

DebuggeeThread* DebuggeeProcess::AddThread(int id, HANDLE handle) {
  DebuggeeThread* thread = GetThread(id);
  if (NULL == thread) {
    thread = new DebuggeeThread(id, handle, this);
    threads_.push_back(thread);
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

