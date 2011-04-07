// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_breakpoint.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debug_execution_engine_inside_observer.h"

namespace {
const unsigned char kBreakpointCode = 0xCC;
}  // namespace

namespace debug {
Breakpoint::Breakpoint()
    : address_(NULL),
      original_code_byte_(0),
      is_valid_(false) {
}

Breakpoint::Breakpoint(void* address)
    : address_(address),
      original_code_byte_(0),
      is_valid_(false) {
}

void Breakpoint::Invalidate() {
  is_valid_ = false;
  address_ = NULL;
  original_code_byte_ = 0;
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

bool Breakpoint::WriteBreakpointCode(DebuggeeProcess* process) {
  if (!is_valid() || (NULL == process))
    return false;
  char code = kBreakpointCode;
  bool res = process->WriteMemory(address(), &code, sizeof(code));
  process->inside_observer().OnWriteBreakpointCode(process->id(),
                                                   address(),
                                                   res);
  return res;
}

bool Breakpoint::RecoverCodeAtBreakpoint(DebuggeeProcess* process) {
  if (!is_valid() || (NULL == process))
    return false;
  bool res = process->WriteMemory(address(),
                                  &original_code_byte_,
                                  sizeof(original_code_byte_));
  process->inside_observer().OnRecoverCodeAtBreakpoint(process->id(),
                                                       address(),
                                                       original_code_byte_,
                                                       res);
  return res;
}

bool Breakpoint::Init(DebuggeeProcess* process) {
  if (!process->ReadMemory(address(),
                           sizeof(original_code_byte_),
                           &original_code_byte_))
    return false;
  is_valid_ = true;
  return (is_valid_ = WriteBreakpointCode(process));
}
}  // namespace debug

