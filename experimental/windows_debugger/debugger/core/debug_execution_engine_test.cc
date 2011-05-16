// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api_mock.h"
#include "debugger/core/debug_breakpoint.h"
#include "debugger/core/debug_event.h"
#include "debugger/core/debug_execution_engine.h"
#include "debugger/core/debuggee_process.h"

#include "gtest/gtest.h"

namespace {
const int kFakeProcessId = 847;
const int kFakeThreadId = 378421;
const int kFakeThreadId2 = 308421;
HANDLE kFakeThreadHandle = reinterpret_cast<void*>(1);
HANDLE kFakeProcessHandle = reinterpret_cast<void*>(2);
HANDLE kFakeFileHandle = NULL;

const debug::DebuggeeThread* kNullThreadPtr = NULL;
const debug::Breakpoint* kNullBreakpointPtr = NULL;
void* kFakeAddr1 = reinterpret_cast<void*>(0x1B0e37C5);
void* kFakeAddr2 = reinterpret_cast<void*>(0x435F976D);

class TestableDebuggeeProcess : public debug::DebuggeeProcess {
 public:
  TestableDebuggeeProcess(int id,
                          HANDLE handle,
                          HANDLE file_handle,
                          debug::DebugAPI& debug_api)
      : debug::DebuggeeProcess(id,
                               handle,
                               file_handle,
                               debug_api) {
  }

  void OnDebugEvent(debug::DebugEvent* debug_event) {
    debug::DebuggeeProcess::OnDebugEvent(debug_event);
  }
};

class TestExecutionEngine : public debug::ExecutionEngine {
 public:
  TestExecutionEngine(debug::DebugAPI& debug_api)
      : debug::ExecutionEngine(debug_api) {
  }
  virtual debug::DebuggeeProcess* CreateDebuggeeProcess(
      int id,
      HANDLE handle,
      HANDLE file_handle,
      debug::DebugAPI& debug_api) {
    return new TestableDebuggeeProcess(id,
                                       handle,
                                       file_handle,
                                       debug_api);
  }
};

class ExecutionEngineTest : public ::testing::Test {
 public:
  ExecutionEngineTest() {
    exec_eng_ = new debug::ExecutionEngine(fake_debug_api_);
  }

  ~ExecutionEngineTest() {
    delete exec_eng_;
  }

  debug::ExecutionEngine* exec_eng_;
  debug::DebugAPIMock fake_debug_api_;
};

TEST_F(ExecutionEngineTest, SimpleAccessors) {
  debug::DebuggeeProcess* halted_process = NULL;
  EXPECT_FALSE(exec_eng_->DoWork(20, &halted_process));
  EXPECT_EQ(NULL, halted_process);
  EXPECT_EQ(NULL, exec_eng_->GetProcess(1));

  std::deque<int> pids;
  exec_eng_->GetProcessesIds(&pids);
  EXPECT_EQ(0, pids.size());
}

TEST_F(ExecutionEngineTest, StartProcess) {
  EXPECT_TRUE(exec_eng_->StartProcess("a.exe", NULL));

/*
  debug::DebugEvent de;
  memset(&de, 0, sizeof(de));
  de.windows_debug_event_.dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
  de.windows_debug_event_.dwProcessId = proc_->id();
  de.windows_debug_event_.dwThreadId = id;
  proc_->OnDebugEvent(&de);
*/

  std::deque<int> pids;
  exec_eng_->GetProcessesIds(&pids);
  EXPECT_EQ(0, pids.size());
}
}  // namespace

