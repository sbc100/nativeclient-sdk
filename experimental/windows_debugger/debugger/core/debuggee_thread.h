// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_THREAD_H_
#define DEBUGGER_CORE_DEBUGGEE_THREAD_H_

#include <windows.h>
#include "debugger/core/debug_breakpoint.h"

namespace debug {
class IDebuggeeProcess;
class DebugAPI;
class DebugEvent;
class Breakpoint;

/// This class represents a thread in the debugged process.

/// Each thread belongs to one DebuggeeProcess.
///
/// Class diagram (and more) is here:
/// https://docs.google.com/a/google.com/document/d/1lTN-IYqDd_oy9XQg9-zlNc_vbg-qyr4q2MKNEjhSA84/edit?hl=en&authkey=CJyJlOgF#
class DebuggeeThread {
 public:
  enum State {
    kRunning = 1,  // thread is alive, event loop is running
    kHalted,  // thread is alive, event loop is not running
    kContinueFromBreakpoint,  // thread is single stepping from breakpoint
    kDead  // thread is deleted by OS
  };

  /// Describes a parameter type for |Continue| method.
  enum ContinueOption {
    kSingleStep,
    kContinue,
    kContinueAndPassException
  };

  /// Creates a DebuggeeThread object with specified thread |id|,
  /// thread |handle| and |parent_process|. There's no need to close
  /// |handle|, system will close handle when thread terminates.
  /// |parent_process| shall not be NULL.
  DebuggeeThread(int id, HANDLE handle, IDebuggeeProcess* parent_process);

  int id() const { return id_; }
  HANDLE handle() const { return handle_; }
  State state() const { return state_; }

  /// @return true if this thread runs nexe code.
  bool is_nexe() const { return is_nexe_; }

  /// Used for debugging debugger.
  /// @param[in] state
  /// @return name of the state
  static const char* GetStateName(State state);

  /// Used for debugging debugger.
  /// @param[in] continue option
  /// @return name of the option
  static const char* GetContinueOptionName(ContinueOption option);

  /// @return true if thread is in kHalted state
  /// Halted thread is the one that caused the process to halt.
  /// Note: only one thread can be halted in one process.
  bool IsHalted() const;

  /// Reads registers of the thread.
  /// Note that CONTEXT structure is defined differently
  /// on 32-bit and 64-bit windows.
  /// @return true if operation was successful.
  bool GetContext(CONTEXT* context);

  /// Writes registers of the thread.
  /// @return true if operation was successful.
  bool SetContext(const CONTEXT& context);

  /// Reads registers of the WoW thread.
  /// It should be used to work with WoW (windows-on-windows)
  /// processes - i.e. 32-bit processes running on 64-bit windows.
  /// @return true if operation was successful.
  bool GetWowContext(WOW64_CONTEXT* context);

  /// Writes registers of the WoW thread.
  /// It should be used to work with WoW (windows-on-windows)
  /// processes - i.e. 32-bit processes running on 64-bit windows.
  /// @return true if operation was successful.
  bool SetWowContext(const WOW64_CONTEXT& context);

  /// Reads IP (instruction pointer).
  /// @return value of EIP (for 32-bit process) or RIP (for 64-bit process).
  void* GetIP();

  /// Writes IP.
  /// Writes EIP (for 32-bit process) or RIP (for 64-bit process).
  void SetIP(void* ip);

  /// Converts relative pointer to flat(aka linear) process address.
  /// Calling this function makes sense only for nexe threads,
  /// it's safe to call for any thread.
  /// @param[in] ptr relative pointer
  /// @return flat address
  void* FromNexeToFlatAddress(void* ptr) const;

 protected:
  friend class DebuggeeProcess;

  /// @return parent process.
  IDebuggeeProcess& parent_process() { return parent_process_; }
  const IDebuggeeProcess& parent_process() const { return parent_process_; }
  DebugAPI& debug_api();

  /// Allows thread execution to continue (i.e. it calls
  /// ContinueDebugEvent()).
  /// If |option| is kContinueAndPassException, and thread was halted due
  /// to exception, that exception is passed to the debuggee thread.
  void Continue(ContinueOption option);

  /// Handler of debug events. DebuggeeThread has a FSM (finite state machine),
  /// and |debug_event| is an only event consumed by FSM.
  /// @param[in] debug_event debug event received from debuggee process
  void OnDebugEvent(DebugEvent* debug_event);

  /// Terminates thread.
  void Kill();

  /// Changes internal state to |new_state|.
  void SetState(State new_state);

  /// Changes a 'Trace' flag in CPUs EFlags register.
  /// @param[in] enable single step
  void EnableSingleStep(bool enable);

  /// Handler for OUTPUT_DEBUG_STRING_EVENT.
  /// @param[in] debug_event debug event received from debuggee process
  void OnOutputDebugString(DebugEvent* debug_event);

  /// Handler for EXCEPTION_DEBUG_EVENT.EXCEPTION_kBreakPOINT
  /// @param[in] debug_event debug event received from debuggee process
  void OnBreakpoint(DebugEvent* debug_event);

  /// Handler for EXCEPTION_DEBUG_EVENT.EXCEPTION_SINGLE_kStep
  /// @param[in] debug_event debug event received from debuggee process
  void OnSingleStep(DebugEvent* debug_event);

  /// Resumes execution of the halted thread, asuming breapoint was triggered.
  void ContinueFromBreakpoint();

  int id_;
  HANDLE handle_;
  IDebuggeeProcess& parent_process_;
  State state_;
  int last_debug_event_id_;

  /// Current breakpoint, if any. NULL if thread did not hit breakpoint.
  void* triggered_breakpoint_addr_;

  // Stuff related only to nexe threads.
  bool is_nexe_;

 private:
  DebuggeeThread(const DebuggeeThread&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const DebuggeeThread&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUGGEE_THREAD_H_

