#ifndef NACL_SDK_DEBUG_CORE_DEBUG_CONTINUE_H_
#define NACL_SDK_DEBUG_CORE_DEBUG_CONTINUE_H_

#include <windows.h>

namespace debug {
class ExecutionEngine;

class DecisionToContinue {
 public:
  enum DecisionStrength {NO_DECISION, WEAK_DECISION, STRONG_DECISION};
  enum HaltDebuggee {DONT_HALT_DEBUGGEE, HALT_DEBUGGEE};
  enum PassExceptionToDebuggee {PASS_EXCEPTION_TO_DEBUGGEE,
                                DONT_PASS_EXCEPTION_TO_DEBUGGEE};
  DecisionToContinue();
  DecisionToContinue(DecisionStrength strength,
                     HaltDebuggee halt_debuggee, 
                     PassExceptionToDebuggee pass_exception_to_debuggee=DONT_PASS_EXCEPTION_TO_DEBUGGEE);
  bool operator==(const DecisionToContinue& other) const;

  // Returns false if decisions are incompatible.
  bool Combine(const DecisionToContinue& other);
  
  bool IsHaltDecision() const;
  DecisionStrength decision_strength() const;
  bool halt_debuggee() const;
  bool pass_exception_to_debuggee() const;

 private:
  DecisionStrength decision_strength_;
  bool halt_debuggee_;
  bool pass_exception_to_debuggee_;
};

class ContinuePolicy {
 public:
  ContinuePolicy() {}
  virtual ~ContinuePolicy() {}
  virtual void MakeContinueDecision(DEBUG_EVENT& de, DecisionToContinue* dtc) = 0;
};

class StandardContinuePolicy : public ContinuePolicy {
 public:
  StandardContinuePolicy(ExecutionEngine& debug_core) : debug_core_(debug_core) {}

  virtual void MakeContinueDecision(DEBUG_EVENT& de, DecisionToContinue* dtc);

 private:
  ExecutionEngine& debug_core_;
};
}  // namespace debug
#endif  // NACL_SDK_DEBUG_CORE_DEBUG_CONTINUE_H_
