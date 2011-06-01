// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api_mock.h"
#include "debugger/core/debug_breakpoint.h"
#include "debugger/core/debug_event.h"
#include "debugger/core/debuggee_process.h"

#include "gtest/gtest.h"

namespace {
const int kFakeProcessId = 847;
const int kFakeThreadId = 378421;
const int kFakeThreadId2 = 308421;
HANDLE kFakeThreadHandle = reinterpret_cast<void*>(1);
HANDLE kFakeProcessHandle = reinterpret_cast<void*>(2);
HANDLE kFakeFileHandle = NULL;

const int kNexeThreadStarted = 1;
const int kThreadHalted = 2;
const int kThreadRunning = 3;

const debug::DebuggeeThread* kNullThreadPtr = NULL;
const debug::Breakpoint* kNullBreakpointPtr = NULL;
void* kFakeAddr1 = reinterpret_cast<void*>(0x1B0e37C5);
void* kFakeAddr2 = reinterpret_cast<void*>(0x435F976D);

class TestableDebuggeeProcess : public debug::DebuggeeProcess {
 public:
  TestableDebuggeeProcess(int id,
                          HANDLE handle,
                          HANDLE file_handle,
                          debug::DebugAPI* debug_api)
      : debug::DebuggeeProcess(id,
                               handle,
                               file_handle,
                               debug_api) {
  }

  void OnDebugEvent(debug::DebugEvent* debug_event) {
    debug::DebuggeeProcess::OnDebugEvent(debug_event);
  }

  void Halt() { state_ = kHalted; }
};

class DebuggeeProcessTest : public ::testing::Test {
 public:
  DebuggeeProcessTest() {
    proc_ = new TestableDebuggeeProcess(kFakeProcessId,
                                        kFakeProcessHandle,
                                        kFakeFileHandle,
                                        &fake_debug_api_);
    CreateThread(kFakeThreadId2);
    fake_debug_api_.ClearCallSequence();
  }

  ~DebuggeeProcessTest() {
    delete proc_;
  }

  void CreateThread(int id) {
    DEBUG_EVENT wde;
    memset(&wde, 0, sizeof(wde));
    wde.dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
    wde.dwProcessId = proc_->id();
    wde.dwThreadId = id;

    debug::DebugEvent de;
    de.set_windows_debug_event(wde);
    proc_->OnDebugEvent(&de);
    proc_->Continue();
  }

  debug::DebugEvent CreateBreakpointDebugEvent(int thread_id, void* addr) {
    DEBUG_EVENT wde;
    memset(&wde, 0, sizeof(wde));
    wde.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
    wde.dwProcessId = proc_->id();
    wde.dwThreadId = thread_id;
    wde.u.Exception.ExceptionRecord.ExceptionCode =
        EXCEPTION_BREAKPOINT;
    wde.u.Exception.ExceptionRecord.ExceptionAddress =
        addr;

    debug::DebugEvent de;
    de.set_windows_debug_event(wde);
    return de;
  }

  TestableDebuggeeProcess* proc_;
  debug::DebugAPIMock fake_debug_api_;
};

TEST_F(DebuggeeProcessTest, SimpleAccessors) {
  ASSERT_NE(reinterpret_cast<debug::DebugAPIMock*>(NULL), &proc_->debug_api());

  EXPECT_EQ(&proc_->debug_api(), &fake_debug_api_);
  EXPECT_EQ(kFakeProcessId, proc_->id());
  EXPECT_EQ(kFakeProcessHandle, proc_->handle());
  EXPECT_EQ(debug::DebuggeeProcess::kRunning, proc_->state());

  debug::DebugAPI deb_api;
  debug::DebuggeeProcess this_proc(::GetCurrentProcessId(),
                                   ::GetCurrentProcess(),
                                   kFakeFileHandle,
                                   &deb_api);

  int sz = this_proc.GetWordSizeInBits();
#ifdef _WIN64
  EXPECT_EQ(64, sz);
  EXPECT_FALSE(this_proc.IsWoW());
#else
  EXPECT_EQ(32, sz);
#endif
}

TEST_F(DebuggeeProcessTest, Threads) {
  std::deque<int> threads;
  threads.push_back(1);
  threads.push_back(2);
  proc_->GetThreadIds(&threads);
  ASSERT_EQ(1, threads.size());
  EXPECT_EQ(kFakeThreadId2, threads[0]);
  EXPECT_EQ(NULL, proc_->GetHaltedThread());

  debug::DebuggeeThread* thread = proc_->GetThread(threads[0]);
  ASSERT_NE(kNullThreadPtr, thread);
}

TEST_F(DebuggeeProcessTest, MemRead) {
  char buff[10];
  ASSERT_TRUE(proc_->ReadMemory(0, sizeof(buff), buff));
  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kReadProcessMemory);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeProcessTest, MemWrite) {
  proc_->Halt();
  char buff[10];
  ASSERT_TRUE(proc_->WriteMemory(0, sizeof(buff), buff));
  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kWriteProcessMemory);
  call_list.push_back(debug::DebugAPIMock::kFlushInstructionCache);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeProcessTest, Breakpoints) {
  CreateThread(kFakeThreadId);
  proc_->Halt();

  std::deque<debug::Breakpoint*> breakpoints;
  breakpoints.push_back(0);
  // DebuggeeProcess::GetBreakpoints shall clear 'breakpoints', just
  // in case something was there.
  proc_->GetBreakpoints(&breakpoints);
  EXPECT_EQ(0, breakpoints.size());
  EXPECT_EQ(kNullBreakpointPtr, proc_->GetBreakpoint(kFakeAddr1));
  EXPECT_EQ(kNullBreakpointPtr, proc_->GetBreakpoint(kFakeAddr2));

  EXPECT_TRUE(proc_->SetBreakpoint(kFakeAddr1));
  EXPECT_TRUE(proc_->SetBreakpoint(kFakeAddr2));
  proc_->GetBreakpoints(&breakpoints);
  ASSERT_EQ(2, breakpoints.size());

  debug::Breakpoint* br1 = breakpoints[0];
  EXPECT_EQ(kFakeAddr1, breakpoints[0]->address());
  EXPECT_EQ(kFakeAddr2, breakpoints[1]->address());

  ASSERT_NE(kNullBreakpointPtr, proc_->GetBreakpoint(kFakeAddr1));
  ASSERT_NE(kNullBreakpointPtr, proc_->GetBreakpoint(kFakeAddr2));
  EXPECT_EQ(kFakeAddr1, proc_->GetBreakpoint(kFakeAddr1)->address());
  EXPECT_EQ(kFakeAddr2, proc_->GetBreakpoint(kFakeAddr2)->address());
}

