/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Server Runtime global scoped objects for handling global resources.
 */

#include "native_client/src/shared/platform/nacl_interruptible_mutex.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/trusted/service_runtime/arch/sel_ldr_arch.h"
#include "native_client/src/trusted/service_runtime/nacl_app.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"

struct NaClMutex            nacl_thread_mu = {NULL};
struct NaClTsdKey           nacl_cur_thread_key;

struct NaClThreadContext    *nacl_user[NACL_THREAD_MAX] = {NULL};
struct NaClThreadContext    *nacl_sys[NACL_THREAD_MAX] = {NULL};
struct NaClAppThread        *nacl_thread[NACL_THREAD_MAX] = {NULL};

/*
 * Hack for gdb.  This records xlate_base in a place where (1) gdb can find it,
 * and (2) gdb doesn't need debug info (it just needs symbol info).
 */
uintptr_t                   nacl_global_xlate_base;

void NaClGlobalModuleInit(void) {
  NaClMutexCtor(&nacl_thread_mu);
  NaClInitGlobals();
  /* key for TSD */
  if (!NaClTsdKeyCreate(&nacl_cur_thread_key)) {
    NaClLog(LOG_FATAL,
            "Could not create thread specific data key for cur_thread\n");
  }
}


void  NaClGlobalModuleFini(void) {
  NaClMutexDtor(&nacl_thread_mu);
}


struct NaClAppThread  *GetCurThread(void) {
  return NaClTsdGetSpecific(&nacl_cur_thread_key);
}


struct NaClApp *GetCurProc(void) {
  struct NaClAppThread *natp;

  natp = GetCurThread();
  if (NULL == natp) {
    /*
     * This should never happen; if we are so lost as to not be able
     * to figure out the current thread, we probably cannot log....
     */
    NaClLog(LOG_FATAL,
            "The puce potion hits me and shatters.  Which thread am I?!?\n");
    return (struct NaClApp *) NULL;
  }
  return natp->nap;
}
