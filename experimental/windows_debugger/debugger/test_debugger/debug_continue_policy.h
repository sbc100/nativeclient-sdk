// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEBUGGER_CORE_DEBUG_CONTINUE_POLICY_H_
#define DEBUGGER_CORE_DEBUG_CONTINUE_POLICY_H_

#include <windows.h>
#include "debugger/core/debug_event.h"

namespace debug {
class DebuggeeThread;

/// \brief This class represents information about decision of debugger
/// to continue execution of the child or halt it.

/// Once debugger gets debug event, it has an option to
/// continue a debuggee thread or not.
/// |DecisionToContinue| allows several entities to take part in
/// decision making.
class DecisionToContinue {
 public:
  /// kNoDecision - no decision yet
  /// kWeakDecision - weak decision, can be overwritten
  /// kStrongDecision - really strong decision
  enum DecisionStrength {kNoDecision, kWeakDecision, kStrongDecision};

  /// kDontHaltDebuggee - continue debuggee thread
  /// kHaltDebuggee - halt debugee thread
  enum HaltDebuggee {kDontHaltDebuggee, kHaltDebuggee};

  /// Debugger can either pass exception to debugee or not.
  /// kPassExceptionToDebuggee - debugger will pass exception
  /// kDontPassExceptionToDebuggee - debugger will not pass exception
  enum PassExceptionToDebuggee {kPassExceptionToDebuggee,
                                kDontPassExceptionToDebuggee};
  DecisionToContinue();
  DecisionToContinue(DecisionStrength strength,
                     HaltDebuggee halt_debuggee,
                     PassExceptionToDebuggee pass_exception_to_debuggee);
  bool operator==(const DecisionToContinue& other) const;

  /// If the decision strength of |other| is stronger than |this| object,
  /// then |this| is overwritten with |other| .
  /// @return false if decisions are incompatible.
  bool Combine(const DecisionToContinue& other);

  bool IsHaltDecision() const;
  DecisionStrength decision_strength() const {
    return decision_strength_;
  }
  bool halt_debuggee() const;
  bool pass_exception_to_debuggee() const;

 public:
  DecisionStrength decision_strength_;
  bool halt_debuggee_;
  bool pass_exception_to_debuggee_;
};

/// \brief This class represents entity that can make a decision
/// to continue execution of the child or halt it.

/// It's a base for whatever complicated policy we might
/// want to have in the future.
class ContinuePolicy {
 public:
  ContinuePolicy() {}
  virtual ~ContinuePolicy() {}

  /// Makes a continue decision.
  /// @param[in] debug_event debug event
  /// @param[in] thread debuggee thread that needs decision
  /// @param[in,out] dtc decision to continue
  virtual void MakeContinueDecision(const DebugEvent& debug_event,
                                    DebuggeeThread* thread,
                                    DecisionToContinue* dtc) = 0;
};

/// This class represents default 'continue or halt' policy.

/// For native-native (trusted) threads it decides not to halt,
/// and pass exceptions to the debuggee, except breakpoint event.
/// For NaCl (untrusted) threads, it weakly decides to halt on
/// all exceptions.
class StandardContinuePolicy : public ContinuePolicy {
 public:
  /// Makes a continue decision.
  /// @param[in] debug_event debug event
  /// @param[in] thread debuggee thread that needs decision
  /// @param[in,out] dtc decision to continue
  virtual void MakeContinueDecision(const DebugEvent& debug_event,
                                    DebuggeeThread* thread,
                                    DecisionToContinue* dtc);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUG_CONTINUE_POLICY_H_

