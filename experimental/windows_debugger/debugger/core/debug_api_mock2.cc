// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api_mock2.h"


namespace debug {
BOOL DebugAPIMock2::WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent,
                                      DWORD dwMilliseconds) {
  if (events_.size() == 0)
    return FALSE;

  *lpDebugEvent = events_[0];
  events_.pop_front();
  return TRUE;
}
}  // namespace debug

