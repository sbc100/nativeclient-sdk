// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_API_MOCK2_H_
#define DEBUGGER_CORE_DEBUG_API_MOCK2_H_
#include <deque>
#include "debugger/core/debug_api_mock.h"

namespace debug {
/// This class is a mock of DebugAPI, used in unit tests.
///
class DebugAPIMock2 : public DebugAPIMock {
 public:
  DebugAPIMock2() {}

  virtual BOOL WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent,
                                 DWORD dwMilliseconds);

  std::deque<DEBUG_EVENT> events_;
};
}  // namespace debug

#endif  // DEBUGGER_CORE_DEBUG_API_MOCK2_H_

