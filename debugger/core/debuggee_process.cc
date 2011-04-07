#include "debuggee_process.h"
#include "debuggee_thread.h"
#include "debug_execution_engine.h"
#include "debug_execution_engine_inside_observer.h"
#include "debug_api.h"

#define REPORT_ERROR(str) inside_observer().OnError(str, __FILE__, __LINE__)

namespace debug {
DebuggeeProcess::DebuggeeProcess(int process_id,
                                 HANDLE handle,
                                 int thread_id,
                                 HANDLE thread_handle,
                                 HANDLE file_handle)
  : event_observer_(NULL),
    inside_observer_(NULL),
    debug_api_(NULL),
    state_(RUNNING),
    id_(process_id),
    handle_(handle),
    halted_thread_(NULL),
    file_handle_(file_handle),
    last_user_command_(NONE) {
    AddThread(thread_id, thread_handle);
}

DebuggeeProcess::~DebuggeeProcess() {
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    ++it;
    if (NULL != thread)
      delete thread;
  }  
  threads_.clear();
  debug_api().CloseHandle(file_handle_);
}

void DebuggeeProcess::SetEventObserver(ExecutionEngineEventObserver* observer) {
  event_observer_ = observer;
}

void DebuggeeProcess::SetInsideObserver(ExecutionEngineInsideObserver* inside_observer) {
  inside_observer_ = inside_observer;
}

void DebuggeeProcess::SetDebugAPI(DebugAPI* debug_api) {
  debug_api_ = debug_api;
}


DebugAPI& DebuggeeProcess::debug_api() {
  static DebugAPI default_debug_api;
  if (NULL == debug_api_)
    return default_debug_api;
  return *debug_api_;
}

ExecutionEngineEventObserver& DebuggeeProcess::event_observer() {
  static ExecutionEngineEventObserver default_event_observer;  // Observer that does nothing.
  if (NULL == event_observer_)
    return default_event_observer;
 return *event_observer_;
}

ExecutionEngineInsideObserver& DebuggeeProcess::inside_observer() {
  static ExecutionEngineInsideObserver default_inside_observer;  // Observer that does nothing.
  if (NULL == inside_observer_)
    return default_inside_observer;
 return *inside_observer_;
}

bool DebuggeeProcess::IsWow() {
  BOOL is_wow = FALSE;
  if (!debug_api().IsWow64Process(handle_, &is_wow))
    return false;
  return is_wow ? true : false;
}

int DebuggeeProcess::GetWordSizeInBits() {
  if (IsWow())
    return 32;
#ifdef _WIN64
  return 64;
#else
  return 32;
#endif
  return 0;
}

void DebuggeeProcess::Detach() {
  debug_api().DebugActiveProcessStop(id());
}

void DebuggeeProcess::Kill() {
  last_user_command_ = KILL;
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    ++it;
    if (NULL != thread)
      thread->Kill();
  }
  Continue();  //TODO: test it
}

DebuggeeProcess::State DebuggeeProcess::GetState() const {
  return state_;
}

void DebuggeeProcess::AddThread(int id, HANDLE handle) {
  if (NULL != GetThread(id))
    return;
  DebuggeeThread* thread = new DebuggeeThread(id, handle, this);
  threads_.push_back(thread);
}

DebuggeeThread* DebuggeeProcess::GetThread(int id) {
  std::deque<DebuggeeThread*>::iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (thread->id() == id)
      return thread;
    ++it;
  }
  return NULL;
}

DebuggeeThread* DebuggeeProcess::GetHaltedThread() {
  return halted_thread_;
}

int DebuggeeProcess::GetCurrentException() const {
  return current_exception_id_;
}

int DebuggeeProcess::GetCurrentDebugEventCode() const {
  return current_debug_event_code_;
}

void DebuggeeProcess::Break() {
  last_user_command_ = BREAK;
  if (!debug_api().DebugBreakProcess(handle_)) {
    REPORT_ERROR("::DebugBreakProcess failed.");
  }
}

