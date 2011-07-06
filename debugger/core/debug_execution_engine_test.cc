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
const debug::IDebuggeeProcess* kNullProcess =
  reinterpret_cast<debug::IDebuggeeProcess*>(NULL);

class TestableDebuggeeProcess : public debug::DebuggeeProcess {
 public:
  TestableDebuggeeProcess(int id,
                          HANDLE handle,
                          HANDLE file_handle,
                          debug::DebugAPI* debug_api)
      : debug::DebuggeeProcess(id,
                               handle,
                               file_handle,
                               debug_api),
        killed_(false) {
  }

  bool Kill() {
    killed_ = true;
    state_ = kDead;
    return true;
  }

  void OnDebugEvent(debug::DebugEvent* debug_event) {
    debug::DebuggeeProcess::OnDebugEvent(debug_event);
    if (killed_)
      state_ = kDead;
  }

  bool killed_;
};

class TestExecutionEngine : public debug::ExecutionEngine {
 public:
  explicit TestExecutionEngine(debug::DebugAPI* debug_api)
      : debug::ExecutionEngine(debug_api) {
  }

  debug::IDebuggeeProcess* CreateDebuggeeProcess(int id,
                                                 HANDLE handle,
                                                 HANDLE file_handle) {
    return new TestableDebuggeeProcess(id,
                                       handle,
                                       file_handle,
                                       &debug_api());
  }
  size_t GetProcessesNum() const {
    std::deque<int> processes;
    GetProcessIds(&processes);
    return processes.size();
  }
};

class ExecutionEngineTest : public ::testing::Test {
 public:
  ExecutionEngineTest() {
    exec_eng_ = new TestExecutionEngine(&fake_debug_api_);
  }

  ~ExecutionEngineTest() {
    delete exec_eng_;
  }

  TestExecutionEngine* exec_eng_;
  debug::DebugAPIMock fake_debug_api_;
};

TEST_F(ExecutionEngineTest, SimpleAccessors) {
  int halted_pid = 0;
  EXPECT_FALSE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(0, halted_pid);
  EXPECT_EQ(NULL, exec_eng_->GetProcess(1));

  std::deque<int> pids;
  exec_eng_->GetProcessIds(&pids);
  EXPECT_EQ(0, pids.size());
}

TEST_F(ExecutionEngineTest, StartProcess) {
  EXPECT_TRUE(exec_eng_->StartProcess("a.exe", NULL));

  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kCreateProcess);
  call_list.push_back(debug::DebugAPIMock::kCloseHandle);
  call_list.push_back(debug::DebugAPIMock::kCloseHandle);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(ExecutionEngineTest, AttachToProcess) {
  EXPECT_TRUE(exec_eng_->AttachToProcess(0));
  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kDebugActiveProcess);
  EXPECT_TRUE(fake_debug_api_.CompareCallSequence(call_list));
}

TEST_F(ExecutionEngineTest, WaitForDebugEventAndDispatchIt) {
  int halted_pid = 0;
  EXPECT_FALSE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(0, halted_pid);
}

TEST_F(ExecutionEngineTest, CreateAndExit) {
  DEBUG_EVENT de;
  memset(&de, 0, sizeof(de));
  de.dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
  de.dwProcessId = 1;
  de.dwThreadId = 2;
  fake_debug_api_.events_.push_back(de);

  DEBUG_EVENT de2;
  memset(&de2, 0, sizeof(de2));
  de2.dwProcessId = 1;
  de2.dwThreadId = 2;
  de2.dwDebugEventCode = EXIT_PROCESS_DEBUG_EVENT;
  fake_debug_api_.events_.push_back(de2);

  int halted_pid = 0;
  EXPECT_TRUE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(1, halted_pid);
  DEBUG_EVENT* de3 = &exec_eng_->debug_event().windows_debug_event();
  EXPECT_EQ(0, memcmp(&de, de3, sizeof(de)));
  EXPECT_NE(kNullProcess, exec_eng_->GetProcess(1));

  EXPECT_TRUE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(1, halted_pid);

  EXPECT_EQ(1, exec_eng_->GetProcessesNum());
  exec_eng_->GetProcess(1)->Continue();
  EXPECT_EQ(1, exec_eng_->GetProcessesNum());
  EXPECT_FALSE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(0, exec_eng_->GetProcessesNum());
}

TEST_F(ExecutionEngineTest, GetProcesses) {
  DEBUG_EVENT de;
  memset(&de, 0, sizeof(de));
  de.dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
  de.dwProcessId = 1;
  de.dwThreadId = 2;
  fake_debug_api_.events_.push_back(de);

  de.dwProcessId = 3;
  de.dwThreadId = 4;
  fake_debug_api_.events_.push_back(de);

  DEBUG_EVENT de2;
  memset(&de2, 0, sizeof(de2));
  de2.dwProcessId = 1;
  de2.dwThreadId = 2;
  de2.dwDebugEventCode = EXIT_PROCESS_DEBUG_EVENT;
  fake_debug_api_.events_.push_back(de2);

  int halted_pid = NULL;
  EXPECT_TRUE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(1, halted_pid);

  EXPECT_TRUE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(3, halted_pid);

  std::deque<int> processes;
  exec_eng_->GetProcessIds(&processes);
  ASSERT_EQ(2, processes.size());
  EXPECT_EQ(1, exec_eng_->GetProcess(processes[0])->id());
  EXPECT_EQ(3, exec_eng_->GetProcess(processes[1])->id());

  // exit process pid=1
  EXPECT_TRUE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_EQ(1, halted_pid);

  debug::IDebuggeeProcess* proc1 = exec_eng_->GetProcess(1);
  debug::IDebuggeeProcess* proc2 = exec_eng_->GetProcess(3);

  EXPECT_EQ(2, exec_eng_->GetProcessesNum());
  exec_eng_->GetProcess(1)->Continue();
  EXPECT_EQ(2, exec_eng_->GetProcessesNum());

  EXPECT_FALSE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
//  exec_eng_->RemoveDeadProcesses();
  EXPECT_EQ(1, exec_eng_->GetProcessesNum());
}

TEST_F(ExecutionEngineTest, HasAliveDebuggee) {
  EXPECT_FALSE(exec_eng_->HasAliveDebuggee());

  DEBUG_EVENT de;
  memset(&de, 0, sizeof(de));
  de.dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
  de.dwProcessId = 1;
  de.dwThreadId = 2;
  fake_debug_api_.events_.push_back(de);

  int halted_pid = NULL;
  EXPECT_TRUE(exec_eng_->WaitForDebugEventAndDispatchIt(20, &halted_pid));
  EXPECT_TRUE(exec_eng_->HasAliveDebuggee());
}

}  // namespace

