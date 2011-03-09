/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_STUB_API_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_STUB_API_H_ 1

#include "native_client/src/include/nacl_base.h"

namespace nacl_debug_stub {


struct DAPIGetInfo {
  int threadCnt;
  int pipeCnt;
};

struct DAPIAddThread {
};

} // namespace nacl_debug_stub
#endif
