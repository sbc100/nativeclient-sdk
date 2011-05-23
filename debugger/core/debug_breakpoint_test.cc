// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_breakpoint.h"
#include "debugger/core/debuggee_process_mock.h"
#include "gtest/gtest.h"

namespace {
const unsigned char kBreakpointCode = 0xCC;
void* kBigAddr = reinterpret_cast<void*>(0xAAFFEE88);
void* kSmallAddr = reinterpret_cast<void*>(2);


// Breakpoint test fixture.
class BreakpointTest : public ::testing::Test {
 protected:
  BreakpointTest()
      : big_addr_(kBigAddr),
        addr_(kSmallAddr),
        proc_(NULL),
        orig_code_(0) {
    bp_ = new debug::Breakpoint(kSmallAddr, &proc_);
  }
  ~BreakpointTest() { delete bp_; }

  void* big_addr_;
  void* addr_;
  debug::DebuggeeProcessMock proc_;
  debug::Breakpoint* bp_;
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
  debug::Breakpoint bp(addr, &proc_);
  EXPECT_EQ(addr, bp.address());
  EXPECT_EQ(0, bp.original_code_byte());
  EXPECT_FALSE(bp.is_valid());
}

TEST_F(BreakpointTest, MockProcessReadOk) {
  // Make sure original code byte is correct.
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(orig_code_), &orig_code_));
  EXPECT_EQ(debug::DebuggeeProcessMock::kFillChar, orig_code_);
}

TEST_F(BreakpointTest, MockProcessReadErr) {
  // Make sure ReadMemory fails when called with big address.
  EXPECT_FALSE(proc_.ReadMemory(big_addr_, sizeof(orig_code_), &orig_code_));
  EXPECT_EQ(0, orig_code_);
}

TEST_F(BreakpointTest, BreakpointInitialization) {
    // Tests breakpoint initialization.
  EXPECT_TRUE(bp_->Init());
  EXPECT_EQ(debug::DebuggeeProcessMock::kFillChar, bp_->original_code_byte());
  EXPECT_TRUE(bp_->is_valid());
  unsigned char code = 0;
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(code), &code));
  EXPECT_EQ(kBreakpointCode, code);
}

TEST_F(BreakpointTest, BreakpointRemoval) {
  EXPECT_TRUE(bp_->Init());
  EXPECT_TRUE(bp_->RecoverCodeAtBreakpoint());
  unsigned char code = 0;
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(code), &code));
  EXPECT_EQ(debug::DebuggeeProcessMock::kFillChar, code);
}

TEST_F(BreakpointTest, BreakpointRecovery) {
  EXPECT_TRUE(bp_->Init());
  EXPECT_TRUE(bp_->WriteBreakpointCode());
  unsigned char code = 0;
  EXPECT_TRUE(proc_.ReadMemory(addr_, sizeof(code), &code));
  EXPECT_EQ(kBreakpointCode, code);
}

TEST_F(BreakpointTest, OpsOnInvalidMemory) {
  debug::Breakpoint bp(kBigAddr, &proc_);
  EXPECT_FALSE(bp.Init());
  EXPECT_FALSE(bp.RecoverCodeAtBreakpoint());
  EXPECT_FALSE(bp.WriteBreakpointCode());
  EXPECT_FALSE(bp.is_valid());
}
}  // namespace

