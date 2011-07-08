// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/nacl-gdb_server/gdb_registers.h"
#include "debugger/base/debug_blob.h"
#include "gtest/gtest.h"

#define REG(sz, name) uint##sz##_t name
#pragma pack(push, 1)
#ifdef _WIN64
struct GdbRegisters {
  REG(64, rax);
  REG(64, rbx);
  REG(64, rcx);
  REG(64, rdx);
  REG(64, rsi);
  REG(64, rdi);
  REG(64, rbp);
  REG(64, rsp);
  REG(64, r8);
  REG(64, r9);
  REG(64, r10);
  REG(64, r11);
  REG(64, r12);
  REG(64, r13);
  REG(64, r14);
  REG(64, r15);
  REG(64, rip);
  REG(32, eflags);
  REG(32, cs);
  REG(32, ss);
  REG(32, ds);
  REG(32, es);
  REG(32, fs);
  REG(32, gs);
};
#else
struct GdbRegisters {
  REG(32, eax);
  REG(32, ecx);
  REG(32, edx);
  REG(32, ebx);
  REG(32, esp);
  REG(32, ebp);
  REG(32, esi);
  REG(32, edi);
  REG(32, eip);
  REG(32, eflags);
  REG(32, cs);
  REG(32, ss);
  REG(32, ds);
  REG(32, es);
  REG(32, fs);
  REG(32, gs);
};
#endif
#pragma pack(pop)
#define REG_OFFSET(name) offsetof(GdbRegisters, name)

namespace {
// GdbRegisters test fixture.
class GdbRegistersTest : public ::testing::Test {
 public:
  GdbRegistersTest::GdbRegistersTest() {
    memset(&gdb_registers_, 0, sizeof(gdb_registers_));
    memset(&ct_, 0, sizeof(ct_));
  }

  void WriteRegister(size_t offset, int reg_size_in_bytes, uint64_t value) {
    char* ptr = reinterpret_cast<char*>(&gdb_registers_);
    memcpy(ptr + offset, &value, reg_size_in_bytes);
    rsp::GdbRegistersToCONTEXT(debug::Blob(&gdb_registers_,
                                           sizeof(gdb_registers_)),
                                           &ct_);
  }

  GdbRegisters gdb_registers_;
  CONTEXT ct_;
};

#define TEST_RW_REG(name, ct_name) \
  CONTEXT copy_ct = ct_;\
  memcpy(&copy_ct, &ct_, sizeof(copy_ct));\
  EXPECT_EQ(0, gdb_registers_.##name);\
  WriteRegister(REG_OFFSET(name), sizeof(gdb_registers_.##name), kRegValue);\
  EXPECT_EQ(kRegValue, gdb_registers_.##name);\
  EXPECT_EQ(gdb_registers_.##name, ct_.##ct_name);\
  WriteRegister(REG_OFFSET(name), \
                sizeof(gdb_registers_.##name), \
                kInvertedRegValue);\
  EXPECT_EQ(kInvertedRegValue, gdb_registers_.##name);\
  EXPECT_EQ(kInvertedRegValue, ct_.##ct_name);\
  WriteRegister(REG_OFFSET(name), sizeof(gdb_registers_.##name), 0);\
  EXPECT_EQ(0, gdb_registers_.##name);\
  EXPECT_EQ(gdb_registers_.##name, ct_.##ct_name);\
  EXPECT_EQ(0, memcmp(&copy_ct, &ct_, sizeof(copy_ct)))

#define TEST_64REG(name, ct_name) \
TEST_F(GdbRegistersTest, name) {\
  uint64_t kRegValue = 0x1234567890abcdef;\
  uint64_t kInvertedRegValue = ~kRegValue;\
  TEST_RW_REG(name, ct_name);\
}

#define TEST_32REG(name, ct_name) \
TEST_F(GdbRegistersTest, name) {\
  uint32_t kRegValue = 0x12345678;\
  uint32_t kInvertedRegValue = ~kRegValue;\
  TEST_RW_REG(name, ct_name);\
}

#define TEST_16REG(name, ct_name) \
TEST_F(GdbRegistersTest, name) {\
  WORD kRegValue = 0x1234;\
  WORD kInvertedRegValue = ~kRegValue;\
  TEST_RW_REG(name, ct_name);\
}

// Unit tests start here.
#ifdef _WIN64
TEST_F(GdbRegistersTest, TestOfTheTestHarness) {
  EXPECT_EQ(0, REG_OFFSET(rax));
  EXPECT_EQ(8, REG_OFFSET(rbx));
  EXPECT_EQ(80, REG_OFFSET(r10));
  EXPECT_EQ(164, sizeof(GdbRegisters));
  EXPECT_EQ(140, REG_OFFSET(cs));
  EXPECT_EQ(160, REG_OFFSET(gs));
}

TEST_64REG(rax, Rax)
TEST_64REG(rbx, Rbx)
TEST_64REG(rcx, Rcx);
TEST_64REG(rdx, Rdx);
TEST_64REG(rsi, Rsi);
TEST_64REG(rdi, Rdi);
TEST_64REG(rbp, Rbp);
TEST_64REG(rsp, Rsp);
TEST_64REG(r8, R8);
TEST_64REG(r9, R9);
TEST_64REG(r10, R10);
TEST_64REG(r11, R11);
TEST_64REG(r12, R12);
TEST_64REG(r13, R13);
TEST_64REG(r14, R14);
TEST_64REG(r15, R15);
TEST_64REG(rip, Rip);
TEST_32REG(eflags, EFlags);
TEST_16REG(cs, SegCs);
TEST_16REG(ss, SegSs);
TEST_16REG(ds, SegDs);
TEST_16REG(es, SegEs);
TEST_16REG(fs, SegFs);
TEST_16REG(gs, SegGs);

#else

TEST_32REG(eax, Eax);
TEST_32REG(ecx, Ecx);
TEST_32REG(edx, Edx);
TEST_32REG(ebx, Ebx);
TEST_32REG(esp, Esp);
TEST_32REG(ebp, Ebp);
TEST_32REG(esi, Esi);
TEST_32REG(edi, Edi);
TEST_32REG(eip, Eip);
TEST_32REG(eflags, EFlags);
TEST_32REG(cs, SegCs);
TEST_32REG(ss, SegSs);
TEST_32REG(ds, SegDs);
TEST_32REG(es, SegEs);
TEST_32REG(fs, SegFs);
TEST_32REG(gs, SegGs);

#endif  // _WIN64
}  // namespace

