// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "c_salt/base/main_thread_factory_base.h"
#include "c_salt/base/utils.h"

#include "ppapi/c/pp_errors.h"


namespace c_salt {

void MainThreadFactoryBase::Callback(void* user_data, int32_t result) {
  if (result == PP_OK) {
    MainThreadFactoryBase* factory =
        static_cast<MainThreadFactoryBase*>(user_data);
    factory->CreateInstance();
    factory->SignalInstanceCreated();
  }
}

void MainThreadFactoryBase::SignalInstanceCreated() {
  signal_.Lock();
  signal_.Signal();
  signal_.Unlock();
}

void MainThreadFactoryBase::WaitForInstanceCreated(uint32_t usec_timeout) {
  timespec abs_time_out = utils::GetTimeFromNow(usec_timeout);
  signal_.Lock();
  while (new_instance_ == NULL) {
    if (!signal_.TimedWait(&abs_time_out)) {
      break;
    }
  }
  signal_.Unlock();
}

}  // namespace c_salt
