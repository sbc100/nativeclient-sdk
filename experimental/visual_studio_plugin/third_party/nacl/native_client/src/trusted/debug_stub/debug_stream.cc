/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "native_client/src/trusted/debug_stub/debug_stream.h"

using namespace nacl_debug_conn;

DebugStream::DebugStream()
 : msec_timeout_(0),
   handle_(0) {}

DebugStream::~DebugStream() {}

void DebugStream::SetTimeout(uint32_t msec) {
  msec_timeout_ = msec;
}

uint32_t DebugStream::GetTimeout() const {
  return msec_timeout_;
}

void DebugStream::SetHandle(DebugHandle h) {
  handle_ = h;
}

DebugHandle DebugStream::GetHandle() const {
  return handle_;
}


