// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_EXECUTION_ENGINE_H_
#define DEBUGGER_CORE_DEBUG_EXECUTION_ENGINE_H_

#include <windows.h>
#include <deque>
#include <map>
#include <string>
#include "debugger/core/debug_event.h"

namespace debug {
class IDebuggeeProcess;
class DebugAPI;

/// ExecutionEngine provides a central control point for the set of
/// debuggee processes.
///
/// It has methods for process creation, for attaching to the running
/// processes, to detach from debuggee. Tracks debugee processes as they come
/// and go. There could be any number of unrelated or related processes all
/// tracked by single ExecutionEngine instance.
/// Example:
///
/// debug::DebugApi debug_api;
/// debug::ExecutionEngine engine(&debug_api);
/// if (engine.StartProcess("chrome.exe")) {
///   // debugger event loop
///   while (true) {
///     int halted_pid = 0;
///     engine.WaitForDebugEventAndDispatchIt(20, &halted_pid);
///     if (0 != halted_pid) {
///       // Can set breakpoints here, read registers etc.
///       debug::DebuggeeProcess* halted_proc = engine.GetProcess(halted_pid);
///       halted_proc->Continue();  //or halted_proc->SingleStep();
///     } else {
///       // Process can be halted (similar to VisualStudio 'BreakAll' command)
///       std::deque<int> pids;
///       engine.GetProcessIds(&pids);
///       engine.GetProcess(pids[0])->Break();
///       // Debuggee process will receive breakpoint exception.
///     }
///   }
/// }
///
/// Note: all methods shall be called from one thread, this is a limitation
/// of Windows debug API, here's links to Microsoft documentation:
/// http://msdn.microsoft.com/en-us/library/ms681423%28v=VS.85%29.aspx
/// http://msdn.microsoft.com/en-us/library/ms681675%28v=vs.85%29.aspx
class ExecutionEngine {
 public:
  explicit ExecutionEngine(DebugAPI* debug_api);
  virtual ~ExecutionEngine();

  /// Starts debugee process, it will be attached to debugger.
  /// @param[in] cmd the command line to be executed.
  /// @param[in] work_dir the full path to the working directory for
  /// the process, or NULL.
  /// @return false if starting process fails
  virtual bool StartProcess(const char* cmd, const char* work_dir);

  /// Enables a debugger to attach to an active process and debug it.
  /// @param[in] pid process id for the process to be debugged.
  /// @return false if attaching to the process fails
  virtual bool AttachToProcess(int pid);

  /// Stops the debugger from debugging all process that it's tracking now.
  /// Debuggee processes are not killed.
  /// Individual processes can be detached by calling
  /// IDebeggeeProcess::Detach().
  virtual void DetachAll();

  /// @param[in] wait_ms number of milliseconds to wait for a debugging event.
  /// @param[out] halted_pid pointer to integer that receives a halted
  /// process id or 0 if no process got halted as a result of this method call.
  /// @return true if debug event is received (and dispatched).
  /// Current implementation halts processes on all debug events,
  /// with single exception - when processing SINGLE_STEP in the process of
  /// continuing from breakpoinnt (see DebuggeeThread for more information).
  virtual bool WaitForDebugEventAndDispatchIt(int wait_ms, int* halted_pid);

  /// Initiates termination of all traced processes and waits
  /// until all debugee processes terminates.
  /// @param wait_ms number of milliseconds to wait for processes to stop.
  virtual void Stop(int wait_ms);

  /// @param pid process id
  /// @return pointer to traced process object, or NULL if there's no process
  /// with requested pid.
  /// ExecutionEngine owns returned process, caller shall not delete it.
  virtual IDebuggeeProcess* GetProcess(int pid);

  /// @param[out] pids list of all tracked processes
  virtual void GetProcessIds(std::deque<int>* pids) const;

  /// @return true if ExecutionEngine traces at least one alive process.
  virtual bool HasAliveDebuggee();

  /// @return reference to latest received debug event
  DebugEvent& debug_event() { return debug_event_; }

 protected:
  /// Handler of debug events.
  /// @param[in] debug_event debug event received from debuggee process
  /// @return a halted process id or 0 if no process got halted as a result
  /// of this method call.
  virtual int OnDebugEvent(const DEBUG_EVENT& debug_event);

  DebugAPI& debug_api() { return debug_api_; }

  /// Factory method.
  /// Used in unit tests, to provide a way to create mock debugee process.
  /// param[in] pid process id
  /// param[in] handle a handle to the process
  /// param[in] a handle to the process's image file.
  /// @return newly created debugee process object
  virtual IDebuggeeProcess* CreateDebuggeeProcess(int pid,
                                                  HANDLE handle,
                                                  HANDLE file_handle);
  /// Delets dead processe objects.
  virtual void RemoveDeadProcesses();

  typedef std::deque<IDebuggeeProcess*>::iterator ProcessIter;
  typedef std::deque<IDebuggeeProcess*>::const_iterator ProcessConstIter;

 private:
  DebugEvent debug_event_;
  std::deque<IDebuggeeProcess*> processes_;
  DebugAPI& debug_api_;

  ExecutionEngine(const ExecutionEngine&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const ExecutionEngine&);
};
}  // namespace debug

#endif  // DEBUGGER_CORE_DEBUG_EXECUTION_ENGINE_H_

