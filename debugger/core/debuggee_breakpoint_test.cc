// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_breakpoint.h"
#include "debugger/core/fake_debuggee_process.h"
#include "gtest/gtest.h"

namespace {
const unsigned char kBreakpontCode = 0xCC;
void* kBigAddr = reinterpret_cast<void*>(0xAAFFEE88);
void* kSmallAddr = reinterpret_cast<void*>(2);


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
  debug::FakeDebuggeeProcess proc_;
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
  EXPECT_EQ(debug::FakeDebuggeeProcess::kFillChar, orig_code_);
}

TEST_F(BreakpointTest, MockProcessReadErr) {
  // Make sure ReadMemory fails when called with big address.
  EXPECT_FALSE(proc_.ReadMemory(big_addr_, sizeof(orig_code_), &orig_code_));
  EXPECT_EQ(0, orig_code_);
}

TEST_F(BreakpointTest, BreakpointInitialization) {
    // Tests breakpoint initialization.
  EXPECT_TRUE(bp_.Init(&proc_));
  EXPECT_EQ(debug::FakeDebuggeeProcess::kFillChar, bp_.original_code_byte());
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
  EXPECT_EQ(debug::FakeDebuggeeProcess::kFillChar, code);
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
