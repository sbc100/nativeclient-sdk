// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/nacl-gdb_server/rsp_stop_from_debug_event.h"
#include "gtest/gtest.h"

namespace {
// CreateStopReplyPacket test fixture.
class CreateStopReplyPacketTest : public ::testing::Test {
 public:
  rsp::StopReply StopMsgFromDebugEvent(int debug_event_code) {
    DEBUG_EVENT wde;
    memset(&wde, 0, sizeof(wde));
    wde.dwDebugEventCode = debug_event_code;
    debug::DebugEvent debug_event;
    debug_event.set_windows_debug_event(wde);
    return rsp::CreateStopReplyPacket(debug_event);
  }

  rsp::StopReply StopMsgFromException(int exception_code) {
    DEBUG_EVENT wde;
    memset(&wde, 0, sizeof(wde));
    wde.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
    wde.u.Exception.ExceptionRecord.ExceptionCode = exception_code;
    debug::DebugEvent debug_event;
    debug_event.set_windows_debug_event(wde);
    return rsp::CreateStopReplyPacket(debug_event);
  }

  rsp::StopReply StopMsgFromExitProc(int exit_code) {
    DEBUG_EVENT wde;
    memset(&wde, 0, sizeof(wde));
    wde.dwDebugEventCode = EXIT_PROCESS_DEBUG_EVENT;
    wde.u.ExitThread.dwExitCode = exit_code;
    debug::DebugEvent debug_event;
    debug_event.set_windows_debug_event(wde);
    return rsp::CreateStopReplyPacket(debug_event);
  }

  bool Expect_SIGNALED(int debug_event_code) {
    rsp::StopReply msg = StopMsgFromDebugEvent(debug_event_code);
    return (rsp::kSIGSTOP == msg.signal_number()) &&
           (rsp::StopReply::SIGNALED == msg.stop_reason());
  }

  int GetSignalCode(rsp::StopReply msg) {
    if (rsp::StopReply::SIGNALED != msg.stop_reason())
      return 0;
    return msg.signal_number();
  }
};

// Unit tests start here.
TEST_F(CreateStopReplyPacketTest, kSIGNALED) {
  EXPECT_TRUE(Expect_SIGNALED(CREATE_PROCESS_DEBUG_EVENT));
  EXPECT_TRUE(Expect_SIGNALED(CREATE_THREAD_DEBUG_EVENT));
  EXPECT_TRUE(Expect_SIGNALED(LOAD_DLL_DEBUG_EVENT));
  EXPECT_TRUE(Expect_SIGNALED(OUTPUT_DEBUG_STRING_EVENT));
  EXPECT_TRUE(Expect_SIGNALED(UNLOAD_DLL_DEBUG_EVENT));
  EXPECT_TRUE(Expect_SIGNALED(EXIT_THREAD_DEBUG_EVENT));
}

TEST_F(CreateStopReplyPacketTest, Exceptions) {
  EXPECT_EQ(
      rsp::kSIGSEGV,
      GetSignalCode(StopMsgFromException(EXCEPTION_ACCESS_VIOLATION)));
  EXPECT_EQ(
      rsp::kSIGSEGV,
      GetSignalCode(StopMsgFromException(EXCEPTION_STACK_OVERFLOW)));
  EXPECT_EQ(
      rsp::kSIGTRAP,
      GetSignalCode(StopMsgFromException(EXCEPTION_BREAKPOINT)));
  EXPECT_EQ(
      rsp::kSIGTRAP,
      GetSignalCode(StopMsgFromException(EXCEPTION_SINGLE_STEP)));
  EXPECT_EQ(
      rsp::kSIGBUS,
      GetSignalCode(StopMsgFromException(EXCEPTION_DATATYPE_MISALIGNMENT)));
}

TEST_F(CreateStopReplyPacketTest, MoreExceptions) {
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_FLT_DENORMAL_OPERAND)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_FLT_DIVIDE_BY_ZERO)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_FLT_INEXACT_RESULT)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_FLT_INVALID_OPERATION)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_FLT_OVERFLOW)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_FLT_STACK_CHECK)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_FLT_UNDERFLOW)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_INT_DIVIDE_BY_ZERO)));
  EXPECT_EQ(
      rsp::kSIGFPE,
      GetSignalCode(StopMsgFromException(EXCEPTION_INT_OVERFLOW)));
}

TEST_F(CreateStopReplyPacketTest, RIP) {
  rsp::StopReply msg = StopMsgFromDebugEvent(RIP_EVENT);
  EXPECT_EQ(rsp::StopReply::TERMINATED, msg.stop_reason());
  EXPECT_EQ(rsp::kSIGSYS, msg.signal_number());
}

TEST_F(CreateStopReplyPacketTest, EXIT_PROCESS) {
  rsp::StopReply msg = StopMsgFromExitProc(13);
  EXPECT_EQ(rsp::StopReply::EXITED, msg.stop_reason());
  EXPECT_EQ(13, msg.exit_code());
}

TEST_F(CreateStopReplyPacketTest, TERM_PROC_by_EXCEPTION_ACCESS_VIOLATION) {
  rsp::StopReply msg = StopMsgFromExitProc(EXCEPTION_ACCESS_VIOLATION);
  EXPECT_EQ(rsp::StopReply::TERMINATED, msg.stop_reason());
  EXPECT_EQ(rsp::kSIGSEGV, msg.signal_number());
}

TEST_F(CreateStopReplyPacketTest, TERM_PROC_by_EXCEPTION_ILLEGAL_INSTRUCTION) {
  rsp::StopReply msg = StopMsgFromExitProc(EXCEPTION_ILLEGAL_INSTRUCTION);
  EXPECT_EQ(rsp::StopReply::TERMINATED, msg.stop_reason());
  EXPECT_EQ(rsp::kSIGILL, msg.signal_number());
}

TEST_F(CreateStopReplyPacketTest, TERM_PROC_by_EXCEPTION_FLT_DIVIDE_BY_ZERO) {
  rsp::StopReply msg = StopMsgFromExitProc(EXCEPTION_FLT_DIVIDE_BY_ZERO);
  EXPECT_EQ(rsp::StopReply::TERMINATED, msg.stop_reason());
  EXPECT_EQ(rsp::kSIGFPE, msg.signal_number());
}
}  // namespace

