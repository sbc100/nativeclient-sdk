// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_DEBUG_SERVER_RSP_REGISTERS_H_
#define DEBUGGER_DEBUG_SERVER_RSP_REGISTERS_H_
#include <windows.h>
#include "debugger/base/debug_blob.h"

namespace rsp {
void GdbRegistersToCONTEXT(const debug::Blob& gdb_regs, CONTEXT* ct);
void CONTEXTToGdbRegisters(const CONTEXT& ct, debug::Blob* gdb_regs);
}  // namespace rsp

#endif  // DEBUGGER_DEBUG_SERVER_RSP_REGISTERS_H_
