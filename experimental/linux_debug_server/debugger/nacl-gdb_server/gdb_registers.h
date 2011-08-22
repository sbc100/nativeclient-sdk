// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_NACL_GDB_SERVER_GDB_REGISTERS_H_
#define DEBUGGER_NACL_GDB_SERVER_GDB_REGISTERS_H_
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>

#include "debugger/base/debug_blob.h"

namespace rsp {
/// Converts from GDB RSP packet registers to linux thread user_regs_struct.
/// @param[in] gdb_regs blob with packed registers in GDB RSP format
/// @param[out] ct destination for the registers
void GdbRegistersToCONTEXT(const debug::Blob& gdb_regs, user_regs_struct* ct);

/// Converts from linux thread user_regs_struct to GDB RSP packet registers.
/// @param[in] ct thread CONTEXT structure filled with registers content
/// @param[out] gdb_regs destination for the registers
void CONTEXTToGdbRegisters(const user_regs_struct& ct, debug::Blob* gdb_regs);
}  // namespace rsp

#endif  // DEBUGGER_NACL_GDB_SERVER_GDB_REGISTERS_H_

