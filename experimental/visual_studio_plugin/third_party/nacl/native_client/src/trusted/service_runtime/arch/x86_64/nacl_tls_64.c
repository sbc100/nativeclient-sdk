/*
 * Copyright 2009 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NB: 64-bit OSX support is not tested at this point.
 */
#if NACL_OSX
# include <pthread.h>
#endif

#include "native_client/src/include/portability.h"
#include "native_client/src/include/nacl_macros.h"

#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/service_runtime/arch/x86/sel_ldr_x86.h"
#include "native_client/src/trusted/service_runtime/arch/x86/sel_rt.h"
#include "native_client/src/trusted/service_runtime/arch/x86/nacl_ldt_x86.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_tls.h"

static struct NaClMutex gNaClTlsMu;

static int gNaClThreadIdxInUse[NACL_THREAD_MAX];  /* bool */
static size_t const kNumThreads = NACL_ARRAY_SIZE_UNSAFE(gNaClThreadIdxInUse);

static void NaClThreadStartupCheck() {
  /* Verify that the thread context size is what we expect */
  CHECK(sizeof(struct NaClThreadContext) == 0xa8);

  /*
   * really only needed for OSX -- make sure that the register
   * size does not exceed the size of a void*.
   */
  CHECK(sizeof(void *) >= sizeof ((struct NaClThreadContext *) 0)->prog_ctr);
}


static int NaClThreadIdxInit () {
  size_t i;

  for (i = 0; i < kNumThreads; i++) {
    gNaClThreadIdxInUse[i] = 0;
  }
  if (!NaClMutexCtor(&gNaClTlsMu)) {
    NaClLog(LOG_WARNING,
            "NaClTlsInit: gNaClTlsMu initialization failed\n");
    return 0;
  }
  return 1;
}

static void NaClThreadIdxFini() {
  NaClMutexDtor(&gNaClTlsMu);
}

/*
 * Returns -1 for error, [0, NACL_ARRAY_SIZE(gNaClThreadIdxInUse)) on success.
 */
static int NaClThreadIdxAllocate() {
  size_t i;

  NaClXMutexLock(&gNaClTlsMu);
  for (i = 0; i < kNumThreads; i++) {
    if (!gNaClThreadIdxInUse[i]) {
      gNaClThreadIdxInUse[i] = 1;
      break;
    }
  }
  NaClXMutexUnlock(&gNaClTlsMu);

  if (NACL_ARRAY_SIZE(gNaClThreadIdxInUse) != i) {
    return (int) i;
  }
  NaClLog(LOG_ERROR, "NaClAllocateThreadIdx: no more slots for a thread\n");
  return -1;
}

static void NaClThreadIdxFree(uint32_t i) {
  CHECK(i < kNumThreads);

  NaClXMutexLock(&gNaClTlsMu);
  gNaClThreadIdxInUse[i] = 0;
  NaClXMutexUnlock(&gNaClTlsMu);
}

uint32_t NaClGetThreadIdx(struct NaClAppThread *natp) {
  return natp->user.tls_idx;
}

#if NACL_OSX

pthread_key_t nacl_thread_info_key;

int NaClTlsInit() {
  int errnum;

  NaClThreadStartupCheck();

  if (!NaClThreadIdxInit()) {
    return 0;
  }

  errnum = pthread_key_create(&nacl_thread_info_key, (void (*)(void *)) 0);
  if (0 != errnum) {
    NaClLog(LOG_WARNING,
            "NaClTlsInit: pthread_key_create failed, %d\n",
            errnum);
    return 0;
  }
  return 1;
}


void NaClTlsFini() {
  int errnum = pthread_key_delete(nacl_thread_info_key);
  if (0 != errnum) {
    NaClLog(LOG_FATAL,
            "NaClTlsInit: pthread_key_delete failed, %d\n",
            errnum);
  }
  NaClThreadIdxFini();
  return;
}


/*
 * Allocation does not mean we can set the TSD variable, since we are
 * not that thread.  The setting of the TSD must wait until the thread
 * actually launches.
 */
uint32_t NaClTlsAllocate(struct NaClAppThread *natp,
                         void                 *base_addr,
                         uint32_t             size) {
  int i;

  UNREFERENCED_PARAMETER(size);

  i = NaClThreadIdxAllocate();

  if (-1 == i) {
    NaClLog(LOG_ERROR, "NaClTlsAllocate: no more slots for a thread\n");
    return 0;
  }

  natp->user.tls_base = base_addr;

  /* bias by 1: successful return value is never 0 */
  return i + 1;
}