TEST_F(DebuggeeProcessTest, ThreadNewAndDelete) {
  CreateThread(kFakeThreadId + 13);
  std::deque<int> threads;
  proc_->GetThreadIds(&threads);
  ASSERT_EQ(2, threads.size());
  EXPECT_EQ(kFakeThreadId2, threads[0]);
  EXPECT_EQ(kFakeThreadId + 13, threads[1]);

  CreateThread(kFakeThreadId + 77);
  proc_->GetThreadIds(&threads);
  ASSERT_EQ(3, threads.size());
  EXPECT_EQ(kFakeThreadId + 77, threads[2]);

  DEBUG_EVENT wde;
  memset(&wde, 0, sizeof(wde));
  wde.dwProcessId = proc_->id();
  wde.dwThreadId = kFakeThreadId2;
  wde.dwDebugEventCode = EXIT_THREAD_DEBUG_EVENT;

  debug::DebugEvent de;
  de.set_windows_debug_event(wde);
  proc_->OnDebugEvent(&de);
  proc_->GetThreadIds(&threads);
  EXPECT_EQ(3, threads.size());

  proc_->Continue();
  proc_->GetThreadIds(&threads);
  EXPECT_EQ(2, threads.size());
  EXPECT_EQ(debug::DebuggeeProcess::kRunning, proc_->state());
}

TEST_F(DebuggeeProcessTest, ProcessExit) {
  DEBUG_EVENT wde;
  memset(&wde, 0, sizeof(wde));
  wde.dwDebugEventCode = EXIT_PROCESS_DEBUG_EVENT;
  wde.dwThreadId = kFakeThreadId2;
  wde.dwProcessId = proc_->id();

  debug::DebugEvent de;
  de.set_windows_debug_event(wde);

  proc_->OnDebugEvent(&de);
  EXPECT_EQ(debug::DebuggeeProcess::kHalted, proc_->state());
  EXPECT_EQ(wde.dwDebugEventCode,
            proc_->last_debug_event().windows_debug_event().dwDebugEventCode);
  EXPECT_EQ(wde.dwProcessId,
            proc_->last_debug_event().windows_debug_event().dwProcessId);
  EXPECT_EQ(wde.dwThreadId,
            proc_->last_debug_event().windows_debug_event().dwThreadId);

  proc_->Continue();
  EXPECT_EQ(debug::DebuggeeProcess::kDead, proc_->state());
}

TEST_F(DebuggeeProcessTest, HitOurBreakpoint) {
  CreateThread(kFakeThreadId);
  proc_->Halt();
  EXPECT_TRUE(proc_->SetBreakpoint(kFakeAddr1));

  debug::DebugEvent de = CreateBreakpointDebugEvent(kFakeThreadId, kFakeAddr1);
  proc_->OnDebugEvent(&de);

  EXPECT_EQ(debug::DebuggeeProcess::kHalted, proc_->state());
  EXPECT_EQ(proc_->GetThread(kFakeThreadId), proc_->GetHaltedThread());

  EXPECT_TRUE(proc_->Continue());
  EXPECT_EQ(debug::DebuggeeProcess::kRunning, proc_->state());
}

TEST_F(DebuggeeProcessTest, HitBreakpointSingleStep) {
  debug::DebugEvent de = CreateBreakpointDebugEvent(kFakeThreadId2, kFakeAddr1);
  proc_->OnDebugEvent(&de);

  EXPECT_EQ(debug::DebuggeeProcess::kHalted, proc_->state());
  EXPECT_EQ(proc_->GetThread(kFakeThreadId2), proc_->GetHaltedThread());

  EXPECT_TRUE(proc_->SingleStep());
  EXPECT_EQ(debug::DebuggeeProcess::kRunning, proc_->state());
}

TEST_F(DebuggeeProcessTest, HitUnknownBreakpoint) {
  CreateThread(kFakeThreadId);
  proc_->Halt();
  EXPECT_TRUE(proc_->SetBreakpoint(kFakeAddr1));

  debug::DebugEvent de = CreateBreakpointDebugEvent(kFakeThreadId, kFakeAddr2);
  proc_->OnDebugEvent(&de);

  EXPECT_EQ(debug::DebuggeeProcess::kHalted, proc_->state());
  EXPECT_EQ(proc_->GetThread(kFakeThreadId), proc_->GetHaltedThread());

  EXPECT_TRUE(proc_->ContinueAndPassExceptionToDebuggee());
  EXPECT_EQ(debug::DebuggeeProcess::kRunning, proc_->state());
}

TEST_F(DebuggeeProcessTest, Break) {
  proc_->Break();
  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kDebugBreakProcess);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeProcessTest, Detach) {
  proc_->Detach();
  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kDebugActiveProcessStop);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
  EXPECT_EQ(debug::DebuggeeProcess::kDead, proc_->state());
}
}  // namespace

