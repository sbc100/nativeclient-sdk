#include "debugger/core/debug_execution_engine.h"
#include "debugger/core/debug_logger.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debug_api.h"

//TODO: add unit tests
//TODO: add comments
//TODO: write a design doc with FSM, sequence diagrams, class diagram, obj diagram...
//TODO: make sure debug::core has enough api to implement RSP debug_server

namespace debug {

ExecutionEngine::ExecutionEngine(DebugAPI& debug_api)
  : debug_api_(debug_api) {
}

ExecutionEngine::~ExecutionEngine() {
  Stop();
}

DebuggeeProcess* ExecutionEngine::CreateDebuggeeProcess(int id,
                                                        HANDLE handle,
                                                        HANDLE file_handle,
                                                        DebugAPI& debug_api) {
  return new DebuggeeProcess(id, handle, file_handle, debug_api);
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
    DBG_LOG("TR01.00", "Memory allocation error.");
    return false;
  }
  BOOL res = debug_api_.CreateProcess(NULL, 
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
  if (!res)
    return false;

  debug_api_.CloseHandle(pi.hThread);
  debug_api_.CloseHandle(pi.hProcess);
  return true;
}

bool ExecutionEngine::AttachToProcess(int id) {
  Stop();
  return (TRUE == debug_api_.DebugActiveProcess(id)) ? true : false;
}

void ExecutionEngine::DetachAll() {
  std::deque<DebuggeeProcess*>::const_iterator it = processes_.begin();
  while (it != processes_.end()) {
    DebuggeeProcess* proc = *it;
    proc->Detach();
    delete proc;
    ++it;
  }
  processes_.clear();
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

void ExecutionEngine::RemoveDeadProcess() {
  //remove dead process, if any.
  std::deque<DebuggeeProcess*>::const_iterator it = processes_.begin();
  while (it != processes_.end()) {
    DebuggeeProcess* proc = *it;
    if ((proc->state() == DebuggeeProcess::kDead)) {
      delete proc;
      processes_.erase(it);
      break;
    }
    ++it;
  }
}

void ExecutionEngine::OnDebugEvent(const DEBUG_EVENT& debug_event, DebuggeeProcess** halted_process) {
  RemoveDeadProcess();

  debug_event_.windows_debug_event_ = debug_event;
  debug_event_.nacl_debug_event_code_ = DebugEvent::kNotNaClDebugEvent;

  DebuggeeProcess* process = GetProcess(debug_event.dwProcessId);

  if (CREATE_PROCESS_DEBUG_EVENT == debug_event.dwDebugEventCode) {
    process = CreateDebuggeeProcess(debug_event.dwProcessId,
                                    debug_event.u.CreateProcessInfo.hProcess,
                                    debug_event.u.CreateProcessInfo.hFile,
                                    debug_api_);
    if (NULL != process)
      processes_.push_back(process);
  }

  char tmp[1000];
  debug_event_.ToString(tmp, sizeof(tmp));
  DBG_LOG("TR01.01", "msg='ExecutionEngine::OnDebugEvent' event='%s'", tmp);

  if (NULL != process)
    process->OnDebugEvent(&debug_event_);

  if (NULL != halted_process) {
    *halted_process = NULL;
    if ((NULL != process) && (DebuggeeProcess::kHalted == process->state()))
      *halted_process = process;
  }
}

bool ExecutionEngine::DoWork(int wait_ms, DebuggeeProcess** halted_process) {
  if (NULL != halted_process)
    *halted_process = NULL;

  DEBUG_EVENT de;
  if (debug_api_.WaitForDebugEvent(&de, wait_ms)) {
    OnDebugEvent(de, halted_process);
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
  DebuggeeProcess* halted_proc = NULL;
  while (processes_.size() > 0) {
    DoWork(20, &halted_proc);
    if (NULL != halted_proc) {
      halted_proc->ContinueAndPassExceptionToDebuggee();
      RemoveDeadProcess();
    }
  }
}
}  // namespace debug
