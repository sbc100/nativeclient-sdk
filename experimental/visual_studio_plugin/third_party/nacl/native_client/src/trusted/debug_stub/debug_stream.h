/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_STREAM_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_STREAM_H_ 1

#include "native_client/src/include/portability.h"

namespace nacl_debug_conn {

class DebugStream {
 public:
  virtual ~DebugStream() = 0 {};

 public:
  virtual int32_t Read(void *ptr, int32_t len) = 0;
  virtual int32_t Write(void *ptr, int32_t len) = 0;
  
  virtual bool IsConnected() const = 0;
  virtual bool DataAvail() const = 0;
};

} // namespace nacl_debug_conn
#endif

