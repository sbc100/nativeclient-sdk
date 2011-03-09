/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * NaCl Service Runtime, C-level context switch code.
 */

#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/arch/arm/sel_rt.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"


NORETURN void NaClStartThreadInApp(struct NaClAppThread *natp,
                                   uint32_t             new_prog_ctr) {
  struct NaClApp  *nap;
  struct NaClThreadContext  *context;

  natp->sys.stack_ptr = (NaClGetSp() & ~0xf) + 4;

  /*
   * springboard pops 4 words from stack which are the parameters for
   * syscall. In this case, it is not a syscall so no parameters, but we still
   * need to adjust the stack
   */
  natp->user.stack_ptr -= 16;
  nap = natp->nap;
  context = &natp->user;
  context->spring_addr = NaClSysToUser(nap,
                                       nap->mem_start + nap->springboard_addr);
  context->new_eip = new_prog_ctr;
  context->sysret = 0; /* not used to return */

  NaClSwitch(context);
}

/*
 * syscall return
 */
NORETURN void NaClSwitchToApp(struct NaClAppThread *natp,
                              uint32_t             new_prog_ctr) {
  struct NaClApp  *nap;
  struct NaClThreadContext  *context;

  nap = natp->nap;
  context = &natp->user;
  context->spring_addr = NaClSysToUser(nap,
                                       nap->mem_start + nap->springboard_addr);
  context->new_eip = new_prog_ctr;
  context->sysret = natp->sysret;

  NaClSwitch(context);
}
