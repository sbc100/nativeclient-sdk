/*
 * Copyright 2008 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Service Runtime, C-level context switch code.
 */

#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/arch/x86/sel_rt.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"


NORETURN void NaClStartThreadInApp(struct NaClAppThread *natp,
                                   nacl_reg_t           new_prog_ctr) {
  struct NaClApp            *nap;
  struct NaClThreadContext  *context;
  /*
   * Save service runtime segment registers; fs/gs is used for TLS
   * on Windows and Linux respectively, so will change.  The others
   * should be global, but we save them from the thread anyway.
   */
#if 0 /* restored by trampoline code */
  natp->sys.cs = NaClGetCs();
  natp->sys.ds = NaClGetDs();
#endif
  natp->sys.es = NaClGetEs();
  natp->sys.fs = NaClGetFs();
  natp->sys.gs = NaClGetGs();
  natp->sys.ss = NaClGetSs();
  /*
   * Preserves stack alignment.  The trampoline code loads this value
   * to %esp, then pushes the thread ID (LDT index) onto the stack as
   * argument to NaClSyscallCSegHook.  See nacl_syscall.S.
   */
  NaClSetThreadCtxSp(&natp->sys, (NaClGetStackPtr() & ~0xf) + 4);

  nap = natp->nap;
  context = &natp->user;
  context->spring_addr = NaClSysToUser(nap,
                                       nap->mem_start + nap->springboard_addr);
  context->new_prog_ctr = new_prog_ctr;
  context->sysret = 0; /* %eax not used to return */

  NaClSwitch(context);
}


/*
 * syscall return
 */
NORETURN void NaClSwitchToApp(struct NaClAppThread *natp,
                              nacl_reg_t           new_prog_ctr) {
  struct NaClApp            *nap;
  struct NaClThreadContext  *context;

  nap = natp->nap;
  context = &natp->user;
  context->new_prog_ctr = new_prog_ctr;
  context->sysret = natp->sysret;

  NaClSwitch(context);
}
