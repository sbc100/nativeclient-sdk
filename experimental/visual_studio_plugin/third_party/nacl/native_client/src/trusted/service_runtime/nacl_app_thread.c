/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Server Runtime user thread state.
 */
#include "native_client/src/shared/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/desc/nacl_desc_effector_ldr.h"

#ifdef NACL_BREAKPAD
#include "native_client/src/trusted/nacl_breakpad/nacl_breakpad.h"
#endif

#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_tls.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"

/* NMM */
#include "native_client/src/trusted/debug_stub/debug_stub.h"

void WINAPI NaClThreadLauncher(void *state) {
  struct NaClAppThread  *natp;

#ifdef NACL_BREAKPAD
  NaClBreakpadInit();
#endif

  NaClLog(4, "NaClThreadLauncher: entered\n");
  natp = (struct NaClAppThread *) state;
  NaClLog(4, "      natp = 0x%016"NACL_PRIxPTR"\n", (uintptr_t) natp);
  NaClLog(4, " prog_ctr  = 0x%016"NACL_PRIxNACL_REG"\n", natp->user.prog_ctr);
  NaClLog(4, "stack_ptr  = 0x%016"NACL_PRIxPTR"\n",
          NaClGetThreadCtxSp(&natp->user));

  NaClTlsSetIdx(NaClGetThreadIdx(natp));

  /*
   * We have to hold the threads_mu lock until after thread_num field
   * in this thread has been initialized.  All other threads can only
   * find and examine this natp through the threads table, so the fact
   * that natp is not consistent (no thread_num) will not be visible.
   */
  NaClXMutexLock(&natp->nap->threads_mu);
  natp->thread_num = NaClAddThreadMu(natp->nap, natp);
  NaClXMutexUnlock(&natp->nap->threads_mu);

  /*
   * We need to set an exception handler in every thread we start,
   * otherwise the system's default handler is called and a message box is
   * shown.
   */

  /* NMM */
  // WINDOWS_EXCEPTION_TRY;
  NaClStartThreadInApp(natp, natp->user.prog_ctr);
  // WINDOWS_EXCEPTION_CATCH;

#ifdef NACL_BREAKPAD
//  NaClBreakpadFini();
#endif
}

int NaClAppThreadCtor(struct NaClAppThread  *natp,
                      struct NaClApp        *nap,
                      int                   is_privileged,
                      uintptr_t             usr_entry,
                      uintptr_t             usr_stack_ptr,
                      uint32_t              tls_idx,
                      uintptr_t             sys_tdb) {
  int                         rv;
  uint64_t                    thread_idx;
  struct NaClDescEffectorLdr  *effp;

  NaClLog(4, "         natp = 0x%016"NACL_PRIxPTR"\n", (uintptr_t) natp);
  NaClLog(4, "          nap = 0x%016"NACL_PRIxPTR"\n", (uintptr_t) nap);
  NaClLog(4, "usr_stack_ptr = 0x%016"NACL_PRIxPTR"\n", usr_stack_ptr);

  NaClThreadContextCtor(&natp->user, nap, usr_entry, usr_stack_ptr, tls_idx);

  effp = NULL;

  if (!NaClMutexCtor(&natp->mu)) {
    return 0;
  }
  if (!NaClCondVarCtor(&natp->cv)) {
    goto cleanup_mutex;
  }

  natp->is_privileged = is_privileged;
  natp->refcount = 1;

  if (!NaClClosureResultCtor(&natp->result)) {
    goto cleanup_cv;
  }
  natp->sysret = 0;
  natp->nap = nap;

  effp = (struct NaClDescEffectorLdr *) malloc(sizeof *effp);
  if (NULL == effp) {
    goto cleanup_cv;
  }

  if (!NaClDescEffectorLdrCtor(effp, natp)) {
    goto cleanup_cv;
  }
  natp->effp = (struct NaClDescEffector *) effp;
  effp = NULL;

  natp->holding_sr_locks = 0;
  natp->state = NACL_APP_THREAD_ALIVE;

  natp->thread_num = -1;  /* illegal index */
  natp->sys_tdb = sys_tdb;

  thread_idx = NaClGetThreadIdx(natp);

  nacl_thread[thread_idx] = natp;
  nacl_user[thread_idx] = &natp->user;
  nacl_sys[thread_idx] = &natp->sys;

  /* NMM */
#ifdef WIN32
  rv = NaClThreadCtor(&natp->thread,
                      NaClDebugStubThreadStart,
                      (void *) natp,
                      NACL_KERN_STACK_SIZE);
#else
  rv = NaClThreadCtor(&natp->thread,
                      NaClThreadLauncher,
                      (void *) natp,
                      NACL_KERN_STACK_SIZE);
#endif
  if (rv != 0) {
    return rv;
  }
  NaClClosureResultDtor(&natp->result);
 cleanup_cv:
  NaClCondVarDtor(&natp->cv);
 cleanup_mutex:
  NaClMutexDtor(&natp->mu);
  free(effp);
  natp->effp = NULL;
  return 0;
}


