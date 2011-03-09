/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


// NOTE(gregoryd): changed the Windows implementation to use mutex instead
// of CRITICAL_SECTION

#ifndef NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_LINUX_LOCK_H_
#define NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_LINUX_LOCK_H_

#ifdef __native_client__
#include "native_client/src/trusted/service_runtime/include/machine/_types.h"
#endif  // __native_client__
#include <pthread.h>
#include "base/basictypes.h"
#include "native_client/src/shared/platform/nacl_sync.h"

// This class implements the underlying platform-specific spin-lock mechanism
// used for the Lock class.  Most users should not use LockImpl directly, but
// should instead use Lock.

namespace NaCl {

class Lock {
 friend class ConditionVariable;
 public:
  Lock();
  ~Lock();
  void Acquire();
  void Release();
  bool Try();

 private:
  pthread_mutex_t mutex_;

  DISALLOW_EVIL_CONSTRUCTORS(Lock);
};

// A helper class that acquires the given Lock while the AutoLock is in scope.
class AutoLock {
 public:
  AutoLock(Lock& lock) : lock_(lock) {
    lock_.Acquire();
  }

  ~AutoLock() {
    lock_.Release();
  }

 private:
  Lock& lock_;
  DISALLOW_EVIL_CONSTRUCTORS(AutoLock);
};

// A helper macro to perform a single operation (expressed by expr)
// in a lock
#define LOCKED_EXPRESSION(lock, expr) \
  do { \
    NaCl::AutoLock _auto_lock(lock);  \
    (expr); \
  } while (0)

}  // namespace NaCl

#endif  // NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_LINUX_LOCK_H_
