// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_PROCESS_H_
#define DEBUGGER_CORE_DEBUGGEE_PROCESS_H_
#include <windows.h>
#include <deque>
#include <map>
#include "debugger/core/debuggee_iprocess.h"
#include "debugger/core/debuggee_thread.h"

/// \brief This namespace groups classes related to OOP (out-of-process)
/// Windows debugger.
namespace debug {
class Breakpoint;
class DebugAPI;

/// This class represents a process in debugged application.

/// Class diagram (and more) is here:
/// https://docs.google.com/a/google.com/document/d/1lTN-IYqDd_oy9XQg9-zlNc_vbg-qyr4q2MKNEjhSA84/edit?hl=en&authkey=CJyJlOgF#
class DebuggeeProcess : public IDebuggeeProcess {
 public:
  /// Creates a DebuggeeProcess object. There's no need to close
  /// |handle|, system will close handle when thread terminates.
  /// |file_handle| shall be closed.
  /// @param[in] id process id
  /// @param[in] handle a handle to the process
  /// @param[in] file_handle a handle to the process's image file
  /// @param[in] debug_api pointer to DebugAPI object,
  ///  system debug API (or mock) is called though 'debug_api'.
  ///  DebuggeeProcess  don't not takes ownership of |debug_api|.
  ///  If |debug_api| is NULL, default DebugAPI is used,
  ///  the one that passes calls to system calls.
  DebuggeeProcess(int id,
                  HANDLE handle,
                  HANDLE file_handle,
                  DebugAPI& debug_api);
  virtual ~DebuggeeProcess();

  /// @return id of the process
  virtual int id() const { return id_; }

  /// @return handle of the process
  virtual HANDLE handle() const { return handle_; }

  /// @return handle of the process image file
  virtual HANDLE file_handle() const { return file_handle_; }

  virtual State state() const { return state_; }
  virtual DebugAPI& debug_api() { return debug_api_; }

  /// @return address of memory region where nexe is loaded.
  virtual void* nexe_mem_base() const { return nexe_mem_base_; }

  virtual void set_nexe_mem_base(void* addr) { nexe_mem_base_ = addr; }

  /// @return code address of nexe _start() routine.
  virtual void* nexe_entry_point() const { return nexe_entry_point_; }

  virtual void set_nexe_entry_point(void* addr) { nexe_entry_point_ = addr; }

  /// @return word size of the debuggee process (32 or 64).
  virtual int GetWordSizeInBits();

  /// @return true for WoW (windows-on-windows) processes -
  /// i.e. 32-bit processes running on 64-bit windows.
  virtual bool IsWoW();

  /// Allows process execution to continue (i.e. it calls
  /// ContinueDebugEvent() for halted thread).
  /// Shall be called only on halted process.
  virtual void Continue();

  /// Allows process execution to continue. If thread was halted due
  /// to exception, that exception is passed to the debugee thread.
  /// Shall be called only on halted process.
  virtual void ContinueAndPassExceptionToDebuggee();

  /// Cause halted thread to execute single CPU instruction.
  /// Shall be called only on halted process.
  virtual void SingleStep();

  /// Cause running process to break (calls
  /// debug::DebugApi::DebugBreakProcess).
  /// Shall not be called on halted process.
  virtual void Break();

  /// Terminates all threads of the process.
  virtual void Kill();

  /// Detaches debugger fom the process. Process is not killed.
  virtual void Detach();

  /// @return a pointer to the thread object, or NULL if there's
  /// no thread with such |id|.
  /// Thread object is owned by the process.
  virtual DebuggeeThread* GetThread(int id);

  /// @return a poiner to the halted thread object, or NULL
  /// if process is not halted.
  /// Thread object is owned by the process.
  virtual DebuggeeThread* GetHaltedThread();

  /// @return all thread ids.
  virtual void GetThreadIds(std::deque<int>* threads) const;

  /// Copies memory from debuggee process to debugger buffer.
  /// @param[in] addr address (in debugger address space) from where to read.
  /// @param[in] size number of bytes to read.
  /// @param[out] destination destination buffer (in debugger address space).
  virtual bool ReadMemory(const void* addr, size_t size, void* destination);

  /// Copies memory from debugger to debuggee process.
  /// @param[in] addr address (in debugger address space) where to write.
  /// @param[in] size number of bytes to write.
  /// @param[in] source address of source buffer.
  virtual bool WriteMemory(const void* addr, size_t size, const void* source);

  /// Sets breakpoint at specified address |addr|.
  /// @param[in] addr address where breakpoint shall be.
  /// @return false if process is not able to access memory at |addr|,
  /// or process is not halted, or if there is breakpoint with the same |addr|.
  virtual bool SetBreakpoint(void* addr);

  /// Removes breakpoint at specified address |addr|.
  /// @param[in] addr address of breakpoint.
  /// @return false if process is not halted.
  virtual bool RemoveBreakpoint(void* addr);

  /// @return breakpoint object, or NULL if there's no breakpoint
  /// set at |addr|.
  /// @param[in] addr address of breakpoint.
  virtual Breakpoint* GetBreakpoint(void* addr);

  /// Return all breakpoints.
  /// @param[out] breakpoints
  virtual void GetBreakpoints(std::deque<Breakpoint*>* breakpoints);

 protected:
  friend class ExecutionEngine;

  /// Handler of debug events.
  /// @param[in] debug_event debug event received from debuggee process
  virtual void OnDebugEvent(DebugEvent* debug_event);

  /// Adds DebuggeeThread object with specified |id| and |handle|.
  /// @param id thread id
  /// @param handle handle to the thread object, should come from
  /// CREATE_THREAD_DEBUG_EVENT or CREATE_PROCESS_DEBUG_EVENT debug events
  /// Don't call ::CloseHandle on it, system releases it when thread is gone.
  /// @return pointer to new thread object, DebuggeeProcess owns it.
  /// Don't delete returned object.
  /// If there's already thread with |id|, no new object is created,
  /// pointer to existing one is returned.
  virtual DebuggeeThread* AddThread(int id, HANDLE handle);

  virtual void ContinueHaltedThread(DebuggeeThread::ContinueOption option);

  /// Removes DebuggeeThread object with specified |id|. System thread
  /// structures are not affected.
  /// Called by DebuggeeProcess when thread is gone.
  /// @param id thread id
  virtual void RemoveThread(int id);

  /// Delets all thread objects. System structures are not affected.
  virtual void DeleteThreads();

  DebugAPI& debug_api_;
  State state_;
  int id_;
  HANDLE handle_;
  HANDLE file_handle_;
  std::deque<DebuggeeThread*> threads_;
  std::map<void*, Breakpoint*> breakpoints_;
  void* nexe_mem_base_;
  void* nexe_entry_point_;

 private:
  DebuggeeProcess(const DebuggeeProcess&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const DebuggeeProcess&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUGGEE_PROCESS_H_

