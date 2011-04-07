#pragma once
#include <string>
#include <deque>
#include <map>
#include <windows.h>

namespace debug {
class DebuggeeThread;
class DebuggeeProcess;
class ExecutionEngineEventObserver;
class ContinuePolicy;
class ExecutionEngineInsideObserver;
class DebugAPI;

class ExecutionEngine
{
public:
  ExecutionEngine();
  virtual ~ExecutionEngine();

  virtual void SetContinuePolicy(ContinuePolicy* continue_policy);
  virtual void SetEventObserver(ExecutionEngineEventObserver* observer);
  virtual void SetInsideObserver(ExecutionEngineInsideObserver* inside_observer);
  virtual void SetDebugAPI(DebugAPI* debug_api);
  virtual ExecutionEngineInsideObserver& inside_observer();
  virtual ExecutionEngineEventObserver& event_observer();
  virtual DebugAPI& debug_api();
  
  virtual bool StartProcess(const char* cmd, const char* work_dir);
  virtual bool AttachToProcess(int id);
  virtual void Detach();  //TODO: -> DetachAll + Detach(process_id)
  virtual void DoWork(int wait_ms);
  virtual void Stop();  // Blocks until all debugee processes terminated.

  virtual DebuggeeProcess* GetProcess(int id);
  virtual void GetProcessesIds(std::deque<int>* processes) const;

 protected:
  virtual void OnDebugEvent(DEBUG_EVENT& de);
  virtual bool HasAliveDebugee() const;

  std::deque<DebuggeeProcess*> processes_;
  int root_process_id_;
  ExecutionEngineEventObserver* event_observer_;
  ContinuePolicy* continue_policy_;
  ExecutionEngineInsideObserver* inside_observer_;
  DebugAPI* debug_api_;
};

class ExecutionEngineEventObserver {
 public:
  virtual ~ExecutionEngineEventObserver() {}

  virtual void OnDebugEvent(DEBUG_EVENT& de) {}
  virtual void OnNexeThreadStarted(DebuggeeThread& thread) {}
  virtual void OnThreadHalted(DebuggeeThread& thread) {}
  virtual void OnThreadRunning(DebuggeeThread& thread) {}
};
}  // namespace debug