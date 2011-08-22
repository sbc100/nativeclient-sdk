// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/nacl-gdb_server/gdb_registers.h"
#include <string.h>

// Ok, now we have a definite answer on how gdbserver package registers
// in 'g' command response.
// You have to download gdb source code, and look there:
//
// <gdb_source_root>\gdb\regformats\reg-i32.dat
// <gdb_source_root>\gdb\regformats\reg-x86-64.dat
//
// name:i386
// expedite:ebp,esp,eip
// 32:eax
// 32:ecx
// 32:edx
// 32:ebx
// 32:esp
// 32:ebp
// 32:esi
// 32:edi
// 32:eip
// 32:eflags
// 32:cs
// 32:ss
// 32:ds
// 32:es
// 32:fs
// 32:gs
//
// name:x86_64
// expedite:rbp,rsp,rip
// 64:rax
// 64:rbx
// 64:rcx
// 64:rdx
// 64:rsi
// 64:rdi
// 64:rbp
// 64:rsp
// 64:r8
// 64:r9
// 64:r10
// 64:r11
// 64:r12
// 64:r13
// 64:r14
// 64:r15
// 64:rip
// 32:eflags
// 32:cs
// 32:ss
// 32:ds
// 32:es
// 32:fs
// 32:gs

namespace {

#define X86_64_REGS \
	REG(rax);\
	REG(rbx);\
	REG(rcx);\
	REG(rdx);\
	REG(rsi);\
	REG(rdi);\
	REG(rbp);\
	REG(rsp);\
	REG(r8);\
	REG(r9);\
	REG(r10);\
	REG(r11);\
	REG(r12);\
	REG(r13);\
	REG(r14);\
	REG(r15);\
	REG(rip);\
	REG(eflags);\
	REG(cs);\
	REG(ss);\
	REG(ds);\
	REG(es);\
	REG(fs);\
	REG(gs)

// Registers SegCs, SegSs, SegDs, SegEs, SegFs and SegGs are defined as
// 64-bit in user.h for linux-64, but passed as 32-bit by RSP protocol.
//
bool IsSegmentRegisterIsPassedAs32Bit(const char* register_name) {
  const char* kSegmentRegistersPassedAs32Bit[] = {
		"eflags",
		"cs",
		"ss",
		"ds",
		"es",
		"fs",
		"gs"
  };
  size_t num = sizeof(kSegmentRegistersPassedAs32Bit) /
      sizeof(kSegmentRegistersPassedAs32Bit[0]);
  for (size_t i = 0; i < num; i++)
    if (strcmp(kSegmentRegistersPassedAs32Bit[i], register_name) == 0)
      return true;
  return false;
}

/// Copies content of the register |name| from GDB RSP packaged blob
/// into thread context.
/// @param[in] blob blob with GDB RSP packaged registers
/// @param[in] offset offset of the register data from the beginning of the blob
/// @param[out] dst destination for the register data (inside CONTEXT structure)
/// @param[int] reg_size size of the register in bytes
/// @param[in] name name of the register as defined in the CONTEXT structure
/// @return number of copied bytes
size_t CopyRegisterFromBlobToCONTEXT(const debug::Blob& blob,
                                     size_t offset,
                                     void* dst,
                                     int reg_size,
                                     const char* name) {
	if (IsSegmentRegisterIsPassedAs32Bit(name))
		reg_size = 4;

	blob.Peek(offset, dst, reg_size);
	return reg_size;
}

void CopyRegisterFromCONTEXTToBlob(const void* src,
                                   int reg_size,
                                   const char* name,
                                   debug::Blob* blob) {
  if (IsSegmentRegisterIsPassedAs32Bit(name)) {
		blob->Append(debug::Blob(src, 4));
  } else {
    blob->Append(debug::Blob(src, reg_size));
  }
}
}  // namespace

namespace rsp {
void GdbRegistersToCONTEXT(const debug::Blob& gdb_regs, user_regs_struct* ct) {
	size_t offset = 0;
#define REG(name) ct->name = 0;\
  offset += CopyRegisterFromBlobToCONTEXT(gdb_regs, \
                                          offset, \
                                          &ct->name, \
                                          sizeof(ct->name), \
                                          #name)
	X86_64_REGS;
#undef REG
}

void CONTEXTToGdbRegisters(const user_regs_struct& ct, debug::Blob* gdb_regs) {
#define REG(name) CopyRegisterFromCONTEXTToBlob(&ct.name, \
                                                sizeof(ct.name), \
                                                #name, \
                                                gdb_regs)
	X86_64_REGS;
#undef REG
}
}  // namespace rsp

