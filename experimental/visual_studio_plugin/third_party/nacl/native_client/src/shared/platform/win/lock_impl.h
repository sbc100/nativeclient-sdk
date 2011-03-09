/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


// NOTE(gregoryd): changed the Windows implementation to use mutex instead
// of CRITICAL_SECTION

#ifndef NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_WIN_LOCK_IMPL_H_
#define NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_WIN_LOCK_IMPL_H_

#include <Windows.h>
#include "base/basictypes.h"

namespace NaCl {

// This class implements the underlying platform-specific spin-lock mechanism
// used for the Lock class.  Most users should not use LockImpl directly, but
// should instead use Lock.
class LockImpl {
 public:
  LockImpl();
  ~LockImpl();

  // If the lock is not held, take it and return true.  If the lock is already
  // held by something else, immediately return false.
  bool Try();

  // Take the lock, blocking until it is available if necessary.
  void Lock();

  // Release the lock.  This must only be called by the lock's holder: after
  // a successful call to Try, or a call to Lock.
  void Unlock();

 private:
  HANDLE mutex_;

  DISALLOW_EVIL_CONSTRUCTORS(LockImpl);
};

class AutoLockImpl {
 public:
  AutoLockImpl(LockImpl* lock_impl)
      : lock_impl_(lock_impl) {
    lock_impl_->Lock();
  }

  ~AutoLockImpl() {
    lock_impl_->Unlock();
  }

 private:
  LockImpl* lock_impl_;
  DISALLOW_EVIL_CONSTRUCTORS(AutoLockImpl);
};

}  // namespace NaCl

#endif  // NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_WIN_LOCK_IMPL_H_
