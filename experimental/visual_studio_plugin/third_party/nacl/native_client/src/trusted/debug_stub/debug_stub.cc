/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
#include "native_client/src/trusted/debug_stub/debug_socket.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"

void NaClDebugStubInit() {
  DebugSocketInit();
}

void NaClDebugStubFini() {
  DebugSocketExit();
}


