#pragma once
#include <windows.h>
#include "debuggee_thread.h"

namespace debug {
class ExecutionEngine;
class DecisionToContinue;

class ExecutionEngineInsideObserver {
 public:
  ExecutionEngineInsideObserver() : execution_engine_(NULL) {}
  virtual ~ExecutionEngineInsideObserver() {}
  void SetExecutionEngine(ExecutionEngine* execution_engine) { execution_engine_ = execution_engine; }
  
  virtual void OnError(const std::string& error, const std::string& src_file, int src_line) const {}
  virtual void OnDebugEvent(DEBUG_EVENT& de) const {}
  virtual void OnDecisionToContinue(int process_id, int thread_id, DecisionToContinue& dtc) const {}
  virtual void OnBreakpointHit(DebuggeeThread* thread, void* addr, bool our) const {}
  virtual void OnThreadStateChange(DebuggeeThread* thread, DebuggeeThread::State old_state, DebuggeeThread::State new_state) const {}
  virtual void OnThreadSetSingleStep(DebuggeeThread* thread, bool enable) const {}
  virtual void OnThreadOutputDebugString(DebuggeeThread* thread, const char* str) const {}
  virtual void OnNexeThreadStarted(DebuggeeThread* thread) const {}
  virtual void OnWriteBreakpointCode(int process_id, void* addr, bool result) const {}
  virtual void OnRecoverCodeAtBreakpoint(int process_id, void* addr, char orig_code, bool result) const {}
  virtual void OnSingleStepDueToContinueFromBreakpoint(DebuggeeThread* thread, void* breakpoint_addr) const {}
  virtual void OnBreakinBreakpoint(DebuggeeThread* thread) const {}

 protected:
  ExecutionEngine* execution_engine_;
};
}  // namespace debug