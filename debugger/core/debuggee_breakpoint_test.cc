// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_breakpoint.h"
#include "debugger/core/debuggee_process.h"
#include "gtest/gtest.h"

namespace {
const unsigned char kBreakpontCode = 0xCC;
const unsigned char kFillChar = 0xA7;
void* kBigAddr = reinterpret_cast<void*>(0xAAFFEE88);
void* kSmallAddr = reinterpret_cast<void*>(2);

// Mock DebuggeeProcess, used in DebuggeeBreakpoint unit tests.
// Provides ReadMemory and WriteMemory methods that access internal buffer,
// not debuggee process memory. Only single byte reads/writes are
// supported - breakpoints are one byte instruction.
class DebuggeeProcessMock : public debug::DebuggeeProcess {
 public:
  DebuggeeProcessMock() : debug::DebuggeeProcess(0, NULL, 0, NULL, NULL) {
    memset(buff, kFillChar, sizeof(buff));
  }

  virtual bool ReadMemory(const void* addr, size_t size, void* destination) {
    intptr_t offset = reinterpret_cast<int>(addr);
    if ((offset < sizeof(buff)) && (1 == size)) {
      memcpy(destination, &buff[offset], size);
      return true;
    }
    return false;
  }

  virtual bool WriteMemory(const void* addr, const void* source, size_t size) {
    intptr_t offset = reinterpret_cast<int>(addr);
    if ((offset < sizeof(buff)) && (1 == size)) {
      memcpy(&buff[offset], source, size);
      return true;
    }
    return false;
  }

  char buff[1000];
};

// Breakpoint test fixture.
class BreakpointTest : public ::testing::Test {
 protected:
  BreakpointTest()
      : big_addr_(kBigAddr),
        addr_(kSmallAddr),
        bp_(kSmallAddr),
        orig_code_(0) {
  }

  void* big_addr_;
  void* addr_;
  DebuggeeProcessMock proc_;
  debug::Breakpoint bp_;
  unsigned char orig_code_;
};

// Unit tests start here.
TEST_F(BreakpointTest, EmptyBreakpoint) {
  debug::Breakpoint bp;
  EXPECT_EQ(NULL, bp.address());
  EXPECT_EQ(0, bp.original_code_byte());
  EXPECT_FALSE(bp.is_valid());
}

TEST_F(BreakpointTest, UninitializedBreakpoints) {
  void* addr = kBigAddr;
  debug::Breakpoint bp(addr);
  EXPECT_EQ(addr, bp.address());
  EXPECT_EQ(0, bp.original_code_byte());
  EXPECT_FALSE(bp.is_valid());
}

TEST_F(BreakpointTest, MockProcessReadOk) {
  // Make sure original code byte is correct.
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(orig_code_), &orig_code_));
  EXPECT_EQ(kFillChar, orig_code_);
}

TEST_F(BreakpointTest, MockProcessReadErr) {
  // Make sure ReadMemory fails when called with big address.
  EXPECT_FALSE(proc_.ReadMemory(big_addr_, sizeof(orig_code_), &orig_code_));
  EXPECT_EQ(0, orig_code_);
}

TEST_F(BreakpointTest, BreakpointInitialization) {
    // Tests breakpoint initialization.
  EXPECT_TRUE(bp_.Init(&proc_));
  EXPECT_EQ(kFillChar, bp_.original_code_byte());
  EXPECT_TRUE(bp_.is_valid());
  unsigned char code = 0;
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(code), &code));
  EXPECT_EQ(kBreakpontCode, code);
}

TEST_F(BreakpointTest, BreakpointRemoval) {
  EXPECT_TRUE(bp_.Init(&proc_));
  EXPECT_TRUE(bp_.RecoverCodeAtBreakpoint(&proc_));
  unsigned char code = 0;
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(code), &code));
  EXPECT_EQ(kFillChar, code);
}

TEST_F(BreakpointTest, BreakpointRecovery) {
  EXPECT_TRUE(bp_.Init(&proc_));
  EXPECT_TRUE(bp_.WriteBreakpointCode(&proc_));
  unsigned char code = 0;
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(code), &code));
  EXPECT_EQ(kBreakpontCode, code);
}

TEST_F(BreakpointTest, OpsOnInvalid) {
  EXPECT_TRUE(bp_.Init(&proc_));
  bp_.Invalidate();
  EXPECT_FALSE(bp_.RecoverCodeAtBreakpoint(&proc_));
  EXPECT_FALSE(bp_.WriteBreakpointCode(&proc_));
  EXPECT_TRUE(bp_.Init(&proc_));
  EXPECT_TRUE(bp_.is_valid());
}

TEST_F(BreakpointTest, OpsOnInvalidMemory) {
  debug::Breakpoint bp(kBigAddr);
  EXPECT_FALSE(bp.Init(&proc_));
  EXPECT_FALSE(bp.RecoverCodeAtBreakpoint(&proc_));
  EXPECT_FALSE(bp.WriteBreakpointCode(&proc_));
  EXPECT_FALSE(bp.is_valid());
}

}  // namespace
