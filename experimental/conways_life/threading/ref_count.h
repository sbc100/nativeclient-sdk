// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REF_COUNT_H_
#define REF_COUNT_H_

#include "experimental/conways_life/threading/scoped_mutex_lock.h"

namespace threading {

// A thread-safe reference counter for class CompletionCallbackFactory.
class RefCount {
 public:
  RefCount() : ref_(0) {
    pthread_mutex_init(&mutex_, NULL);
  }
  ~RefCount() {
    pthread_mutex_destroy(&mutex_);
  }

  int32_t AddRef() {
    int32_t ret_val = 0;
    threading::ScopedMutexLock scoped_mutex(&mutex_);
    if (scoped_mutex.is_valid()) {
      ret_val = ++ref_;
    }
    return ret_val;
  }

  int32_t Release() {
    int32_t ret_val = -1;
    threading::ScopedMutexLock scoped_mutex(&mutex_);
    if (scoped_mutex.is_valid()) {
      ret_val = --ref_;
    }
    return ret_val;
  }

 private:
  int32_t ref_;
  pthread_mutex_t mutex_;
};

}  // namespace threading
#endif  // REF_COUNT_H_
