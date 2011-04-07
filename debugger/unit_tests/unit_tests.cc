// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stdio.h>
#include "debugger/core/debuggee_breakpoint_test.h"

int main(int argc, char* argv[]) {
  printf("Running DebuggeeBreakpointTest...\n");
  debug::DebuggeeBreakpointTest test1;
  int res = test1.Run();
  return res;
}
