/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
#include "native_client/src/trusted/debug_stub/debug_stub.h"
#include "native_client/src/trusted/debug_stub/debug_socket_impl.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

DSError DebugSocketInit() {} 

DSError DebugSocketExit() { }

DSError DebugSocketCreate(DSHandle *h) {
  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == s)
    return PROCESS_ERROR();

  *h = (DSHandle) s;
  return DSE_OK;
}


DSError DebugSocketClose(DSHandle handle) {
  SOCKET s = (SOCKET) handle;

  // Check this isn't already invalid
  if (-1 == s)
    return DSE_OK;

  // If not then close it
  if (close(s) != 0)
    return CHECK_ERROR();

  return DSE_OK;
}


int DebugSocketGetError(int block_ok) {
  int err = GetLastError();

  if (block_ok && (EWOULDBLOCK == err))
    return 0;

  return err;
}

