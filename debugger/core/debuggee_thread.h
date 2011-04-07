#ifndef NACL_SDK_DEBUG_CORE_DEBUGGEE_THREAD_H_
#define NACL_SDK_DEBUG_CORE_DEBUGGEE_THREAD_H_

#include <windows.h>
#include "debuggee_breakpoint.h"

namespace debug {
class DebuggeeProcess;
class DebugAPI;
class ExecutionEngineEventObserver;
class ExecutionEngineInsideObserver;
class DecisionToContinue;

/// This class represents a thread in debugged process.
/// Each thread belongs to one DebugeeProcess.
///
/// Class diagram (and more) is here:
/// https://docs.google.com/a/google.com/document/d/1lTN-IYqDd_oy9XQg9-zlNc_vbg-qyr4q2MKNEjhSA84/edit?hl=en&authkey=CJyJlOgF#
class DebuggeeThread {
 public:
  enum State {RUNNING, HALTED, ZOMBIE, DEAD};
  enum UserCommand {NONE, CONTINUE, STEP, BREAK, KILL};

  /// Creates a DebugeeThread object with specified thread |id|,
  /// thread |handle| and |parent_process|. There's no need to close
  /// |handle|, system will close handle when thread terminates.
  /// |parent_process| shall not be NULL.
  DebuggeeThread(int id, HANDLE handle, DebuggeeProcess* parent_process);
  virtual ~DebuggeeThread();

  virtual DebuggeeProcess& process();  /// @return parent process.
  virtual const DebuggeeProcess& process() const;
  virtual DebugAPI& debug_api();
  virtual ExecutionEngineEventObserver& event_observer();

  virtual int id() const;
  virtual State state() const;
  static const char* GetStateName(State state);
  virtual bool is_nexe() const;
  virtual void* nexe_mem_base() const;
  virtual void* nexe_entry_point() const;

  /// @return true if debug events are not processed for this thread.
  /// I.e. when ContinueDebugEvent() is not called.
  /// Only one thread can be halted in one process.
  virtual bool IsHalted() const;

  virtual void Kill();  // Terminates thread.

  /// Allows thread execution to continue (i.e. it calls ContinueDebugEvent()).
  virtual void Continue();

  /// Allows thread execution to continue. If thread was halted due to exception,
  /// that exception is passed to the debugee thread.
  virtual void ContinueAndPassExceptionToDebuggee();

  /// Cause thread to execute single CPU instruction.
  virtual void SingleStep();

  /// Note that CONTEXT structure is defined differently
  /// on 32-bit and 64-bit windows.
  /// @return true if operation was successful.
  virtual bool GetContext(CONTEXT* context);  // Reads registers of the thread.
  virtual bool SetContext(const CONTEXT& context);  // Writes registers.

  /// These functions should be used to work with WoW (windows-on-windows)
  /// processes - i.e. 32-bit processes running on 64-bit windows
  /// @return true if operation was successful.
  virtual bool GetWowContext(WOW64_CONTEXT* context);
  virtual bool SetWowContext(const WOW64_CONTEXT& context);

  virtual char* GetIP();  /// Reads IP (instruction pointer).
  virtual void SetIP(char* ip);  /// Writes IP.

  /// Converts IP (instruction pointer) to flat process address. 
  /// @param ip instruction pointer
  /// @return flat address of instruction
  virtual char* IpToFlatAddress(char* ip) const;

  /// Handler of debug events. |dtc| is used to communicate decision
  /// to continue or halt to/from thread.
  /// @param de debug event
  /// @param dtc decision to continue or halt
  virtual void OnDebugEvent(DEBUG_EVENT& de, DecisionToContinue* dtc);

 private:
  virtual ExecutionEngineInsideObserver& inside_observer();
  virtual UserCommand GetLastUserCommand() const;

  /// Changes a 'Trace' flag in 
  virtual void EnableSingleStep(bool enable);

  virtual void OnOutputDebugString(DEBUG_EVENT& de, DecisionToContinue* dtc);
  virtual void OnExitThread(DEBUG_EVENT& de, DecisionToContinue* dtc);
  virtual void OnBreakpoint(DEBUG_EVENT& de, DecisionToContinue* dtc);
  virtual void OnSingleStep(DEBUG_EVENT& de, DecisionToContinue* dtc);
  
  virtual void ContinueFromHalted(bool single_step,
                                  bool pass_exception_to_debuggee);

  /// These functions implement actions attached to the states.
  virtual void EnterRunning();
  virtual void EnterHalted();
  virtual void EnterZombie();
  virtual void EnterDead();

  virtual void EmitNexeThreadStarted();
  virtual void EmitHalted();
  virtual void EmitRunning();

  int id_;
  HANDLE handle_;
  DebuggeeProcess& parent_process_;
  State state_;
  UserCommand last_user_command_;

  //Current breakpoint, if any.
  Breakpoint current_breakpoint_;
  
  // Stuff related only to nexe threads.
  bool is_nexe_;
  void* nexe_mem_base_;
  void* nexe_entry_point_;
};
}  // namespace debug
#endif  // NACL_SDK_DEBUG_CORE_DEBUGGEE_THREAD_H_
