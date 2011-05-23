// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api_mock.h"
#include "debugger/core/debug_event.h"
#include "debugger/core/debuggee_iprocess.h"
#include "debugger/core/debuggee_process_mock.h"
#include "debugger/core/debuggee_thread.h"
#include "gtest/gtest.h"

namespace {
const int kFakeProcessId = 847;
const int kFakeThreadId = 378421;
const int kFakeThreadId2 = 308421;
HANDLE kFakeThreadHandle = 0;
HANDLE kFakeProcessHandle = 0;
HANDLE kFakeFileHandle = 0;

const int kNexeThreadStarted = 1;
const int kThreadHalted = 2;
const int kThreadRunning = 3;

const char* kNexeUuid =
    "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11} -v 1"
    " -event NaClThreadStart -mb c0000000 -ep 20080";
const void* kNexeMemBaseAddr = reinterpret_cast<void*>(0xc0000000);
const void* kNexeEntryPoint = reinterpret_cast<void*>(0x20080);

class TestableDebuggeeThread : public debug::DebuggeeThread {
 public:
  TestableDebuggeeThread(int id,
                         HANDLE handle,
                         debug::IDebuggeeProcess* parent_process)
      : debug::DebuggeeThread(id, handle, parent_process) {}

  void Kill() { debug::DebuggeeThread::Kill(); }
  void OnDebugEvent(debug::DebugEvent* debug_event) {
    debug::DebuggeeThread::OnDebugEvent(debug_event);
  }
  void Continue(debug::DebuggeeThread::ContinueOption option) {
    debug::DebuggeeThread::Continue(option);
  }
  debug::IDebuggeeProcess& parent_process() {
    return debug::DebuggeeThread::parent_process();
  }
  debug::DebugAPI& debug_api() { return debug::DebuggeeThread::debug_api(); }
};

// DebuggeeThread test fixture.
class DebuggeeThreadTest : public ::testing::Test {
 public:
  DebuggeeThreadTest() {
    fake_proc_ = new debug::DebuggeeProcessMock(&fake_debug_api_);
    fake_proc_->AddThread(kFakeThreadId2, kFakeThreadHandle);

    no_thread_ = new TestableDebuggeeThread(kFakeThreadId,
                                            kFakeThreadHandle,
                                            fake_proc_);

    this_thread_ = new TestableDebuggeeThread(GetCurrentThreadId(),
                                              GetCurrentThread(),
                                              fake_proc_);

    fake_thread_ = new TestableDebuggeeThread(kFakeThreadId,
                                              kFakeThreadHandle,
                                              fake_proc_);
    DEBUG_EVENT wde;
    memset(&wde, 0, sizeof(wde));
    wde.dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
    wde.dwProcessId = fake_proc_->id();
    wde.dwThreadId = kFakeThreadId;

    debug::DebugEvent de;
    de.set_windows_debug_event(wde);
    fake_proc_->OnDebugEvent(&de);
  }

  ~DebuggeeThreadTest() {
    delete fake_proc_;
    delete fake_thread_;
    delete no_thread_;
    delete this_thread_;
  }

  void InitDebugEventWithString(const char* str,
                                 int addr,
                                 debug::DebugEvent* de) {
    DEBUG_EVENT wde;
    memset(&wde, 0, sizeof(wde));
    wde.dwDebugEventCode = OUTPUT_DEBUG_STRING_EVENT;
    wde.u.DebugString.lpDebugStringData = reinterpret_cast<char*>(addr);
    wde.u.DebugString.nDebugStringLength = static_cast<WORD>(strlen(str));
    fake_proc_->WriteMemory(
        wde.u.DebugString.lpDebugStringData,
        strlen(str),
        str);
    de->set_windows_debug_event(wde);
  }

  TestableDebuggeeThread* no_thread_;
  TestableDebuggeeThread* this_thread_;
  debug::DebuggeeProcessMock* fake_proc_;
  TestableDebuggeeThread* fake_thread_;
  debug::DebugAPIMock fake_debug_api_;
};

TEST_F(DebuggeeThreadTest, SimpleAccessors) {
  EXPECT_EQ(fake_proc_, &no_thread_->parent_process());
  EXPECT_EQ(kFakeThreadId, no_thread_->id());
  EXPECT_EQ(&fake_proc_->debug_api(), &no_thread_->debug_api());
  EXPECT_EQ(debug::DebuggeeThread::kHalted, no_thread_->state());
  EXPECT_STREQ("kHalted",
               debug::DebuggeeThread::GetStateName(no_thread_->state()));
}

TEST_F(DebuggeeThreadTest, Nothread) {
  EXPECT_FALSE(no_thread_->IsNaClAppThread());
  EXPECT_TRUE(no_thread_->IsHalted());
  CONTEXT context;
  EXPECT_TRUE(no_thread_->GetContext(&context));
  EXPECT_TRUE(no_thread_->SetContext(context));

  WOW64_CONTEXT wow;
  EXPECT_TRUE(no_thread_->GetWowContext(&wow));
  EXPECT_TRUE(no_thread_->SetWowContext(wow));
}

TEST_F(DebuggeeThreadTest, NothreadShallNotCrash) {
  no_thread_->Kill();
  no_thread_->Continue(debug::DebuggeeThread::kContinue);
  no_thread_->Continue(debug::DebuggeeThread::kContinueAndPassException);
  no_thread_->Continue(debug::DebuggeeThread::kSingleStep);
  no_thread_->SetIP(NULL);

  debug::DebugEvent de;
  no_thread_->OnDebugEvent(&de);
}

TEST_F(DebuggeeThreadTest, SimpleAccessorsForThisThread) {
  EXPECT_EQ(GetCurrentThreadId(), this_thread_->id());
}

TEST_F(DebuggeeThreadTest, ReadRegsForThisThread) {
  CONTEXT zeroed_context;
  memset(&zeroed_context, 0, sizeof(zeroed_context));
  CONTEXT context;
  EXPECT_TRUE(this_thread_->GetContext(&context));
  EXPECT_NE(0, memcmp(&context, &zeroed_context, sizeof(context)));
  const void* zero_ip = 0;
  EXPECT_NE(zero_ip, this_thread_->GetIP());
}

TEST_F(DebuggeeThreadTest, RecvDebugUnicodeStringAndHalt) {
  DEBUG_EVENT wde;
  memset(&wde, 0, sizeof(wde));
  wde.dwDebugEventCode = OUTPUT_DEBUG_STRING_EVENT;
  wde.u.DebugString.fUnicode = 1;

  debug::DebugEvent de;
  de.set_windows_debug_event(wde);
  no_thread_->OnDebugEvent(&de);
  EXPECT_TRUE(no_thread_->IsHalted());
}

TEST_F(DebuggeeThreadTest, RecvDebugStringB) {
  debug::DebugEvent de;
  InitDebugEventWithString("abc", 1, &de);

  fake_thread_->OnDebugEvent(&de);
  EXPECT_TRUE(fake_thread_->IsHalted());
  EXPECT_EQ(debug::DebugEvent::kNotNaClDebugEvent, de.nacl_debug_event_code());
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugString) {
  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);

