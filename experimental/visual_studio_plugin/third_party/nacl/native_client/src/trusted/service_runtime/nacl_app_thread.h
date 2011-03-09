/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Server Runtime user thread state.
 */

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_NACL_APP_THREAD_H__
#define NATIVE_CLIENT_SERVICE_RUNTIME_NACL_APP_THREAD_H__ 1

#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/trusted/service_runtime/nacl_bottom_half.h"
#include "native_client/src/trusted/service_runtime/sel_rt.h"


EXTERN_C_BEGIN

struct NaClApp;

enum NaClThreadState {
  NACL_APP_THREAD_ALIVE,
  /* NACL_APP_THREAD_INTERRUPTIBLE_MUTEX, etc */
  NACL_APP_THREAD_SUICIDE_PENDING,
  NACL_APP_THREAD_DEAD
};

/*
 * Generally, only the thread itself will need to manipulate this
 * structure, but occasionally we may need to blow away a thread for
 * some reason, and look inside.  While a thread is in the NaCl
 * application running untrusted code, the lock must *not* be held.
 */
struct NaClAppThread {
  struct NaClMutex          mu;
  struct NaClCondVar        cv;

  int                       is_privileged;  /* can make "special" syscalls? */

  uint32_t                  refcount;

  struct NaClClosureResult  result;

  uint32_t                  sysret;

  /*
   * The NaCl app that contains this thread.  The app must exist as
   * long as a thread is still alive.
   */
  struct NaClApp            *nap;

  /*
   * The effector interface object used to manipulate NaCl apps by the
   * objects in the NaClDesc class hierarchy.  Used by this thread when
   * making NaClDesc method calls from syscall handlers.
   */
  struct NaClDescEffector   *effp;

  int                       thread_num;  /* index into nap->threads */
  uintptr_t                 sys_tdb;  /* saved tdb ptr */

  struct NaClThread         thread;  /* low level thread representation */

  /*
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   *
   * The locking behavior described below is not yet implemented.
   *
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   *
   * When a thread invokes a service call from the NaCl application,
   * it must first grab its own lock (mu) before it executes any code
   * that can grab other service runtime locks/resources.  The thread
   * clears the holding_sr_locks flag as it is about to return from
   * the service call.  Short duration service calls can just hold the
   * lock throughout; potentially blocking service calls must drop the
   * thread lock and reacquire when it can unblock.  Condition
   * variables are used to allow the thread to wake up.  If a thread
   * is blocked waiting for I/O, a central epoll thread is responsible
   * for waking up the thread.  (The epoll thread serves the same
   * purpose as hardware interrupt handlers.)
   *
   * To summarily kill a thread from elsewhere, we check the
   * holding_sr_locks flag after acquiring the target thread's lock.
   * If it is clear, then the thread is running in the application
   * space (or at least it has not yet touched any service runtime
   * resources), and we can directly pthread_kill it before we release
   * the thread lock.  If it is set, we set the state flag to
   * NACL_APP_THREAD_SUICIDE_PENDING and release the thread lock,
   * possibly waking the blocked thread.
   *
   * When the thread is about to leave service runtime code and return
   * to the NaCl application, it should have released all locks to
   * service-runtime resources.  Next, the thread grabs its own lock
   * to clear the holding_sr_locks flag, at which point it examines
   * the suicide flag; if it finds that set, it should gracefully
   * exit.
   */
  int                       holding_sr_locks;

  /*
   * a thread cannot free up its own mutex lock and commit suicide,
   * since another thread may be trying to summarily kill it and is
   * waiting on the lock in order to ask it to commit suicide!
   * instead, the suiciding thread just marks itself as dead, and a
   * periodic thread grabs a global thread table lock to do thread
   * deletion (which the thread requesting summary execution must also
   * grab).
   */
  enum NaClThreadState      state;

  struct NaClThreadContext  user;
  /* sys is only used to hold segment registers */
  struct NaClThreadContext  sys;
  /*
   * NaClThread abstraction allows us to specify the stack size
   * (NACL_KERN_STACK_SIZE), but not its location.  The underlying
   * implementation takes care of finding memory for the thread stack,
   * and when the thread exits (they're not joinable), the stack
   * should be automatically released.
   */

  uintptr_t                 *syscall_args;
  /*
   * user's sp translated to system address, used for accessing syscall
   * arguments
   */
};

int NaClAppThreadCtor(struct NaClAppThread  *natp,
                      struct NaClApp        *nap,
                      int                   is_privileged,
                      uintptr_t             entry,
                      uintptr_t             stack_ptr,
                      uint32_t              tls_idx,
                      uintptr_t             sys_tdb) NACL_WUR;

void NaClAppThreadDtor(struct NaClAppThread *natp);

/*
 * Low level initialization of thread, with validated values.  The
 * usr_entry and usr_stack_ptr values are directly used to initialize the
 * user register values; the sys_tdb_base is the system address for
 * allocating a %gs thread descriptor block base.  The caller is
 * responsible for error checking: usr_entry is a valid entry point (0
 * mod N) and sys_tdb_base is in the NaClApp's address space.
 */
int NaClAppThreadAllocSegCtor(struct NaClAppThread  *natp,
                              struct NaClApp        *nap,
                              int                   is_privileged,
                              uintptr_t             usr_entry,
                              uintptr_t             usr_stack_ptr,
                              uintptr_t             sys_tdb_base,
                              size_t                tdb_size) NACL_WUR;

int NaClAppThreadIncRef(struct NaClAppThread *natp);

int NaClAppThreadDecRef(struct NaClAppThread *natp);

EXTERN_C_END

#endif  /* NATIVE_CLIENT_SERVICE_RUNTIME_NACL_APP_THREAD_H__ */