void DebuggeeProcess::GetThreadIds(std::deque<int>* threads) const {
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it++;
    threads->push_back(thread->id());
  }
}

void DebuggeeProcess::RemoveThread(int id) {
  std::deque<DebuggeeThread*>::iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (thread->id() == id) {
      threads_.erase(it);
      if (halted_thread_ == thread) {
        halted_thread_ = NULL;
      }
      delete thread;
      break;
    }
    ++it;
  }
}

void DebuggeeProcess::SingleStep() {
  last_user_command_ = STEP;
  if (NULL != halted_thread_)
    halted_thread_->SingleStep();
}

bool DebuggeeProcess::ReadMemory(const void* addr,
                                 size_t size,
                                 void* destination) {
  SIZE_T sz = 0;
  if (!debug_api().ReadProcessMemory(handle_, addr, destination, size, &sz)) {
    REPORT_ERROR("::ReadProcessMemory failed.");
    return false;
  }
  return true;
}

bool DebuggeeProcess::WriteMemory(const void* addr,
                                  const void* source,
                                  size_t size) {
  SIZE_T wr = 0;
  BOOL res = debug_api().WriteProcessMemory(handle_,
                                            const_cast<void*>(addr),
                                            source,
                                            size,
                                            &wr);
  if (!res) {
    REPORT_ERROR("::WriteProcessMemory failed.");
    return false;
  }
  res = debug_api().FlushInstructionCache(handle_, addr, size);
  if (!res) {
    REPORT_ERROR("::FlushInstructionCache failed.");
    return false;
  }
  return true;
}

void DebuggeeProcess::OnDebugEvent(DEBUG_EVENT& de, DecisionToContinue* dtc) {
  switch (de.dwDebugEventCode) {
    case CREATE_THREAD_DEBUG_EVENT: {
      AddThread(de.dwThreadId, de.u.CreateThread.hThread);
      break;
    }
    case EXIT_PROCESS_DEBUG_EVENT: state_ = DEAD; break;
  }
  
  DebuggeeThread* thread = GetThread(de.dwThreadId);
  if (NULL != thread) {
    thread->OnDebugEvent(de, dtc);
    if (thread->state() == DebuggeeThread::DEAD) {
      RemoveThread(de.dwThreadId);
    } else if (thread->IsHalted()) {
      halted_thread_ = thread;
    }
  }
}

void DebuggeeProcess::ContinueAndPassExceptionToDebuggee() {
  last_user_command_ = CONTINUE;
  if (NULL != halted_thread_) {
    halted_thread_->ContinueAndPassExceptionToDebuggee();
    halted_thread_ = NULL;
  }
}

void DebuggeeProcess::Continue() {
  last_user_command_ = CONTINUE;
  if (NULL != halted_thread_) {
    halted_thread_->Continue();
    halted_thread_ = NULL;
  }
}

bool DebuggeeProcess::IsHalted() const {
  return (NULL != halted_thread_);
}

bool DebuggeeProcess::HasNexeThread() const {
  std::deque<DebuggeeThread*>::const_iterator it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it++;
    if (thread->is_nexe())
      return true;
  }
  return false;
}

bool DebuggeeProcess::SetBreakpoint(void* addr) {
  RemoveBreakpoint(addr);
  Breakpoint br(addr);
  if (br.Init(this)) {
    breakpoints_[addr] = br;
    return true;
  } 
  return false;
}

Breakpoint* DebuggeeProcess::GetBreakpoint(void* addr) {
  std::map<void*, Breakpoint>::iterator it = breakpoints_.find(addr);
  if (breakpoints_.end() == it)
    return NULL;
  return &it->second;
}

void DebuggeeProcess::RemoveBreakpoint(void* addr) {
  Breakpoint br = breakpoints_[addr];
  if (br.is_valid()) {
    br.RecoverCodeAtBreakpoint(this);
    breakpoints_.erase(breakpoints_.find(addr));
  }
}
}  // namespace debug
