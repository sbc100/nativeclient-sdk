// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_PROCESS_H_
#define DEBUGGER_CORE_DEBUGGEE_PROCESS_H_
#include <windows.h>
#include <deque>
#include <map>
#include <string>
#include "debugger/core/debug_event.h"
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
///
/// Note: most methods shall be called from one thread, this is limitation
/// of Windows debug API, here's links to Microsoft documentation:
/// http://msdn.microsoft.com/en-us/library/ms681423%28v=VS.85%29.aspx
/// http://msdn.microsoft.com/en-us/library/ms681675%28v=vs.85%29.aspx
/// Simple accessors can be called from any thread.
///
/// Note: not thread-safe.
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
                  DebugAPI* debug_api);
  virtual ~DebuggeeProcess();

  /// If enabled, DebuggeeProcess runs in the mode
  /// compatible with in-process debug stub:
  /// it decrements IP after hitting breakpoints
  void EnableCompatibilityMode() {
    compatibility_mode_ = true;
  }

  /// @return id of the process
  virtual int id() const { return id_; }

  /// @return handle of the process
  virtual HANDLE handle() const { return handle_; }

  /// @return handle of the process image file
  virtual HANDLE file_handle() const { return file_handle_; }

  virtual State state() const { return state_; }
  virtual bool IsHalted() const { return kHalted == state(); }

  /// Shall be called only on dead processes (i.e. state_ == kDead).
  /// @return exit code or exception number, if process is terminated
  /// by exception.
  virtual int return_code() const { return exit_code_; }

  /// @return reference to last received debug event
  virtual const DebugEvent& last_debug_event() const {
    return last_debug_event_;
  }

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
  /// Shall be called only on halted process, and only from the thread that
  /// started the debuggee.
  virtual bool Continue();

  /// Allows process execution to continue. If thread was halted due
  /// to exception, that exception is passed to the debugee thread.
  /// Shall be called only on halted process, and only from the thread that
  /// started the debuggee.
  virtual bool ContinueAndPassExceptionToDebuggee();

  /// Cause halted thread to execute single CPU instruction.
  /// Shall be called only on halted process, and only from the thread that
  /// started the debuggee.
  /// Does not block waiting for SINGLE_STEP exception.
  /// Other events can arrive before SINGLE_STEP due to activity of
  /// other threads.
  virtual bool SingleStep();

  /// Cause running process to break (calls
  /// debug::DebugApi::DebugBreakProcess).
  /// Shall not be called on halted process, and only from the thread that
  /// started the debuggee.
  /// Returns before process is stopped, it just initiates breaking debuggee
  /// process. As a result, system will create another thread in debuggee
  /// process, and then that tread executs a trap instruction, causing
  /// delivery of breakpoint exception to the debugger.
  virtual bool Break();

  /// Initiates termination of all threads of the process.
  /// Event loop should process exiting debug event before DebuggeeProcess
  /// object gets into kDead state and can be safely deleted.
  /// TODO(garianov): verify that |Kill| can be called from any thread.
  virtual bool Kill();

  /// Detaches debugger fom the process. Debuggee process is not killed,
  /// it runs freely after debugger is detached.
  /// TODO(garianov): verify that |Detach| can be called from any thread.
  virtual bool Detach();

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
  /// Shall be called only on halted process. There's no harm though if you
  /// call it on running process.
  /// @param[in] addr address (in debugger address space) from where to read.
  /// @param[in] size number of bytes to read.
  /// @param[out] destination destination buffer (in debugger address space).
  /// TODO(garianov): verify that |ReadMemory| can be called from any thread.
  virtual bool ReadMemory(const void* addr, size_t size, void* destination);

  /// Copies memory from debugger to debuggee process.
  /// Shall be called only on halted process.
  /// @param[in] addr address (in debugger address space) where to write.
  /// @param[in] size number of bytes to write.
  /// @param[in] source address of source buffer.
  /// TODO(garianov): verify that |WriteMemory| can be called from any thread.
  virtual bool WriteMemory(const void* addr, size_t size, const void* source);

  /// Reads string passed by OUTPUT_DEBUG_STRING_EVENT.
  /// Note: string data is located in debuggee process.
  /// Shall be called only on halted process.
  /// @param[out] debug_string destination for the string.
  /// @return true if last debug event was OUTPUT_DEBUG_STRING_EVENT and
  /// string data transfer from debuggee process was successful.
  virtual bool ReadDebugString(std::string* debug_string);

  /// Sets breakpoint at specified address |addr|.
  /// Shall be called only on halted process.
  /// Note: for NaCl threads, breakpoints are supported only in nexe code,
  /// i.e. breakpoints in TCB won't work.
  /// TODO(garianov): add support for breakpoints in TCB.
  /// @param[in] addr address where breakpoint shall be.
  /// @return false if process is not able to access memory at |addr|,
  /// or process is not halted, or if there is breakpoint with the same |addr|.
  virtual bool SetBreakpoint(void* addr);

  /// Removes breakpoint at specified address |addr|.
  /// Shall be called only on halted process.
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

  /// Converts relative pointer to flat(aka linear) process address.
  /// Calling this function makes sense only for nexe threads,
  /// it's safe to call for any thread.
  /// @param[in] addr relative pointer
  /// @return flat address
  virtual void* FromNexeToFlatAddress(void* addr) const;

  bool compatibility_mode() const {
    return compatibility_mode_;
  }

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

  virtual bool ContinueHaltedThread(DebuggeeThread::ContinueOption option);

  /// Removes DebuggeeThread object with specified |id|. System thread
  /// structures are not affected.
  /// Called by DebuggeeProcess when thread is gone.
  /// @param id thread id
  virtual void RemoveThread(int id);

  /// Delets all thread objects. System structures are not affected.
  virtual void DeleteThreads();

  DebugAPI& debug_api_;
  int id_;
  HANDLE handle_;
  HANDLE file_handle_;
  State state_;
  int exit_code_;
  DebugEvent last_debug_event_;
  std::deque<DebuggeeThread*> threads_;
  std::map<void*, Breakpoint*> breakpoints_;
  void* nexe_mem_base_;
  void* nexe_entry_point_;
  bool compatibility_mode_;

 private:
  DebuggeeProcess(const DebuggeeProcess&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const DebuggeeProcess&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUGGEE_PROCESS_H_

