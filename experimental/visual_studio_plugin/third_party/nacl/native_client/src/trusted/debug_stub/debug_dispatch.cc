/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
#include "native_client/src/shared/platform/nacl_threads.h"

#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"

#include "native_client/src/trusted/debug_stub/debug_dispatch.h"
#include "native_client/src/trusted/debug_stub/debug_inst.h"
#include "native_client/src/trusted/debug_stub/debug_packet.h"
#include "native_client/src/trusted/debug_stub/debug_pipe.h"
#include "native_client/src/trusted/debug_stub/debug_socket.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"
#include "native_client/src/trusted/debug_stub/debug_stub_api.h"
#include "native_client/src/trusted/debug_stub/debug_thread.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"



EXTERN_C_BEGIN

DSResult NaClDebugStubDispatch(int cmd, void *src, int len) {
    return nacl_debug_stub::DebugStubDispatch(cmd, src, len);
}

EXTERN_C_END

using namespace nacl_debug_conn;
using namespace nacl_debug_stub;

DSResult nacl_debug_stub::DebugStubDispatch(int cmd, void *src, int len) {
  DebugInst *inst = DebugInst::DebugStub();

  if (inst) {
    debug_log_info("Request handle %d at %Xh size %d\n",
                    cmd, src, len);

    switch(cmd) {
      case DAPI_SET_ENV:
        if (len != sizeof(DAPISetEnvironment_t))
          return DS_ERROR;
        inst->SetEnvironment((DAPISetEnvironment_t *) src);
        return DS_OK;

      case DAPI_INSERT_BREAK:
        inst->AddBreakpoint((intptr_t) src);
        inst->EnableBreakpoint((intptr_t) src);
        return DS_OK;

      case DAPI_GET_INFO: 
        if (sizeof(DAPIGetInfo) != len)
          return DS_ERROR;
        return DispatchInfo(inst, (DAPIGetInfo *) src);

      case DAPI_SOCKER_SERVER:
        if (src == 0)
          return DS_ERROR;      
        return DispatchServer(inst, (const char *) src);
    }
  }

  return DS_ERROR;
}

void WINAPI DebugServer(void *cookie) {
  DebugInst *target = DebugInst::DebugStub();

  debug_log_warning("Started debugging server.\n");
  while(1) {
    if (target->GetPendingClient()) {
      debug_log_warning("Debugger Connected.\n");
      while (1) {
        if (target->PacketPump() == DS_ERROR)
          break;
      }
    }
    debug_log_warning("Debugger Disconnected.\n");
    // target->RemoveAllBreakpoints();
    // target->ResumeThreads();
    target->FreeClient();
  } 
}

DSResult nacl_debug_stub::DispatchServer(DebugInst *inst, const char *addr) {
  NaClThread *thread;
  if (false == inst->PrepSocketServer(addr)) {
    debug_log_error("Failed to prep server socket at '%s'.\n", addr);
    return DS_ERROR;
  }

  thread = new NaClThread;
  if (NaClThreadCtor(thread, DebugServer, 0, 65536))
    return DS_OK;

  delete thread;
  return DS_ERROR;
}


DSResult nacl_debug_stub::DispatchInfo(DebugInst *inst, DAPIGetInfo *info) {
  info->pipeCnt = (inst->HasPipe() ? 1 : 0);
  info->threadCnt = (int) inst->GetThreadCount();

  return DS_OK;
}
