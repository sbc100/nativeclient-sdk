/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_DISPATCH_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_DISPATCH_ 1

#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"


namespace nacl_debug_stub {

class  DebugInst;
struct DAPIGetInfo;

DSResult DebugStubDispatch(int cmd, void *src, int len);
DSResult DispatchServer(DebugInst *inst, const char *addr);
DSResult DispatchInfo(DebugInst *inst, DAPIGetInfo *info);
DSResult DispatchSocketServer(DebugInst *inst, const char *addr);
DSResult DispatchTryConn(DebugInst *inst);
DSResult DispatchTryWork(DebugInst *inst);

} // namespace nacl_debug_stub
#endif