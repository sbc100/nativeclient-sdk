/*
 * Copyright 2009 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef SERVICE_RUNTIME_NACL_THREAD_H__
#define SERVICE_RUNTIME_NACL_THREAD_H__ 1

/*
 * This header contains the prototypes for thread/tls related
 * functions whose implementation is highly architecture/platform
 * specific.
 *
 * The function primarily adress complications stemming from the
 * x86-32 segmentation model - on arm things are somewhat simpler.
 *
 * On x86-64, no segment is used either.  This API is (ab)used to
 * stash the tdb in a thread-local variable (TLS on NACL_LINUX and
 * NACL_WINDOWS, and TSD on NACL_OSX since OSX does not implement
 * __thread).
 */
#include "native_client/src/include/portability.h"

struct NaClAppThread;

int NaClTlsInit();

void NaClTlsFini();


/*
 * Allocate a new tls descriptor and return it.
 * On x86 this the value for the gs segment register; on arm
 * it is the value we keep in r9.
 * This is called for the main thread and all subsequent threads
 * being created via NaClAppThreadAllocSegCtor()
 */
uint32_t NaClTlsAllocate(struct NaClAppThread *natp,
                         void                 *tdb,
                         uint32_t             size) NACL_WUR;

/*
 * Free a tls descriptor (almost a nop on ARM).
 * This is called from NaClAppThreadDtor which in turn is called
 * after a thread terminates.
 */
void NaClTlsFree(struct NaClAppThread *natp);


/*
 * Called in thread bootup code, to set TLS/TSD when the thread ID is not
 * saved in a reserved register (e.g., %gs in NaCl x86-32).
 *
 * No-op for x86-32 NaCl classic and ARM.
 */
void NaClTlsSetIdx(uint32_t tls_idx);

/*
 * This is only called from the sycall TlsInit which is typically
 * call at module startup time.
 * It installs tls descriptor for the main thread and also returns it.
 */
uint32_t NaClTlsChange(struct NaClAppThread *natp,
                       void                 *tdb,
                       uint32_t             size) NACL_WUR;

/*
 * Get the current thread index which is used to look up information in a
 * number of internal structures, e.g.nacl_thread[], nacl_user[], nacl_sys[]
 */
uint32_t NaClGetThreadIdx(struct NaClAppThread *natp);


#endif /* SERVICE_RUNTIME_NACL_THREAD_H__ */
