/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <string.h>

#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"

#include "native_client/src/trusted/debug_stub/debug_stub.h"
#include "native_client/src/trusted/debug_stub/debug_thread.h"

using namespace nacl_debug_stub;

DebugThread::DebugThread(ThreadId_t id, ThreadHandle_t hdl, intptr_t fun, void *cookie)
 :  start_fn_(fun),
    cookie_(cookie),
    id_(id),
    handle_(hdl),
    registers_(0),
    sig_(0),
    res_(DTR_WAIT) {
  
  if (sizeof(intptr_t) == 8)
    arch_ = DTA_X86_64;
  else
    arch_ = DTA_X86_32;

  registers_ = malloc(GetContextSize());
  memset(registers_, 0xFF, GetContextSize());

}

DebugThread::~DebugThread() {
  // We only allocate the context, someone else owns the NTP
  if (registers_)
    delete registers_;

  if (handle_)
    CloseHandle(handle_);
}

void *DebugThread::GetContextPtr() {
  return registers_;
}


uint32_t DebugThread::GetContextSize() const {
  switch(arch_) {
    case DTA_X86_32:  return sizeof(uint32_t) * 16;
    case DTA_X86_64:  return sizeof(uint64_t) * 17 + sizeof(uint32_t) * 7;
    case DTA_UNKNOWN: return 0;
    default:
      return 0;
  }
}

DebugThread::ThreadId_t DebugThread::GetID() const {
  return id_;
}

DebugThread::ThreadHandle_t DebugThread::GetHandle() const {
  return handle_;
}

NaClThreadContext *DebugThread::GetUserCtx() {
  return &natp_->user;
}

NaClAppThread *DebugThread::GetNaClAppThread() {
  return natp_;
}

void DebugThread::SetNaClAppThread(NaClAppThread *natp) {
  natp_ = natp;
}

