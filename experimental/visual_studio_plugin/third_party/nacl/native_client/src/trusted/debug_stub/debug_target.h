/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_TARGET_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_TARGET_H_ 1

#include "native_client/src/include/portability.h"

namespace nacl_debug_conn {


const char *RegisterIndexToName(uint32_t regIndex);

enum DebugTargetFlagsX86 {
  DF_CARRY  = 0x00000001,
  DF_PARITY = 0x00000004,
  DF_ADJUST = 0x00000010,
  DF_ZERO   = 0x00000040,
  DF_SIGN   = 0x00000080,
  DF_TRAP   = 0x00000100,
  DF_INT    = 0x00000200,
  DF_DIR    = 0x00000400,
  DF_OVER   = 0x00000800,
  DF_IOPL   = 0x00003000,
  DF_NEST   = 0x00004000,
  DF_RESUME = 0x00010000,
  DF_VIRT   = 0x00020000,
  DF_ALIGN  = 0x00040000,
  DF_VIF    = 0x00080000,
  DF_VIP    = 0x00100000,
  DF_ID     = 0x00200000
};

// Please keep this structure in sync with Registers.cs in the NaClVsx project.
//
typedef struct DebugTargetRegsX86_64_s  {
  union {
    uint64_t IntRegs[16];
    struct {
      uint64_t Rax;
      uint64_t Rbx;
      uint64_t Rcx;
      uint64_t Rdx;
      uint64_t Rsi;
      uint64_t Rdi;
      uint64_t Rbp;
      uint64_t Rsp;
      uint64_t R8;
      uint64_t R9;
      uint64_t R10;
      uint64_t R11;
      uint64_t R12;
      uint64_t R13;
      uint64_t R14;
      uint64_t R15;
    };
  };
  uint64_t Rip; 
  uint32_t EFlags; 
  uint32_t SegCs; 
  uint32_t SegSs; 
  uint32_t SegDs; 
  uint32_t SegEs; 
  uint32_t SegFs; 
  uint32_t SegGs;
  uint32_t pad;
} DebugTargetRegsX86_64_t;


} // namespace nacl_debug_conn

#endif