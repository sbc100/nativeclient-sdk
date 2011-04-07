#ifndef NACL_SDK_DEBUG_CORE_DEBUGGEE_PROCESS_H_
#define NACL_SDK_DEBUG_CORE_DEBUGGEE_PROCESS_H_
#include <windows.h>
#include <deque>
#include <map>

namespace debug {
class Breakpoint;
class DebugAPI;
class ExecutionEngineEventObserver;
class ExecutionEngineInsideObserver;
class DebuggeeThread;
class DecisionToContinue;

class DebuggeeProcess {
 public:
  enum State {RUNNING, PAUSED, HIT_BREAKPOINT, DEAD};
  enum UserCommand {NONE, CONTINUE, STEP, BREAK, KILL};

  DebuggeeProcess(int process_id,
                  HANDLE handle,
                  int thread_id,
                  HANDLE thread_handle,
                  HANDLE file_handle);
  virtual ~DebuggeeProcess();
  virtual void SetEventObserver(ExecutionEngineEventObserver* observer);
  virtual void SetInsideObserver(ExecutionEngineInsideObserver* inside_observer);
  virtual void SetDebugAPI(DebugAPI* debug_api);

  virtual void Continue();
  virtual void ContinueAndPassExceptionToDebuggee();
  virtual void SingleStep();
  virtual void Break();
  virtual void Kill();
  virtual void Detach();

  virtual bool SetBreakpoint(void* addr);
  virtual void RemoveBreakpoint(void* addr);
  virtual Breakpoint* GetBreakpoint(void* addr);
  virtual int GetWordSizeInBits();
  virtual bool IsWow();

  virtual DebugAPI& debug_api();
  virtual ExecutionEngineEventObserver& event_observer();
  virtual ExecutionEngineInsideObserver& inside_observer();

  virtual int id() const {return id_;}
  virtual HANDLE Handle() const { return handle_; }
  virtual State GetState() const;
  virtual bool IsHalted() const;
  virtual bool HasNexeThread() const;

  virtual DebuggeeThread* GetThread(int id);
  virtual DebuggeeThread* GetHaltedThread();

  virtual int GetCurrentException() const;
  virtual int GetCurrentDebugEventCode() const;

  virtual void GetThreadIds(std::deque<int>* threads) const;
  virtual bool ReadMemory(const void* addr, size_t size, void* destination);
  virtual bool WriteMemory(const void* addr, const void* source, size_t size);
  
  virtual UserCommand last_user_command() const { return last_user_command_;}
  virtual void OnDebugEvent(DEBUG_EVENT& de, DecisionToContinue* dtc);

 protected:
  virtual void AddThread(int id, HANDLE handle);
  virtual void RemoveThread(int id);

  ExecutionEngineEventObserver* event_observer_;
  ExecutionEngineInsideObserver* inside_observer_;
  DebugAPI* debug_api_;
  
  State state_;
  int id_;
  HANDLE handle_;
  HANDLE file_handle_;
  std::deque<DebuggeeThread*> threads_;
  int current_exception_id_;
  int current_debug_event_code_;
  std::map<void*, Breakpoint> breakpoints_;
  DebuggeeThread* halted_thread_;
  UserCommand last_user_command_;
};
}  // namespace debug
#endif  // NACL_SDK_DEBUG_CORE_DEBUGGEE_PROCESS_H_