  fake_thread_->OnDebugEvent(&de);
  EXPECT_TRUE(fake_thread_->IsHalted());
  EXPECT_EQ(debug::DebugEvent::kThreadIsAboutToStart,
            de.nacl_debug_event_code());
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugStringValidateParams) {
  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);
  fake_thread_->OnDebugEvent(&de);

  EXPECT_TRUE(fake_thread_->IsNaClAppThread());
  EXPECT_EQ(kNexeMemBaseAddr, fake_proc_->nexe_mem_base());
  EXPECT_EQ(kNexeEntryPoint, fake_proc_->nexe_entry_point());
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugStringAndContinue) {
  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);

  fake_thread_->OnDebugEvent(&de);

  EXPECT_TRUE(fake_thread_->IsHalted());
  EXPECT_EQ(debug::DebuggeeThread::kHalted, fake_thread_->state());

  fake_thread_->Continue(debug::DebuggeeThread::kContinue);
  EXPECT_FALSE(fake_thread_->IsHalted());
  EXPECT_EQ(debug::DebuggeeThread::kRunning, fake_thread_->state());
  EXPECT_FALSE(fake_debug_api_.single_step_enabled_);
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugStringAndContinueB) {
  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);
  fake_thread_->OnDebugEvent(&de);

  fake_thread_->Continue(debug::DebuggeeThread::kContinueAndPassException);
  EXPECT_FALSE(fake_thread_->IsHalted());
  EXPECT_EQ(debug::DebuggeeThread::kRunning, fake_thread_->state());
  EXPECT_FALSE(fake_debug_api_.single_step_enabled_);
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugStringAndContinueVerifyCallList) {
  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);
  fake_thread_->OnDebugEvent(&de);

  fake_debug_api_.ClearCallSequence();
  fake_thread_->Continue(debug::DebuggeeThread::kContinue);

  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kContinueDebugEvent);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugStringAndContinueVerifyCallListB) {
  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);
  fake_thread_->OnDebugEvent(&de);

  fake_debug_api_.ClearCallSequence();
  fake_thread_->Continue(debug::DebuggeeThread::kContinueAndPassException);

  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kContinueDebugEvent);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugStringAndSingleStep) {
  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);

  fake_thread_->OnDebugEvent(&de);
  EXPECT_TRUE(fake_thread_->IsHalted());

  fake_thread_->Continue(debug::DebuggeeThread::kSingleStep);
  EXPECT_FALSE(fake_thread_->IsHalted());
  EXPECT_TRUE(fake_debug_api_.single_step_enabled_);
}

TEST_F(DebuggeeThreadTest, RecvNexeDebugStringAndSingleStepVerifyCallList) {
  fake_debug_api_.ClearCallSequence();

  debug::DebugEvent de;
  InitDebugEventWithString(kNexeUuid, 2, &de);
  fake_thread_->OnDebugEvent(&de);
  fake_thread_->Continue(debug::DebuggeeThread::kSingleStep);

  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kGetThreadContext);
  call_list.push_back(debug::DebugAPIMock::kSetThreadContext);
  call_list.push_back(debug::DebugAPIMock::kGetThreadContext);
  call_list.push_back(debug::DebugAPIMock::kSetThreadContext);
  call_list.push_back(debug::DebugAPIMock::kContinueDebugEvent);

  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeThreadTest, Kill) {
  TestableDebuggeeThread thread(1, reinterpret_cast<HANDLE>(1), fake_proc_);
  fake_debug_api_.ClearCallSequence();
  thread.Kill();

  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kTerminateThread);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeThreadTest, SetContextVerifyCallList) {
  fake_debug_api_.ClearCallSequence();
  debug::DebuggeeThread thread(1, reinterpret_cast<HANDLE>(1), fake_proc_);

  CONTEXT ct;
  thread.GetContext(&ct);
  thread.SetContext(ct);

  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kGetThreadContext);
  call_list.push_back(debug::DebugAPIMock::kSetThreadContext);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(DebuggeeThreadTest, RecvAlienBreakpoint) {
  DEBUG_EVENT wde;
  memset(&wde, 0, sizeof(wde));
  wde.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
  wde.u.Exception.ExceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;

  debug::DebugEvent de;
  de.set_windows_debug_event(wde);
  fake_thread_->OnDebugEvent(&de);
  EXPECT_TRUE(fake_thread_->IsHalted());
}
}  // namespace