void NaClAppThreadDtor(struct NaClAppThread *natp) {
  /*
   * the thread must not be still running, else this crashes the system
   */
  NaClThreadDtor(&natp->thread);
  NaClClosureResultDtor(&natp->result);
  (*natp->effp->vtbl->Dtor)(natp->effp);
  free(natp->effp);
  natp->effp = NULL;
  NaClTlsFree(natp);
  NaClCondVarDtor(&natp->cv);
  NaClMutexDtor(&natp->mu);
}


int NaClAppThreadAllocSegCtor(struct NaClAppThread  *natp,
                              struct NaClApp        *nap,
                              int                   is_privileged,
                              uintptr_t             usr_entry,
                              uintptr_t             usr_stack_ptr,
                              uintptr_t             sys_tdb_base,
                              size_t                tdb_size) {
  uint32_t  tls_idx;
  uint32_t  tdb_size32;

  if (tdb_size > UINT32_MAX) {
    NaClLog(LOG_ERROR, "Requested TDB size is too large");
    return 0;
  } else {
    tdb_size32 = (uint32_t) tdb_size;
  }

  /*
   * Even though we don't know what segment base/range should gs/r9/nacl_tls_idx
   * select, we still need one, since it identifies the thread when we context
   * switch back.  This use of a dummy tls is only needed for the main thread,
   * which is expected to invoke the tls_init syscall from its crt code (before
   * main or much of libc can run).  Other threads are spawned with the tdb
   * address and size as a parameter.
   */
  tls_idx = NaClTlsAllocate(natp, (void *) sys_tdb_base, tdb_size32);

  NaClLog(4,
        "NaClAppThreadAllocSegCtor: stack_ptr 0x%08"NACL_PRIxPTR", "
        "tls_idx 0x%02"NACL_PRIx32"\n",
         usr_stack_ptr, tls_idx);

  if (0 == tls_idx) {
    NaClLog(LOG_ERROR, "No tls for thread, num_thread %d\n", nap->num_threads);
    return 0;
  }

  return NaClAppThreadCtor(natp,
                           nap,
                           is_privileged,
                           usr_entry,
                           usr_stack_ptr,
                           tls_idx,
                           sys_tdb_base);
}


int NaClAppThreadIncRef(struct NaClAppThread *natp) {
  int refcount;

  NaClXMutexLock(&natp->mu);
  if (natp->refcount == 0) {
    NaClLog(LOG_FATAL, "NaClAppThreadIncRef: count was already 0!\n");
  }
  if (++natp->refcount == 0) {
    NaClLog(LOG_FATAL, "NaClAppThreadIncRef: refcount overflow\n");
  }
  refcount = natp->refcount;
  NaClXMutexUnlock(&natp->mu);

  return refcount;
}


int NaClAppThreadDecRef(struct NaClAppThread *natp) {
  int refcount;

  NaClXMutexLock(&natp->mu);
  refcount = --natp->refcount;
  NaClXMutexUnlock(&natp->mu);

  if (0 == refcount) {
    NaClAppThreadDtor(natp);
    free(natp);
  }
  return refcount;
}
