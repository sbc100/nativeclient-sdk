#include "debuggee_thread.h"
#include "debuggee_process.h"
#include "debug_api.h"
#include "debug_continue_policy.h"
#include "debug_execution_engine.h"
#include "debug_execution_engine_inside_observer.h"

#pragma warning(disable : 4996)  // Disable sscanf warning.

namespace {
const size_t kMaxStringSize = 32 * 1024;
const char* kNexeUuid =
    "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11} -v 1 -event NaClThreadStart";
}

#define REPORT_ERROR(str) inside_observer().OnError(str, __FILE__, __LINE__)

namespace debug {
DebuggeeThread::DebuggeeThread(int id,
                               HANDLE handle,
                               DebuggeeProcess* parent_process)
    : id_(id),
      handle_(handle),
      parent_process_(*parent_process),
      state_(RUNNING),
      last_user_command_(NONE),
      is_nexe_(false),
      nexe_mem_base_(NULL),
      nexe_entry_point_(NULL) {
  EnterRunning();
}

DebuggeeThread::~DebuggeeThread() {
}

ExecutionEngineInsideObserver& DebuggeeThread::inside_observer() {
  return process().inside_observer();
}

DebugAPI& DebuggeeThread::debug_api() {
  return process().debug_api();
}

ExecutionEngineEventObserver& DebuggeeThread::event_observer() {
  return process().event_observer();
}

DebuggeeProcess& DebuggeeThread::process() {
  return parent_process_;
}

const DebuggeeProcess& DebuggeeThread::process() const {
  return parent_process_;
}

int DebuggeeThread::id() const {
  return id_;
}

DebuggeeThread::State DebuggeeThread::state() const {
  return state_;
}

bool DebuggeeThread::IsHalted() const {
  return (state_ == HALTED);
}

DebuggeeThread::UserCommand DebuggeeThread::GetLastUserCommand() const {
  return last_user_command_;
}

bool DebuggeeThread::is_nexe() const {
  return is_nexe_;
}

void* DebuggeeThread::nexe_mem_base() const {
  return nexe_mem_base_;
}

void* DebuggeeThread::nexe_entry_point() const {
  return nexe_entry_point_;
}

void DebuggeeThread::EmitNexeThreadStarted() {
  inside_observer().OnNexeThreadStarted(this);
  event_observer().OnNexeThreadStarted(*this);
}

void DebuggeeThread::EmitRunning() {
  event_observer().OnThreadRunning(*this);
}

void DebuggeeThread::EmitHalted() {
  event_observer().OnThreadHalted(*this);
}

char* DebuggeeThread::IpToFlatAddress(char* ip) const {
#ifndef _WIN64
  if (is_nexe_)   
    ip += reinterpret_cast<size_t>(nexe_mem_base_);
#endif
  return ip;
}

void DebuggeeThread::OnOutputDebugString(DEBUG_EVENT& de, DecisionToContinue* dtc) {
  if (0 == de.u.DebugString.fUnicode) {
    size_t str_sz = min(kMaxStringSize, de.u.DebugString.nDebugStringLength + 1);
    char* tmp = static_cast<char*>(malloc(str_sz));
    if (NULL != tmp) {
      if (parent_process_.ReadMemory(de.u.DebugString.lpDebugStringData, str_sz, tmp)) {
        tmp[str_sz - 1] = 0;
        inside_observer().OnThreadOutputDebugString(this, tmp);

        if (strncmp(tmp, kNexeUuid, strlen(kNexeUuid)) == 0) {
          is_nexe_ = true;
          void* tread_inf_addr = 0;
          sscanf(tmp + strlen(kNexeUuid), " -mb %p -ep %p", &nexe_mem_base_, &nexe_entry_point_);
          EmitNexeThreadStarted();
          dtc->Combine(DecisionToContinue(DecisionToContinue::STRONG_DECISION, DecisionToContinue::HALT_DEBUGGEE));
          if (dtc->IsHaltDecision())
            EnterHalted();
          free(tmp);
          return;
        }
      }
      free(tmp);
    }
  }
  if (dtc->IsHaltDecision())
    EnterHalted();
}

void DebuggeeThread::OnExitThread(DEBUG_EVENT& de, DecisionToContinue* dtc) {
  if (dtc->IsHaltDecision())
    EnterZombie();
  else
    EnterDead();
}

void DebuggeeThread::EnterRunning() {
  inside_observer().OnThreadStateChange(this, state_, RUNNING);
  state_ = RUNNING;
  EmitRunning();
}

void DebuggeeThread::EnterHalted() {
  inside_observer().OnThreadStateChange(this, state_, HALTED);
  state_ = HALTED;
  EnableSingleStep(false);
  EmitHalted();
}

void DebuggeeThread::EnterZombie() {
  inside_observer().OnThreadStateChange(this, state_, ZOMBIE);
  state_ = ZOMBIE;
  EmitHalted();
}

void DebuggeeThread::EnterDead() {
  inside_observer().OnThreadStateChange(this, state_, DEAD);
  state_ = DEAD;
}

void DebuggeeThread::OnBreakpoint(DEBUG_EVENT& de, DecisionToContinue* dtc) {
  char* br_addr = IpToFlatAddress(
      static_cast<char*>(de.u.Exception.ExceptionRecord.ExceptionAddress));

  Breakpoint* br = parent_process_.GetBreakpoint(br_addr);
  inside_observer().OnBreakpointHit(this, br_addr, (NULL != br));

  if (NULL != br) {
    // it's our breakpoint.
    current_breakpoint_ = *br;
    current_breakpoint_.RecoverCodeAtBreakpoint(&process());
    SetIP(GetIP() - 1);
    *dtc = DecisionToContinue(DecisionToContinue::STRONG_DECISION, DecisionToContinue::HALT_DEBUGGEE);
  } else if (DebuggeeProcess::BREAK == process().last_user_command()) {
    inside_observer().OnBreakinBreakpoint(this);
     *dtc = DecisionToContinue(DecisionToContinue::STRONG_DECISION, DecisionToContinue::HALT_DEBUGGEE);
  }
  if (dtc->IsHaltDecision())
    EnterHalted();
}

void DebuggeeThread::ContinueAndPassExceptionToDebuggee() {
  if ((HALTED == state_) || (ZOMBIE == state_)) {
    last_user_command_ = CONTINUE;
    ContinueFromHalted(false, true);
  }
}

void DebuggeeThread::Continue() {
  if ((HALTED == state_) || (ZOMBIE == state_)) {
    last_user_command_ = CONTINUE;
    ContinueFromHalted(false, false);
  }
}

void DebuggeeThread::SingleStep() {
  if ((HALTED == state_) || (ZOMBIE == state_)) {
    last_user_command_ = STEP;
    ContinueFromHalted(true, false);
  }
}

void DebuggeeThread::ContinueFromHalted(bool single_step, bool pass_exception_to_debuggee) {
  if (current_breakpoint_.is_valid()) {
    // Check that breakpoint is not deleted
    if (NULL == process().GetBreakpoint(current_breakpoint_.address())) {
      current_breakpoint_.Invalidate();
    } else {
      char* flat_ip = IpToFlatAddress(GetIP());
      if (flat_ip == current_breakpoint_.address()) {
        EnableSingleStep(true);
      } else {
        current_breakpoint_.WriteBreakpointCode(&process());
        current_breakpoint_.Invalidate();
      }
    }
  }
  if (single_step)
    EnableSingleStep(true);
  
  // pass_exception_to_debuggee_ is set by last decision to halt
  int flag = (pass_exception_to_debuggee ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE);
  debug_api().ContinueDebugEvent(parent_process_.id(), id(), flag);

  if (ZOMBIE == state_)
    EnterDead();
  else
    EnterRunning();
}

void DebuggeeThread::OnSingleStep(DEBUG_EVENT& de, DecisionToContinue* dtc) {
  if (STEP == last_user_command_) {
    *dtc = DecisionToContinue(DecisionToContinue::STRONG_DECISION, DecisionToContinue::HALT_DEBUGGEE);
  } else {  // It was 'continue'.
    inside_observer().OnSingleStepDueToContinueFromBreakpoint(this, current_breakpoint_.address());
    *dtc = DecisionToContinue(DecisionToContinue::STRONG_DECISION, DecisionToContinue::DONT_HALT_DEBUGGEE, DecisionToContinue::DONT_PASS_EXCEPTION_TO_DEBUGGEE);
  }

  if (current_breakpoint_.is_valid()) {
    current_breakpoint_.WriteBreakpointCode(&process());  // recover breakpoint.
    current_breakpoint_.Invalidate();
  }

  if (dtc->IsHaltDecision())
    EnterHalted();
}

void DebuggeeThread::OnDebugEvent(DEBUG_EVENT& de, DecisionToContinue* dtc) {
  switch (de.dwDebugEventCode) {
    case OUTPUT_DEBUG_STRING_EVENT: OnOutputDebugString(de, dtc); break;
    case EXIT_THREAD_DEBUG_EVENT: OnExitThread(de, dtc); break;
    case EXCEPTION_DEBUG_EVENT: {
      switch (de.u.Exception.ExceptionRecord.ExceptionCode) {
        case EXCEPTION_BREAKPOINT: OnBreakpoint(de, dtc); break;
        case EXCEPTION_SINGLE_STEP: OnSingleStep(de, dtc); break;
      }
    }
  }
  if (dtc->IsHaltDecision() && (RUNNING == state_))
    EnterHalted();
}

void DebuggeeThread::EnableSingleStep(bool enable) {
  inside_observer().OnThreadSetSingleStep(this, enable);
  if (process().IsWow()) {
    WOW64_CONTEXT context;
    GetWowContext(&context);
    if (enable)
      context.EFlags |= 1 << 8;
    else
      context.EFlags &= ~(1 << 8);
    SetWowContext(context);
  } else {
    CONTEXT context;
    GetContext(&context);
    if (enable)
      context.EFlags |= 1 << 8;
    else
      context.EFlags &= ~(1 << 8);
    SetContext(context);
  }
}

const char* DebuggeeThread::GetStateName(State state) {
  switch (state) {
    case RUNNING: return "RUNNING";
    case HALTED: return "HALTED";
    case ZOMBIE: return "ZOMBIE";
    case DEAD: return "DEAD";
  }
  return "N/A";
}

void DebuggeeThread::Kill() {
 if (NULL != handle_) 
  debug_api().TerminateThread(handle_, 0);
}

bool DebuggeeThread::GetContext(CONTEXT* context) {
  context->ContextFlags = CONTEXT_ALL;
  bool res = (debug_api().GetThreadContext(handle_, context) != FALSE);
  if (!res)
    REPORT_ERROR("::GetThreadContext failed.");
  return res;
}

bool DebuggeeThread::SetContext(const CONTEXT& context) {
  CONTEXT context_copy = context;
  bool res = (debug_api().SetThreadContext(handle_, &context_copy) != FALSE);
  if (!res)
    REPORT_ERROR("::SetThreadContext failed");
  return res;
}

bool DebuggeeThread::GetWowContext(WOW64_CONTEXT* context) {
  context->ContextFlags = CONTEXT_ALL;
  bool res = (debug_api().Wow64GetThreadContext(handle_, context) != FALSE);
  if (!res)
    REPORT_ERROR("::Wow64GetThreadContext failed.");
  return res;
}

bool DebuggeeThread::SetWowContext(const WOW64_CONTEXT& context) {
  WOW64_CONTEXT context_copy = context;
  bool res = (debug_api().Wow64SetThreadContext(handle_, &context_copy) != FALSE);
  if (!res)
    REPORT_ERROR("::Wow64SetThreadContext failed");
  return res;
}

char* DebuggeeThread::GetIP() {
  if (process().IsWow()) {
    WOW64_CONTEXT context;
    GetWowContext(&context);
    return reinterpret_cast<char*>(context.Eip);
  } else {
    CONTEXT context;
    GetContext(&context);
#ifdef _WIN64
    return reinterpret_cast<char*>(context.Rip);
#else
    return reinterpret_cast<char*>(context.Eip);
#endif
  }
}

void DebuggeeThread::SetIP(char* ip) {
  if (process().IsWow()) {
    WOW64_CONTEXT context;
    GetWowContext(&context);
    context.Eip = reinterpret_cast<DWORD>(ip);
    SetWowContext(context);
  } else {
    CONTEXT ct;
    GetContext(&ct);
#ifdef _WIN64
    ct.Rip = reinterpret_cast<DWORD64>(ip);
#else
    ct.Eip = reinterpret_cast<DWORD>(ip);
#endif
    SetContext(ct);
  }
}
}  // namespace debug