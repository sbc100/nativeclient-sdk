// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_BREAKPOINT_TEST_H_
#define DEBUGGER_CORE_DEBUGGEE_BREAKPOINT_TEST_H_

// Unit test for debug::DebuggeeBreakpoint class.

namespace debug {
class DebuggeeBreakpointTest {
 public:
  DebuggeeBreakpointTest() {}

  /// Runs unit test.
  /// @return @a true on success.
  int Run();
};
}  // namespace debug

#endif  // DEBUGGER_CORE_DEBUGGEE_BREAKPOINT_TEST_H_
