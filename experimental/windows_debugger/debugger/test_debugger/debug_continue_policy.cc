// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debug_continue_policy.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debuggee_thread.h"

namespace {
// MS Visual C++ runtime exception.
const int kVS2008_THREAD_INFO = 0x406D1388;
}  // namespace

namespace debug {
DecisionToContinue::DecisionToContinue()
    : decision_strength_(kNoDecision),
      halt_debuggee_(false),
      pass_exception_to_debuggee_(true) {
}

DecisionToContinue::DecisionToContinue(
    DecisionStrength strength,
    HaltDebuggee halt_debuggee,
    PassExceptionToDebuggee pass_exception_to_debuggee)
    : decision_strength_(strength),
      halt_debuggee_(kHaltDebuggee == halt_debuggee),
      pass_exception_to_debuggee_(
          kPassExceptionToDebuggee == pass_exception_to_debuggee) {
}

bool DecisionToContinue::operator==(const DecisionToContinue& other) const {
  return (other.decision_strength_ == decision_strength_) &&
         (other.halt_debuggee_ == halt_debuggee_) &&
         (other.pass_exception_to_debuggee_ == pass_exception_to_debuggee_);
}

bool DecisionToContinue::Combine(const DecisionToContinue& other) {
  if (other == *this)
    return true;

  if (other.decision_strength_ == decision_strength_)
    return false;

  if ((kStrongDecision == other.decision_strength_) ||
     (kNoDecision == decision_strength_))
    *this = other;

  return true;
}

bool DecisionToContinue::halt_debuggee() const {
  return halt_debuggee_;
}

bool DecisionToContinue::pass_exception_to_debuggee() const {
  return pass_exception_to_debuggee_;
}

bool DecisionToContinue::IsHaltDecision() const {
  return (kNoDecision != decision_strength_) && halt_debuggee_;
}

void StandardContinuePolicy::MakeContinueDecision(
    const DebugEvent& debug_event,
    DebuggeeThread* thread,
    DecisionToContinue* dtc) {
  if (DebugEvent::kNotNaClDebugEvent != debug_event.nacl_debug_event_code()) {
    *dtc = DecisionToContinue(
        DecisionToContinue::kStrongDecision,
        DecisionToContinue::kHaltDebuggee,
        DecisionToContinue::kDontPassExceptionToDebuggee);    
    return;
  }

  int exception_code = debug_event.windows_debug_event().u.Exception.ExceptionRecord.ExceptionCode;
  if (EXCEPTION_DEBUG_EVENT == debug_event.windows_debug_event().dwDebugEventCode) {
    if (kVS2008_THREAD_INFO == exception_code) {
      dtc->Combine(
          DecisionToContinue(
              DecisionToContinue::kWeakDecision,
              DecisionToContinue::kDontHaltDebuggee,
//              DecisionToContinue::kDontPassExceptionToDebuggee));
              DecisionToContinue::kPassExceptionToDebuggee));
    } else {
      bool is_nexe = false;
      if (NULL != thread)
        is_nexe = thread->IsNaClAppThread();

      DecisionToContinue::PassExceptionToDebuggee pass_to_debuggee =
          DecisionToContinue::kPassExceptionToDebuggee;

      if (EXCEPTION_BREAKPOINT == exception_code)
        pass_to_debuggee = DecisionToContinue::kDontPassExceptionToDebuggee;

      if (is_nexe) {
        dtc->Combine(DecisionToContinue(
            DecisionToContinue::kWeakDecision,
            DecisionToContinue::kHaltDebuggee,
            pass_to_debuggee));
      } else {
        dtc->Combine(
            DecisionToContinue(
                DecisionToContinue::kWeakDecision,
                DecisionToContinue::kDontHaltDebuggee,
                pass_to_debuggee));
      }
    }
  }
}
}  // namespace debug