void NaClTlsSetIdx(uint32_t tls_idx) {
#if 1  /* PARANOIA */
  if (NULL != pthread_getspecific(nacl_thread_info_key)) {
    NaClLog(LOG_WARNING,
            "NaClSetThreadInfo invoked twice for the same thread\n");
  }
#endif
  pthread_setspecific(nacl_thread_info_key, (void *) tls_idx);
}


/*
 * May be called from asm (or have compiler-generated code copied into
 * asm)!  We assume that register state is amenable to the use of
 * pthread_getspecific.  For both x86-32 and x86-64 OSX, this compiles
 * to just a memory load using the %gs or %fs segment prefixes, and
 * for x86-64 we do not change %fs.  (However, we are unlikely to use
 * this for x86-32 on OSX, since that's handled by NaCl "Classic"
 * where %gs gets swapped, and we use %gs >> 3 in the asm code.)
 */
int NaClTlsGetIdx(void) {
  return (int) (intptr_t) pthread_getspecific(nacl_thread_info_key);
}


void NaClTlsFree(struct NaClAppThread *natp) {
  int tls_idx;

  tls_idx = NaClGetThreadIdx(natp);
  NaClThreadIdxFree(tls_idx - 1);
  /*
   * don't do
   *
   * pthread_setspecific(nacl_thread_info_key, (void *) 0);
   *
   * since this is invoked from the NaClAppThread's dtor, not by the
   * dying thread.  The thread is already dead and gone, so the TSD is
   * no longer available.
   */
}


uint32_t NaClTlsChange(struct NaClAppThread *natp,
                       void                 *base_addr,
                       uint32_t             size) {
  UNREFERENCED_PARAMETER(size);

  natp->user.tls_base = base_addr;
  return NaClGetThreadIdx(natp);
}


#elif NACL_LINUX || NACL_WINDOWS


THREAD uint32_t nacl_thread_index;
/* encoded index; 0 is used to indicate error */

int NaClTlsInit() {

  NaClThreadStartupCheck();

  if (!NaClThreadIdxInit()) {
    return 0;
  }
  return 1;
}


void NaClTlsFini() {
  NaClThreadIdxFini();
}


/*
 * The API, defined for x86-32, requires that we return 0 for error.
 * This was because 0 is an illegal segment selector value.  Since we
 * are essentially allocating indices in an array and array indexing
 * is zero-based, we must bias the result.
 */
uint32_t NaClTlsAllocate(struct NaClAppThread *natp,
                         void                 *base_addr,
                         uint32_t             size) {
  int i;

  UNREFERENCED_PARAMETER(size);

  i = NaClThreadIdxAllocate();
  if (-1 == i) {
    NaClLog(LOG_ERROR, "NaClTlsAllocate: no more slots for a thread\n");
    return 0;
  }
  natp->user.tls_base = base_addr;

  /*
   * since we return 0 for error, we bias the result.
   */
  return i + 1;
}


/*
 * On x86-64, we must avoid using segment registers since Windows
 * Vista 64, Windows 7 64, etc do not permit user code to create LDT
 * entries.  The sandboxing scheme reserves %r15 for the base of the
 * NaCl app's 4G address space, but does not reserve any additional
 * registers for thread identity.  This reduces additional register
 * pressure, but implies that we have to figure out the thread
 * identity in some othre way -- we use TLS (or TSD, see below) to do
 * so, and on context switch we must access the TLS variable in order
 * to determine where to save the user register context.
 */
void NaClTlsSetIdx(uint32_t tls_idx) {
  nacl_thread_index = tls_idx;
}

uint32_t NaClTlsGetIdx(void) {
  return nacl_thread_index;
}

void NaClTlsFree(struct NaClAppThread *natp) {
                 uint32_t             tls_idx;

  tls_idx = NaClGetThreadIdx(natp);
  NaClThreadIdxFree(tls_idx - 1);
}

uint32_t NaClTlsChange(struct NaClAppThread   *natp,
                       void                   *base_addr,
                       uint32_t               size) {
  UNREFERENCED_PARAMETER(size);

  natp->user.tls_base = base_addr;
  return NaClGetThreadIdx(natp);
}


#else
# error "Woe to the service runtime.  What OS is it being compiled for?!?"
#endif
