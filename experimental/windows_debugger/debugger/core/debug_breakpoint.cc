// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_breakpoint.h"
#include "debugger/core/debug_logger.h"
#include "debugger/core/debuggee_iprocess.h"

namespace {
const unsigned char kBreakpointCode = 0xCC;
}  // namespace

namespace debug {
Breakpoint::Breakpoint()
    : address_(NULL),
      original_code_byte_(0),
      is_valid_(false),
      process_(NULL) {
}

Breakpoint::Breakpoint(void* address, IDebuggeeProcess* process)
    : address_(address),
      original_code_byte_(0),
      is_valid_(false),
      process_(process) {
}

bool Breakpoint::is_valid() const {
  return is_valid_;
}

void* Breakpoint::address() const {
  return address_;
}

unsigned char Breakpoint::original_code_byte() const {
  return original_code_byte_;
}

bool Breakpoint::WriteBreakpointCode() {
  if (!is_valid() || (NULL == process_))
    return false;
  char code = kBreakpointCode;
  bool res = process_->WriteMemory(address(), sizeof(code), &code);

  DBG_LOG("TR04.00",
          "msg=Breakpoint::WriteBreakpointCode pid=%d addr=0x%p",
          process_->id(),
          address_);
  return res;
}

bool Breakpoint::RecoverCodeAtBreakpoint() {
  if (!is_valid() || (NULL == process_))
    return false;
  bool res = process_->WriteMemory(address(),
                                   sizeof(original_code_byte_),
                                   &original_code_byte_);

  DBG_LOG("TR04.01",
          "msg=Breakpoint::RecoverCodeAtBreakpoint pid=%d addr=0x%p inst=0x%d",
          process_->id(),
          address_,
          static_cast<int>(original_code_byte_));
  return res;
}

bool Breakpoint::Init() {
  if (NULL == process_)
    return false;
  if (!process_->ReadMemory(address(),
                            sizeof(original_code_byte_),
                            &original_code_byte_))
    return false;

  is_valid_ = true;
  return (is_valid_ = WriteBreakpointCode());
}
}  // namespace debug

