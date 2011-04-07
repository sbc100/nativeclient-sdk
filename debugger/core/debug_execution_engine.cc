#include "debug_execution_engine.h"
#include "debug_core_inside_observer.h"
#include "debug_continue_policy.h"
#include "debuggee_process.h"
#include "debug_api.h"

//TODO: add unit tests
//TODO: add comments
//TODO: write a design doc with FSM, sequence diagrams, class diagram, obj diagram...
//TODO: make sure debug::core has enough api to implement RSP debug_server

#define REPORT_ERROR(str) inside_observer().OnError(str, __FILE__, __LINE__)

namespace debug {

ExecutionEngine::ExecutionEngine()
  : root_process_id_(0),
    event_observer_(NULL),
    continue_policy_(NULL),
    inside_observer_(NULL),
    debug_api_(NULL) {
}

ExecutionEngine::~ExecutionEngine() {
  Stop();
}

bool ExecutionEngine::StartProcess(const char* cmd, const char* work_dir) {
  Stop();

  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  char* cmd_dup = _strdup(cmd);
  if (NULL == cmd_dup) {
    REPORT_ERROR("Memory allocation error.");
    return false;
  }
  BOOL res = debug_api().CreateProcess(NULL, 
                             cmd_dup,
                             NULL,
                             NULL,
                             FALSE,
                             DEBUG_PROCESS | CREATE_NEW_CONSOLE,
                             NULL,
                             work_dir,
                             &si,
                             &pi);
  free(cmd_dup);
  if (!res) {
    REPORT_ERROR("::CreateProcess failed.");
    return false;
  }

  debug_api().CloseHandle(pi.hThread);
  debug_api().CloseHandle(pi.hProcess);
  root_process_id_ = pi.dwProcessId;
  return true;
}

bool ExecutionEngine::AttachToProcess(int id) {
  Stop();
  root_process_id_ = id;
  bool res = debug_api().DebugActiveProcess(id) ? true : false;
  if (!res)
    REPORT_ERROR("::DebugActiveProcess failed.");
  return res;
}

void ExecutionEngine::Detach() {
  std::deque<DebuggeeProcess*>::const_iterator it = processes_.begin();
  while (it != processes_.end()) {
    DebuggeeProcess* proc = *it;
    proc->Detach();
    delete proc;
    ++it;
  }
  processes_.clear();
  root_process_id_ = 0;
}

void ExecutionEngine::SetContinuePolicy(ContinuePolicy* continue_policy) {
  continue_policy_ = continue_policy;
}

void ExecutionEngine::SetEventObserver(ExecutionEngineEventObserver* observer) {
  event_observer_ = observer;
}

void ExecutionEngine::SetInsideObserver(ExecutionEngineInsideObserver* inside_observer) {
 inside_observer_ = inside_observer; 
 inside_observer_->SetExecutionEngine(this);
}

ExecutionEngineInsideObserver& ExecutionEngine::inside_observer() {
  static ExecutionEngineInsideObserver default_inside_observer;  // Observer that does nothing.
  if (NULL == inside_observer_)
    return default_inside_observer;
 return *inside_observer_;
}

ExecutionEngineEventObserver& ExecutionEngine::event_observer() {
  static ExecutionEngineEventObserver default_event_observer;  // Observer that does nothing.
  if (NULL == event_observer_)
    return default_event_observer;
 return *event_observer_;
}

void ExecutionEngine::SetDebugAPI(DebugAPI* debug_api) {
  debug_api_ = debug_api;
}

DebugAPI& ExecutionEngine::debug_api() {
  static DebugAPI default_debug_api;
  if (NULL == debug_api_)
    return default_debug_api;
  return *debug_api_;
}

DebuggeeProcess* ExecutionEngine::GetProcess(int id) {
  std::deque<DebuggeeProcess*>::const_iterator it = processes_.begin();
  while (it != processes_.end()) {
    DebuggeeProcess* proc = *it;
    ++it;
    if ((NULL != proc) && (id == proc->id()))
      return proc;
  }
  return NULL;
}

void ExecutionEngine::GetProcessesIds(std::deque<int>* processes) const {
  processes->clear();
  std::deque<DebuggeeProcess*>::const_iterator it = processes_.begin();
  while (it != processes_.end()) {
    DebuggeeProcess* proc = *it;
    ++it;
    if (NULL != proc)
      processes->push_back(proc->id());
  }
}

void ExecutionEngine::OnDebugEvent(DEBUG_EVENT& de) {
  DebuggeeProcess* process = GetProcess(de.dwProcessId);
  DecisionToContinue dtc;
  if (CREATE_PROCESS_DEBUG_EVENT == de.dwDebugEventCode) {
    process = new DebuggeeProcess(de.dwProcessId,
                                  de.u.CreateProcessInfo.hProcess,
                                  de.dwThreadId,
                                  de.u.CreateProcessInfo.hThread,
                                  de.u.CreateProcessInfo.hFile);
    if (NULL != process) {
      process->SetEventObserver(event_observer_);
      process->SetInsideObserver(inside_observer_);
      process->SetDebugAPI(debug_api_);
      processes_.push_back(process);
    }
  }

  inside_observer().OnDebugEvent(de);
  event_observer().OnDebugEvent(de);

  if (NULL != continue_policy_)
    continue_policy_->MakeContinueDecision(de, &dtc);

  if (NULL != process)
    process->OnDebugEvent(de, &dtc);

  inside_observer().OnDecisionToContinue(de.dwProcessId, de.dwThreadId, dtc);

  if (!dtc.IsHaltDecision()) {
    int flag = (dtc.pass_exception_to_debuggee() ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE);
    debug_api().ContinueDebugEvent(de.dwProcessId, de.dwThreadId, flag);
  }
}

void ExecutionEngine::DoWork(int wait_ms) {
  DEBUG_EVENT de;
  while (debug_api().WaitForDebugEvent(&de, 0))
    OnDebugEvent(de);

  if (debug_api().WaitForDebugEvent(&de, wait_ms))
    OnDebugEvent(de);
}

bool ExecutionEngine::HasAliveDebugee() const {
  std::deque<DebuggeeProcess*>::const_iterator it = processes_.begin();
  while (it != processes_.end()) {
    DebuggeeProcess* proc = *it;
    ++it;
    if ((NULL != proc) && (proc->GetState() != DebuggeeProcess::DEAD))
      return true;
  }
  return false;
}

void ExecutionEngine::Stop() {
  std::deque<DebuggeeProcess*>::const_iterator it = processes_.begin();
  while (it != processes_.end()) {
    DebuggeeProcess* proc = *it;
    ++it;
    if (NULL != proc)
      proc->Kill();
  }
  while (HasAliveDebugee())
    DoWork(20);

  while (processes_.size() > 0) {
    DebuggeeProcess* proc = processes_.front();
    processes_.pop_front();
    if (NULL != proc)
      delete proc;
  }
}
}  // namespace debug