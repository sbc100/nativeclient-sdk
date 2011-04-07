#include "debug_execution_engine.h"
#include "debug_continue_policy.h"
#include "debuggee_process.h"
#include "debuggee_thread.h"

namespace {
const int kVS2008_THREAD_INFO = 0x406D1388;
}  // namespace

namespace debug {
void StandardContinuePolicy::MakeContinueDecision(DEBUG_EVENT& de, DecisionToContinue* dtc) {
  if (EXCEPTION_DEBUG_EVENT == de.dwDebugEventCode) {
    if (kVS2008_THREAD_INFO == de.u.Exception.ExceptionRecord.ExceptionCode) {
      dtc->Combine(DecisionToContinue(DecisionToContinue::WEAK_DECISION, DecisionToContinue::DONT_HALT_DEBUGGEE, DecisionToContinue::DONT_PASS_EXCEPTION_TO_DEBUGGEE));
    } else {
      DebuggeeProcess* proc = debug_core_.GetProcess(de.dwProcessId);
      DebuggeeThread* thread = NULL;
      if (NULL != proc)
        thread = proc->GetThread(de.dwThreadId);

      bool is_nexe = false;
      if (NULL != thread)
        is_nexe = thread->is_nexe();

      DecisionToContinue::PassExceptionToDebuggee pass_to_debuggee = (EXCEPTION_BREAKPOINT != de.u.Exception.ExceptionRecord.ExceptionCode) ? DecisionToContinue::PASS_EXCEPTION_TO_DEBUGGEE : DecisionToContinue::DONT_PASS_EXCEPTION_TO_DEBUGGEE;
      if (is_nexe) {
        dtc->Combine(DecisionToContinue(DecisionToContinue::WEAK_DECISION, DecisionToContinue::HALT_DEBUGGEE, pass_to_debuggee));
      } else {
        dtc->Combine(DecisionToContinue(DecisionToContinue::WEAK_DECISION, DecisionToContinue::DONT_HALT_DEBUGGEE, pass_to_debuggee));
      }
    }
  }
}

DecisionToContinue::DecisionToContinue()
    : decision_strength_(NO_DECISION),
      halt_debuggee_(false),
      pass_exception_to_debuggee_(true) {
}

DecisionToContinue::DecisionToContinue(
    DecisionStrength strength,
    HaltDebuggee halt_debuggee, 
    PassExceptionToDebuggee pass_exception_to_debuggee)
    : decision_strength_(strength),
      halt_debuggee_(HALT_DEBUGGEE == halt_debuggee),
      pass_exception_to_debuggee_(PASS_EXCEPTION_TO_DEBUGGEE == pass_exception_to_debuggee) {
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

  if ((STRONG_DECISION == other.decision_strength_) || (NO_DECISION == decision_strength_))
    *this = other;
    
  return true;
}

DecisionToContinue::DecisionStrength DecisionToContinue::decision_strength() const {
  return decision_strength_;
}

bool DecisionToContinue::halt_debuggee() const {
  return halt_debuggee_;
}

bool DecisionToContinue::pass_exception_to_debuggee() const {
  return pass_exception_to_debuggee_;
}

bool DecisionToContinue::IsHaltDecision() const {
  return (NO_DECISION != decision_strength_) && halt_debuggee_;
}
}  // namespace debug
