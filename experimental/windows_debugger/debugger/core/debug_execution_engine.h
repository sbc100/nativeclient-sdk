#pragma once
#include <string>
#include <deque>
#include <map>
#include <windows.h>
#include "debug_event.h"

namespace debug {
class DebuggeeProcess;
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
///     debug::DebuggeeProcess* halted_process = NULL;
///     engine.DoWork(20, DebuggeeProcess** halted_process);
///     if (NULL != halted_process) {
///       // Can set breakpoints here, read registers etc.
///       halted_process->Continue();  //or halted_process->SingleStep();
///     } else {
///       // Process can be halted (similar to VisualStudio 'BreakAll' command)
///       std::deque<int> pids;
///       engine.GetProcessesIds(&pids);
///       engine.GetProcess(pids[0])->Break();
///       // Debuggee process will receive breakpoint exception.
///     }
///   }
/// }
///
/// Note: all methods shall be called from one thread, this is limitation
/// of Windows debug API, here's links to Microsoft documentation:
/// http://msdn.microsoft.com/en-us/library/ms681423%28v=VS.85%29.aspx
/// http://msdn.microsoft.com/en-us/library/ms681675%28v=vs.85%29.aspx
///
/// And thread used with one ExecutionEngine shall not be simultaneously
/// used with another ExecutionEngine instance.
class ExecutionEngine
{
public:
  ExecutionEngine(DebugAPI& debug_api);
  virtual ~ExecutionEngine();

  /// Starts debugee process, it will be attached to debugger.
  /// @return false if starting process fails
  virtual bool StartProcess(const char* cmd, const char* work_dir);

  /// Enables a debugger to attach to an active process and debug it.
  /// @return false if attaching to the process fails
  virtual bool AttachToProcess(int id);

  /// Stops the debugger from debugging all process that it's tracking now.
  /// Debuggee processes are not killed.
  /// Individual processes can be detached by calling
  /// DebeggeeProcess::Detach().
  virtual void DetachAll();

  /// @return true if debug event is processed
  virtual bool DoWork(int wait_ms, DebuggeeProcess** halted_process);

  /// Initiates termination of all traced processes and waits
  /// until all debugee processes terminates.
  virtual void Stop();

  /// @param id process id
  /// @return pointer to DebuggeeProcess or NULL.
  /// ExecutionEngine owns returned process, don't delete it.
  virtual DebuggeeProcess* GetProcess(int id);

  /// @param[out] pids list of all tracked processes
  virtual void GetProcessesIds(std::deque<int>* pids) const;

  /// @return reference to lates debug event, receiving of that
  /// event cause |DoWork| to return.
  DebugEvent& debug_event() { return debug_event_; }

 protected:
  /// Handler of debug events.
  /// @param[in] debug_event debug event received from debuggee process
  /// @param[out] halted_process pointer to the halted process or NULL,
  /// if no process becomes halted in the process of |debug_event| processing.
  /// Current implementation halts processes on all debug events,
  /// with single exception - when processing SINGLE_STEP
  virtual void OnDebugEvent(const DEBUG_EVENT& debug_event,
                            DebuggeeProcess** halted_process);

  /// Searches for dead processes and delets one DebuggeeProcess
  /// if it finds any dead one.
  virtual void RemoveDeadProcess();

  virtual DebuggeeProcess* CreateDebuggeeProcess(int id,
                                                 HANDLE handle,
                                                 HANDLE file_handle,
                                                 DebugAPI& debug_api);

  DebugEvent debug_event_;
  std::deque<DebuggeeProcess*> processes_;
  DebugAPI& debug_api_;

 private:
  ExecutionEngine(const ExecutionEngine&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const ExecutionEngine&);
};
}  // namespace debug
