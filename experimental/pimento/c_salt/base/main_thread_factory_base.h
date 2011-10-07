// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MAIN_THREAD_FACTORY_BASE_H_
#define BASE_MAIN_THREAD_FACTORY_BASE_H_

#include <stdint.h>
#include "c_salt/threading/thread_condition.h"

namespace c_salt {

// Provides base, non-templated functionality for MainThreadfactory.
// (See MainThreadFactory.h)
class MainThreadFactoryBase {
 public:
  MainThreadFactoryBase() : new_instance_(NULL) {}

 protected:
  // The asynchronous main-thread callback function.
  static void Callback(void* user_data, int32_t result);

  // Subclasses must override. This function is called from the main-thread
  // callback to actually create the instance. 
  virtual void CreateInstance() = 0;

  // Synchronization signals.
  void SignalInstanceCreated();
  void WaitForInstanceCreated(uint32_t usec_timeout);

  // Generic pointer to the class instance to be created. It's also used as
  // the thread condition in conjunction with the signal below.
  void* new_instance_;
  // Synchronisation signal, used when the instace creation is dispatched to
  // the main thread.
  c_salt::threading::ThreadCondition signal_;
};

}  // namespace c_salt

#endif  // BASE_MAIN_THREAD_FACTORY_BASE_H_
