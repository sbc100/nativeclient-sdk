// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_thread.h"
#include <assert.h>
#include "debugger/base/debug_command_line.h"
#include "debugger/core/debug_api.h"
#include "debugger/core/debug_breakpoint.h"
#include "debugger/core/debug_event.h"
#include "debugger/core/debug_logger.h"
#include "debugger/core/debuggee_iprocess.h"

#pragma warning(disable : 4996)  // Disable sscanf warning.

namespace {
const size_t kMaxStringSize = 32 * 1024;
const char* kNexeUuid = "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11}";
}

namespace debug {
DebuggeeThread::DebuggeeThread(int id,
                               HANDLE handle,
                               IDebuggeeProcess* parent_process)
    : id_(id),
      handle_(handle),
      parent_process_(*parent_process),
      state_(kHalted),
      exit_code_(0),
      triggered_breakpoint_addr_(NULL),
      is_nacl_app_thread_(false) {
  assert(NULL != parent_process);
}

DebugAPI& DebuggeeThread::debug_api() {
  return parent_process().debug_api();
}

void DebuggeeThread::SetState(State new_state) {
  DBG_LOG("TR03.01",
          "msg='DebuggeeThread::SetState' thread_id=%d old_state=%s"
          " new_state=%s",
          id(),
          GetStateName(state_),
          GetStateName(new_state));
  state_ = new_state;
}

bool DebuggeeThread::IsHalted() const {
  return (kHalted == state_);
}

void DebuggeeThread::OnOutputDebugString(DebugEvent* debug_event) {
  DEBUG_EVENT de = debug_event->windows_debug_event();
  if (0 == debug_event->windows_debug_event().u.DebugString.fUnicode) {
    size_t sz = de.u.DebugString.nDebugStringLength + 1;
    size_t str_sz = min(kMaxStringSize, sz);
    char* tmp = static_cast<char*>(malloc(str_sz));
    if (NULL != tmp) {
      if (parent_process().ReadMemory(
          de.u.DebugString.lpDebugStringData,
          str_sz,
          tmp)) {
        tmp[str_sz - 1] = 0;

        DBG_LOG("TR03.02", "OutputDebugString=%s", tmp);

        if (strncmp(tmp, kNexeUuid, strlen(kNexeUuid)) == 0) {
          /// This string is coming from sel_ldr.
          is_nacl_app_thread_ = true;
          // Here we are passing pointers from sel_ldr to the debugger.
          // One might think that we can't use them because they are
          // valid in sel_ldr address space only. This is not true.
          // ::ReadProcessMemory can be used with these pointers.
          // Note: these pointers are not untrusted NaCl, they are
          // native windows flat pointers.

          // Here's list of possible messages from sel_ldr:
          // "-version 1 -event AppCreate -nap %p -mem_start %p -user_entry_pt
          //     %p -initial_entry_pt %p"
          // "-version 1 -event ThreadCreate -natp %p"
          // "-version 1 -event ThreadExit -natp %p -exit_code %d"
          // "-version 1 -event AppExit -exit_code %d"
          CommandLine cmd_line(tmp);
          std::string event = cmd_line.GetStringSwitch("-event", "");
          if ("AppCreate" == event) {
            void* nexe_mem_base = cmd_line.GetAddrSwitch("-mem_start");
            void* user_entry_point = cmd_line.GetAddrSwitch("-user_entry_pt");
            parent_process().set_nexe_mem_base(nexe_mem_base);
            parent_process().set_nexe_entry_point(user_entry_point);
            DBG_LOG("TR03.03",
                    "NaClAppCreate mem_base=%p entry_point=%p",
                    nexe_mem_base,
                    user_entry_point);
          } else if ("ThreadCreate" == event) {
            debug_event->set_nacl_debug_event_code(
                DebugEvent::kThreadIsAboutToStart);
            DBG_LOG("TR03.07",
                    "ThreadCreate thread_id=%d",
                    id());
          }
        }
      }
      free(tmp);
    }
  }
}

// Here's what we have to do when we hit our breakpoint:
// a) recover original code at breakpoint adddress.
// b) return IP to the beginning of instruction (simple decrement work).
void DebuggeeThread::OnBreakpoint(DebugEvent* debug_event) {
  DEBUG_EVENT de = debug_event->windows_debug_event();
  void* ex_addr = de.u.Exception.ExceptionRecord.ExceptionAddress;
  void* br_addr = ex_addr;
  if (IsNaClAppThread())
    br_addr = parent_process().FromNexeToFlatAddress(ex_addr);

  Breakpoint* br = parent_process().GetBreakpoint(br_addr);

  DBG_LOG("TR03.04",
          "msg='DebuggeeThread::OnBreakpoint' thread_id=%d ex_addr=%p"
          " br_addr=0x%p our=%s",
          id(),
          ex_addr,
          br_addr,
          (NULL != br) ? "yes" : "no");

  if (NULL != br) {
    // it's our breakpoint.
    triggered_breakpoint_addr_ = br->address();
    br->RecoverCodeAtBreakpoint();
    SetIP(reinterpret_cast<char*>(GetIP()) - 1);
  }
}

bool DebuggeeThread::Continue(ContinueOption option) {
  if (!IsHalted()) {
    DBG_LOG("WARN03.01",
            "msg='DebuggeeThread::Continue(%s) called while thread was in an"
            " incompatible state: %s' thread_id=%d",
            GetContinueOptionName(option),
            GetStateName(state_),
            id());
    return false;
  }
  const DebugEvent& last_debug_event = parent_process().last_debug_event();
  int last_debug_event_id =
      last_debug_event.windows_debug_event().dwDebugEventCode;
  if ((EXIT_THREAD_DEBUG_EVENT == last_debug_event_id) ||
     (EXIT_PROCESS_DEBUG_EVENT == last_debug_event_id)) {
    SetState(kDead);
    return (TRUE == debug_api().ContinueDebugEvent(parent_process().id(),
                                                   id(),
                                                   DBG_CONTINUE));
  }

  if (NULL != triggered_breakpoint_addr_) {
    void* flat_ip = GetIP();
    if (IsNaClAppThread())
      flat_ip = parent_process().FromNexeToFlatAddress(GetIP());

    if (flat_ip == triggered_breakpoint_addr_) {
      return ContinueFromBreakpoint();
    } else {
      // Just in case user changed IP so that it's not pointing to
      // triggered breakpoint, we need to:
      // a) recover breakpoint
      // b) make it not triggered
      Breakpoint* br =
          parent_process().GetBreakpoint(triggered_breakpoint_addr_);
      if (NULL != br)
        br->WriteBreakpointCode();
      triggered_breakpoint_addr_ = NULL;
    }
  }
  if (kSingleStep == option)
    EnableSingleStep(true);

  int flags = DBG_CONTINUE;
  if (kContinueAndPassException == option)
    flags = DBG_EXCEPTION_NOT_HANDLED;

  SetState(kRunning);
  return (TRUE == debug_api().ContinueDebugEvent(parent_process().id(),
                                                 id(),
                                                 flags));
}

/// Implements first steps of 'Continue from breakpoint' algorithm,
/// described in |DebuggeeThread::OnDebugEvent| method.
bool DebuggeeThread::ContinueFromBreakpoint() {
  SetState(kContinueFromBreakpoint);
  EnableSingleStep(true);
  return (TRUE == debug_api().ContinueDebugEvent(parent_process().id(),
                                                 id(),
                                                 DBG_CONTINUE));
}

void DebuggeeThread::OnSingleStep(DebugEvent* debug_event) {
  if (kContinueFromBreakpoint == state_) {
    if (NULL == triggered_breakpoint_addr_) {
      DBG_LOG("ERR03.01",
            "msg=OnSingleStepDueToContinueFromBreakpoint thread_id=%d"
            " triggered_breakpoint_=NULL",
            id());
      SetState(kHalted);
    } else {
      DBG_LOG("TR03.05",
              "msg=OnSingleStepDueToContinueFromBreakpoint thread_id=%d"
              " breakpoint_addr=0x%p",
              id(),
              triggered_breakpoint_addr_);
      // We got here because thread was continuing from breakpoint.
      Breakpoint* br =
          parent_process().GetBreakpoint(triggered_breakpoint_addr_);
      if (NULL != br)
        br->WriteBreakpointCode();
      triggered_breakpoint_addr_ = NULL;
      debug_api().ContinueDebugEvent(parent_process().id(),
                                     id(),
                                     DBG_CONTINUE);
      SetState(kRunning);
    }
  } else {
    SetState(kHalted);
  }
}

void DebuggeeThread::OnDebugEvent(DebugEvent* debug_event) {
  DEBUG_EVENT de = debug_event->windows_debug_event();
  EnableSingleStep(false);

  // Thread expected 'SingleStep' exception due to 'continue from breakpoint'
  // algorithm, but got something else. For example, it happens when breakpoint
  // is set to instruction generating 'memory access violation' exception.
  //
  // Here's description of 'continue from breakpoint' algorithm:
  // a) recover original code at breakpoint address
  // b) enable single step mode of CPU
  // c) call DebugAPI::ContinueDebugEvent
  // d) receive SingleStep exception
  // e) write breakpoint code at breakpoint address
  // f) disable single step mode of CPU
  // h) call DebugAPI::ContinueDebugEvent
  // f) DebuggeeThread is now in 'kRunning' state.
  // Note: we can get different exception on step (d),
  // we have to do the following:
  // 1) disable single step mode of CPU (it's done in first statement in
  //    |DebuggeeThread::OnDebugEvent| method).
  // 2) write breakpoint code at breakpoint address, if IP points
  //    somewhere else.
  if (!debug_event->IsSingleStep() &&
      (kContinueFromBreakpoint == state_) &&
      (NULL != triggered_breakpoint_addr_)) {
    void* flat_ip = GetIP();
    if (IsNaClAppThread())
      flat_ip = parent_process().FromNexeToFlatAddress(flat_ip);

    if (flat_ip != triggered_breakpoint_addr_) {
      Breakpoint* br =
          parent_process().GetBreakpoint(triggered_breakpoint_addr_);
      if (NULL != br)
        br->WriteBreakpointCode();
      triggered_breakpoint_addr_ = NULL;
    }
  }

  // Now we can proceed with debug event.
  switch (de.dwDebugEventCode) {
    case OUTPUT_DEBUG_STRING_EVENT: {
      OnOutputDebugString(debug_event);
      break;
    }
    case EXIT_THREAD_DEBUG_EVENT: {
      exit_code_ = de.u.ExitThread.dwExitCode;
      break;
    }
    case EXIT_PROCESS_DEBUG_EVENT: {
      exit_code_ = de.u.ExitProcess.dwExitCode;
      break;
    }
    case EXCEPTION_DEBUG_EVENT: {
      switch (debug_event->GetExceptionCode()) {
        case EXCEPTION_BREAKPOINT: {
          OnBreakpoint(debug_event);
          break;
        }
        case EXCEPTION_SINGLE_STEP: {
          OnSingleStep(debug_event);
          // Thread halts on all other events unconditionally,
          // but here |OnSingleStep| can decide to continue -
          // in case it's due to 'continue from breakpoint'.
          return;
        }
      }
    }
  }
  // DebugeeThread halts on all debug events, with one exception -
  // when it receives SingleStep while continue from breakpoint.
  SetState(kHalted);
}

void DebuggeeThread::EnableSingleStep(bool enable) {
  DBG_LOG("TR03.06",
          "msg='DebuggeeThread::EnableSingleStep(%s)' thread_id=%d",
          enable ? "true" : "false",
          id());

  if (parent_process().IsWoW()) {
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
    case kRunning: return "kRunning";
    case kHalted: return "kHalted";
    case kContinueFromBreakpoint: return "kContinueFromBreakpoint";
    case kDead: return "kDead";
  }
  return "N/A";
}

const char* DebuggeeThread::GetContinueOptionName(ContinueOption option) {
  switch (option) {
    case kSingleStep: return "kSingleStep";
    case kContinue: return "kContinue";
    case kContinueAndPassException: return "kContinueAndPassException";
  }
  return "N/A";
}

void DebuggeeThread::Kill() {
  if (NULL != handle_)
    debug_api().TerminateThread(handle_, 0);
}

bool DebuggeeThread::GetContext(CONTEXT* context) {
  if (!parent_process().IsHalted())
    return false;

  context->ContextFlags = CONTEXT_ALL;
  return (debug_api().GetThreadContext(handle_, context) != FALSE);
}

bool DebuggeeThread::SetContext(const CONTEXT& context) {
  if (!parent_process().IsHalted())
    return false;

  CONTEXT context_copy = context;
  return (debug_api().SetThreadContext(handle_, &context_copy) != FALSE);
}

bool DebuggeeThread::GetWowContext(WOW64_CONTEXT* context) {
  if (!parent_process().IsHalted())
    return false;

  context->ContextFlags = CONTEXT_ALL;
  return (debug_api().Wow64GetThreadContext(handle_, context) != FALSE);
}

bool DebuggeeThread::SetWowContext(const WOW64_CONTEXT& context) {
  if (!parent_process().IsHalted())
    return false;

  return (debug_api().Wow64SetThreadContext(handle_, &context) != FALSE);
}

void* DebuggeeThread::GetIP() {
  if (!parent_process().IsHalted())
    return false;

  if (parent_process().IsWoW()) {
    WOW64_CONTEXT context;
    GetWowContext(&context);
    return reinterpret_cast<void*>(context.Eip);
  } else {
    CONTEXT context;
    GetContext(&context);
#ifdef _WIN64
    return reinterpret_cast<void*>(context.Rip);
#else
    return reinterpret_cast<void*>(context.Eip);
#endif
  }
}

bool DebuggeeThread::SetIP(void* ip) {
  if (!parent_process().IsHalted())
    return false;

  if (parent_process().IsWoW()) {
    WOW64_CONTEXT context;
    if (!GetWowContext(&context))
      return false;
    context.Eip = reinterpret_cast<DWORD>(ip);
    return SetWowContext(context);
  } else {
    CONTEXT ct;
    if (!GetContext(&ct))
      return false;
#ifdef _WIN64
    ct.Rip = reinterpret_cast<DWORD64>(ip);
#else
    ct.Eip = reinterpret_cast<DWORD>(ip);
#endif
    return SetContext(ct);
  }
}
}  // namespace debug

