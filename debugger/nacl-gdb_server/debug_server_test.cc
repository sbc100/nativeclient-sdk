// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <time.h>
#include "debugger/base/debug_socket.h"
#include "debugger/core/debug_api_mock.h"
#include "debugger/core/debuggee_iprocess.h"
#include "debugger/core/debuggee_thread.h"
#include "debugger/nacl-gdb_server/debug_server.h"
#include "debugger/nacl-gdb_server/gdb_registers.h"
#include "debugger/rsp/rsp_packet_utils.h"
#include "gtest/gtest.h"

#pragma warning(disable : 4996)  // Disable sprintf warning.

namespace {
const int kListPort = 4014;
const int kWaitMs = 200;
const char* kNexeThreadCreateMsg =
    "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11} -version 1 "
    "-event ThreadCreate -natp 00000000001CD3F0 ";

const char* kNexeAppStartMsg =
    "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11} -version 1 "
    "-event AppCreate -nap 00000000001CD3F0 "
    "-mem_start 0000000C0000000 "
    "-user_entry_pt 0000000000020080 "
    "-initial_entry_pt 0000000008000080";
void* kEntryAddr = reinterpret_cast<void*>(0xC0000000 + 0x20080);
debug::IDebuggeeProcess* kNullProcPtr =
    reinterpret_cast<debug::IDebuggeeProcess*>(0);

uint32_t kReg1 = 0x1234;
uint32_t kReg2 = 0x0c83;
uint32_t kReg3 = 0x3325;
uint32_t kReg4 = 0xcb8b;
uint32_t kReg5 = 0x51b1;
uint32_t kReg6 = 0xfabc;
uint32_t kReg7 = 0xf120;
uint32_t kReg32_1 = 0x12340c83;
uint32_t kReg32_2 = 0xdc25fabc;
uint32_t kReg32_3 = 0xfabc51b1;
uint32_t kReg32_4 = 0x00000000;
uint32_t kReg32_5 = 0xffffffff;
uint32_t kReg32_6 = 0x28c51248;
uint32_t kReg32_7 = 0xc5623146;
uint32_t kReg32_8 = 0xc1d261c6;
uint32_t kReg32_9 = 0x5725d801;

uint64_t kReg64_1 = 0x56de4816dbb04501;
uint64_t kReg64_2 = 0xdc25fabc3325cb8b;
uint64_t kReg64_3 = 0xfabc51b10d561215;
uint64_t kReg64_4 = 0x0000000061ee2563;
uint64_t kReg64_5 = 0xffffffffdb231782;
uint64_t kReg64_6 = 0x28c51248c4132188;
uint64_t kReg64_7 = 0xc56231468823f1da;
uint64_t kReg64_8 = 0xc1d261c6c2a8037d;
uint64_t kReg64_9 = 0x5725d801563b7240;
uint64_t kReg64_10 = 0x12340c831dd278d5;
uint64_t kReg64_11 = 0x0000000000000000;
uint64_t kReg64_12 = 0xffffffffffffffff;
uint64_t kReg64_13 = 0xfffffff000000000;
uint64_t kReg64_14 = 0x0000ffffffffffff;
uint64_t kReg64_15 = 0xb67e1224af3ced15;
uint64_t kReg64_16 = 0x80bc624e5ff0b346;
uint64_t kReg64_17 = 0xac2cba3363621810;

DEBUG_EVENT CreateWinDebugEvent(int code, int exception_code = 0);
debug::DebugEvent CreateDebugEvent(int code, int exception_code = 0);
void InitDebugEventWithString(const char* str, DEBUG_EVENT* de);
void FillMemoryWithNicePattern(char* buff, size_t buff_len);

#define EXPECT_CSTREQ(s1, s2) \
EXPECT_STREQ(std::string(s1).c_str(), std::string(s2).c_str())

class DebugAPIMock2 : public debug::DebugAPIMock {
 public:
  // Reads a memory of the current process.
  BOOL ReadProcessMemory(HANDLE hProcess,
                         LPCVOID lpBaseAddress,
                         LPVOID lpBuffer,
                         SIZE_T nSize,
                         SIZE_T *lpNumberOfBytesRead);

  // Writes a memory of the current process.
  BOOL WriteProcessMemory(HANDLE hProcess,
                          LPVOID lpBaseAddress,
                          LPCVOID lpBuffer,
                          SIZE_T nSize,
                          SIZE_T *lpNumberOfBytesWritten);

  BOOL GetThreadContext(HANDLE hThread, LPCONTEXT lpContext);
  BOOL SetThreadContext(HANDLE hThread, CONTEXT* lpContext);

  CONTEXT thread_context_;
};

class TestableDebugServer : public debug::DebugServer {
 public:
  explicit TestableDebugServer(debug::DebugAPIMock* api);

  virtual void OnPacket(const debug::Blob& body, bool valid_checksum);
  debug::IDebuggeeProcess* GetFocusedProc();

  void MakeContinueDecision(const debug::DebugEvent& debug_event,
                            bool is_nacl_app_thread,
                            bool* halt,
                            bool* pass_exception);

  debug::IDebuggeeProcess* GetFocusedProcess() {
    return debug::DebugServer::GetFocusedProcess();
  }

  int received_packets_num_;
};

// debug::ListeningSocket test fixture.
class DebugServerTest : public ::testing::Test {
 public:
  DebugServerTest() : srv_(NULL) {}
  ~DebugServerTest();

  bool Init(int port, bool compatibility_mode);
  bool Connect();
  void SetNaclMemBase(void* addr);
  void PostMsg(const char* msg);
  std::string ReceiveReply();
  std::string RPC(const std::string& request);
  std::string RPC(const debug::Blob& request);
  bool StartNaClProc();
  bool InitAndStartNaClProc();
  bool WaitForOneMoreCommand();
  void FillRegistersWithCrap(CONTEXT* ct);
  bool VerifyRegisters(const CONTEXT& ct);
  void AddSecondNaClAppThread(int* id);

  DebugAPIMock2 mock_debug_api_;
  TestableDebugServer* srv_;
  debug::Socket conn_;
};

// Unit tests start here.
TEST_F(DebugServerTest, UnsupportedCommand) {
  ASSERT_TRUE(Connect());
  EXPECT_CSTREQ("", RPC("Z4"));
}

TEST_F(DebugServerTest, LogFileCreated) {
  const char* log_file_name = "nacl-gdb_server_log.txt";
  ::DeleteFile(log_file_name);
  DWORD attr = ::GetFileAttributes(log_file_name);
  ASSERT_EQ(INVALID_FILE_ATTRIBUTES, attr);

  debug::DebugAPIMock api;
  debug::DebugServer srv(&api, kListPort);
  EXPECT_FALSE(srv.ProcessExited());

  // Check that log file is created.
  srv.Init();
  attr = ::GetFileAttributes(log_file_name);
  EXPECT_NE(INVALID_FILE_ATTRIBUTES, attr);
}

TEST_F(DebugServerTest, ListenFails) {
  // Check that |Listen| fails on the busy port
  debug::ListeningSocket listening_socket;
  ASSERT_TRUE(listening_socket.Listen(kListPort));

  debug::DebugAPIMock api;
  debug::DebugServer srv(&api, kListPort);
  EXPECT_FALSE(srv.Init());
}

TEST_F(DebugServerTest, ListenOk) {
  ASSERT_TRUE(Init(kListPort, false));
}

TEST_F(DebugServerTest, ConnectOk) {
  ASSERT_TRUE(Init(kListPort, false));
  debug::Socket conn;
  ASSERT_TRUE(conn.ConnectTo("localhost", kListPort));
  conn.Close();
  ASSERT_TRUE(conn.ConnectTo("localhost", kListPort));
  conn.Close();
  ASSERT_TRUE(conn.ConnectTo("localhost", kListPort));
}

#define STR2BUFF_LEN(str) str, strlen(str)

TEST_F(DebugServerTest, StopReqFails) {
  ASSERT_TRUE(Init(kListPort, false));
  debug::Socket conn;
  ASSERT_TRUE(conn.ConnectTo("localhost", kListPort));
  conn.WriteAll(STR2BUFF_LEN("$?#3f"));

  debug::Blob rcv_data;
  clock_t endwait = clock() + ((10 * kWaitMs * CLOCKS_PER_SEC) / 1000);
  const char* expected_reply = "+$E02#a7";
  while (clock() < endwait) {
    srv_->DoWork(kWaitMs);
    char buff[100];
    size_t rd = conn.Read(buff, sizeof(buff) - 1, 0);
    if (rd) {
      rcv_data.Append(debug::Blob(buff, rd));
      if (rcv_data.size() >= strlen(expected_reply))
        break;
    }
  }
  EXPECT_CSTREQ(expected_reply, rcv_data.ToString());
}

TEST_F(DebugServerTest, StopReqFailsRPC) {
  ASSERT_TRUE(Connect());
  EXPECT_CSTREQ("E02", RPC("?"));
  EXPECT_CSTREQ("E02", RPC("?"));
  EXPECT_CSTREQ("E02", RPC("?"));
}

TEST_F(DebugServerTest, StartProc) {
  ASSERT_TRUE(Connect());
  EXPECT_TRUE(srv_->StartProcess("", NULL));
  std::deque<debug::DebugAPIMock::FunctionId> call_list;
  call_list.push_back(debug::DebugAPIMock::kCreateProcess);
  call_list.push_back(debug::DebugAPIMock::kCloseHandle);
  call_list.push_back(debug::DebugAPIMock::kCloseHandle);
  EXPECT_TRUE(mock_debug_api_.CompareCallSequence(call_list));

  DEBUG_EVENT de = CreateWinDebugEvent(CREATE_PROCESS_DEBUG_EVENT);
  mock_debug_api_.events_.push_back(de);

  srv_->DoWork(0);
  debug::IDebuggeeProcess* proc = srv_->GetFocusedProc();
  EXPECT_EQ(NULL, proc);
  // Nexe is not started yet, there's no focused process.
  EXPECT_CSTREQ("E02", RPC("?"));
}

TEST_F(DebugServerTest, StartNexeProc) {
  ASSERT_TRUE(InitAndStartNaClProc());
  EXPECT_CSTREQ("S13", RPC("?"));
}

TEST_F(DebugServerTest, StartNexeProcAndContinue) {
  ASSERT_TRUE(InitAndStartNaClProc());
  EXPECT_CSTREQ("S13", RPC("?"));
  PostMsg("c");

  ASSERT_TRUE(WaitForOneMoreCommand());
  debug::IDebuggeeProcess* proc = srv_->GetFocusedProc();
  ASSERT_FALSE(proc->IsHalted());
  EXPECT_CSTREQ("O", RPC("?"));
}

TEST_F(DebugServerTest, StartNexeProcAndContinueAndHitBr) {
  ASSERT_TRUE(InitAndStartNaClProc());
  EXPECT_CSTREQ("S13", RPC("?"));
  PostMsg("c");

  ASSERT_TRUE(WaitForOneMoreCommand());
  ASSERT_LE(2, srv_->received_packets_num_);
  debug::IDebuggeeProcess* proc = srv_->GetFocusedProc();
  ASSERT_FALSE(proc->IsHalted());
  EXPECT_CSTREQ("O", RPC("?"));

  DEBUG_EVENT de = CreateWinDebugEvent(EXCEPTION_DEBUG_EVENT,
                                       EXCEPTION_BREAKPOINT);
  mock_debug_api_.events_.push_back(de);
  srv_->DoWork(0);
  EXPECT_CSTREQ("S05", RPC("?"));
}

TEST_F(DebugServerTest, ReadRegs) {
  ASSERT_TRUE(InitAndStartNaClProc());
  FillRegistersWithCrap(&mock_debug_api_.thread_context_);

  std::string reply = RPC("g");
  ASSERT_NE(0, reply.size());

  debug::Blob regs_blob;
  regs_blob.FromHexString(reply);
  CONTEXT ct;
  memset(&ct, 0, sizeof(ct));
  rsp::GdbRegistersToCONTEXT(regs_blob, &ct);
  EXPECT_TRUE(VerifyRegisters(ct));
}

TEST_F(DebugServerTest, WriteRegs) {
  ASSERT_TRUE(InitAndStartNaClProc());
  CONTEXT ct;
  memset(&ct, 0, sizeof(ct));
  FillRegistersWithCrap(&ct);
  debug::Blob regs_blob;
  rsp::CONTEXTToGdbRegisters(ct, &regs_blob);

  std::string write_regs_msg = std::string("G") + regs_blob.ToHexString();
  RPC(write_regs_msg);
  EXPECT_TRUE(VerifyRegisters(mock_debug_api_.thread_context_));
}

TEST_F(DebugServerTest, ReadMemory) {
  ASSERT_TRUE(InitAndStartNaClProc());
  SetNaclMemBase(0);

  char mem[] = "123";
  debug::Blob cmd;
  rsp::Format(&cmd, "m%p,3", mem);
  std::string reply = RPC(cmd.ToString());
  ASSERT_NE(0, reply.size());
  EXPECT_CSTREQ("313233", reply);
}

TEST_F(DebugServerTest, ReadMoreMemory) {
  ASSERT_TRUE(InitAndStartNaClProc());
  SetNaclMemBase(0);

  char some_memory[512];
  FillMemoryWithNicePattern(some_memory, sizeof(some_memory));
  debug::Blob blob(some_memory, sizeof(some_memory));
  std::string expected_reply = blob.ToHexString();

  debug::Blob cmd;
  rsp::Format(&cmd, "m%p,%x", some_memory, sizeof(some_memory));
  EXPECT_CSTREQ(expected_reply, RPC(cmd.ToString()));
}

TEST_F(DebugServerTest, WriteMemory) {
  ASSERT_TRUE(InitAndStartNaClProc());
  SetNaclMemBase(0);

  char some_memory[3];
  memset(some_memory, 0, sizeof(some_memory));

  debug::Blob cmd;
  rsp::Format(&cmd, "M%p,3:313233", some_memory);
  std::string reply = RPC(cmd.ToString());
  EXPECT_CSTREQ("OK", reply);

  debug::Blob blob(some_memory, sizeof(some_memory));
  std::string written_mem = blob.ToString();
  EXPECT_CSTREQ("123", written_mem);
}

TEST_F(DebugServerTest, WriteMoreMemory) {
  ASSERT_TRUE(InitAndStartNaClProc());
  SetNaclMemBase(0);

  char some_memory[256];
  memset(some_memory, 0, sizeof(some_memory));

  char wr_content[sizeof(some_memory)];
  FillMemoryWithNicePattern(wr_content, sizeof(wr_content));
  debug::Blob blob(wr_content, sizeof(wr_content));
  std::string expected_mem = blob.ToHexString();

  debug::Blob cmd;
  rsp::Format(&cmd,
              "M%p,%x:%s",
              some_memory,
              sizeof(some_memory),
              expected_mem.c_str());
  EXPECT_CSTREQ("OK", RPC(cmd.ToString()));

  std::string mem =
      debug::Blob(some_memory, sizeof(some_memory)).ToHexString();
  EXPECT_CSTREQ(expected_mem, mem);
}

TEST_F(DebugServerTest, GetThreadList) {
  int thread1_id = ::GetCurrentThreadId();
  int thread2_id = 0;
  AddSecondNaClAppThread(&thread2_id);

  debug::Blob expected_reply;
  rsp::Format(&expected_reply, "m%x,%x", thread1_id, thread2_id);
  EXPECT_CSTREQ(expected_reply.ToString(), RPC("qfThreadInfo"));
  EXPECT_CSTREQ("l", RPC("qsThreadInfo"));
}

TEST_F(DebugServerTest, GetCurrentThread) {
  ASSERT_TRUE(InitAndStartNaClProc());
  debug::Blob expected_reply;
  rsp::Format(&expected_reply, "QC%x", ::GetCurrentThreadId());
  EXPECT_CSTREQ(expected_reply.ToString(), RPC("qC"));
}

TEST_F(DebugServerTest, SetCurrentThread) {
  int thread2_id = 0;
  AddSecondNaClAppThread(&thread2_id);

  EXPECT_CSTREQ("OK", RPC(rsp::Format(&debug::Blob(), "Hc%x", thread2_id)));
  EXPECT_CSTREQ("E03",
                RPC(rsp::Format(&debug::Blob(), "Hc%x", thread2_id + 100)));

  debug::Blob expected_reply;
  rsp::Format(&expected_reply, "QC%x", thread2_id);
  EXPECT_CSTREQ(expected_reply.ToString(), RPC("qC"));
}

TEST_F(DebugServerTest, IsThreadAlive) {
  int thread2_id = 0;
  AddSecondNaClAppThread(&thread2_id);
  debug::Blob cmd;
  EXPECT_CSTREQ("OK", RPC(rsp::Format(&cmd, "T%x", thread2_id)));
  EXPECT_CSTREQ("E0a", RPC(rsp::Format(&cmd, "T%x", thread2_id + 100)));
}

// dbg_code, excpt_code, is_nacl_app_thread -> halt, pass_exception
struct MakeContinueDecisionTableEntry {
  int dbg_code_;
  int excpt_code_;
  int nacl_dbg_code_;
  bool is_nacl_app_thread_;
  bool halt_;
  bool pass_exception_;
};

TEST_F(DebugServerTest, MakeContinueDecision1) {
// These two macros are needed to make the table look good,
// with lines < 80 chars.
#define EX(x) EXCEPTION_##x
#define DE(x) x##_DEBUG_EVENT
  MakeContinueDecisionTableEntry table[] = {
    {DE(EXCEPTION), EX(ACCESS_VIOLATION), 0, false, false, true},
    {DE(EXCEPTION), EX(ACCESS_VIOLATION), 0, true, true, true},
    {DE(EXCEPTION), EX(ILLEGAL_INSTRUCTION), 0, false, false, true},
    {DE(EXCEPTION), EX(ILLEGAL_INSTRUCTION), 0, true, true, true},
    {DE(EXCEPTION), EX(ARRAY_BOUNDS_EXCEEDED), 0, false, false, true},
    {DE(EXCEPTION), EX(ARRAY_BOUNDS_EXCEEDED), 0, true, true, true},
    {DE(EXCEPTION), EX(BREAKPOINT), 0, false, false, false},
    {DE(EXCEPTION), EX(BREAKPOINT), 0, true, true, false},
    {DE(EXCEPTION), EX(SINGLE_STEP), 0, false, false, true},
    {DE(EXCEPTION), EX(SINGLE_STEP), 0, true, true, false},
    {DE(CREATE_THREAD), 0,  0, false, false, false},
    {DE(CREATE_THREAD), 0,  0, true, false, false},
    {DE(CREATE_PROCESS), 0,  0, false, false, false},
    {DE(CREATE_PROCESS), 0,  0, true, false, false},
    {DE(EXIT_THREAD), 0,  0, false, false, false},
    {DE(EXIT_THREAD), 0,  0, true, true, false},
    {DE(EXIT_PROCESS), 0,  0, false, false, false},
    {DE(EXIT_PROCESS), 0,  0, true, false, false},
    {DE(LOAD_DLL), 0,  0, false, false, false},
    {DE(LOAD_DLL), 0,  0, true, false, false},
    {DE(UNLOAD_DLL), 0,  0, false, false, false},
    {DE(UNLOAD_DLL), 0,  0, true, false, false},
    {OUTPUT_DEBUG_STRING_EVENT, 0,  0, false, false, false},
    {OUTPUT_DEBUG_STRING_EVENT, 0,  0, true, false, false},
    {OUTPUT_DEBUG_STRING_EVENT, 0,  1, true, true, false},
    {OUTPUT_DEBUG_STRING_EVENT, 0,  2, true, false, false},
  };
#undef EX
#undef DE

  for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
    debug::DebugEvent de = CreateDebugEvent(table[i].dbg_code_,
                                            table[i].excpt_code_);
    de.set_nacl_debug_event_code(
        static_cast<debug::DebugEvent::NaClDebugEventCode>(
            table[i].nacl_dbg_code_));
    bool halt = false;
    bool pass_exception = false;
    srv_->TestableDebugServer::MakeContinueDecision(
        de,
        table[i].is_nacl_app_thread_,
        &halt,
        &pass_exception);
    EXPECT_EQ(table[i].halt_, halt) << i;
    if (!halt && (EXCEPTION_DEBUG_EVENT == table[i].dbg_code_)) {
      EXPECT_EQ(table[i].pass_exception_, pass_exception) << i;
    }
  }
}

TEST_F(DebugServerTest, CompatibilityMode) {
  ASSERT_TRUE(Init(kListPort, true));
  EXPECT_FALSE(conn_.ConnectTo("localhost", kListPort));
  EXPECT_TRUE(StartNaClProc());

  DEBUG_EVENT de = CreateWinDebugEvent(EXCEPTION_DEBUG_EVENT,
                                       EXCEPTION_BREAKPOINT);
  de.u.Exception.ExceptionRecord.ExceptionAddress = kEntryAddr;
  mock_debug_api_.events_.push_back(de);
  srv_->DoWork(0);

  EXPECT_TRUE(conn_.ConnectTo("localhost", kListPort));
  EXPECT_CSTREQ("S05", RPC("?"));
}

/////////////////////////////////////////////////////////
// Implementation of helper functions.
/////////////////////////////////////////////////////////
void DebugServerTest::AddSecondNaClAppThread(int* id) {
  ASSERT_TRUE(InitAndStartNaClProc());
  PostMsg("c");
  ASSERT_TRUE(WaitForOneMoreCommand());

  // Simulate another NaClAppThread creation
  int thread1_id = ::GetCurrentThreadId();
  int thread2_id = thread1_id + 10;
  DEBUG_EVENT de = CreateWinDebugEvent(CREATE_THREAD_DEBUG_EVENT);
  de.dwThreadId = thread2_id;
  mock_debug_api_.events_.push_back(de);

  InitDebugEventWithString(kNexeThreadCreateMsg, &de);
  de.dwThreadId = thread2_id;
  mock_debug_api_.events_.push_back(de);
  srv_->DoWork(0);

  // Now, nacl-gdb_server should send 'S13' message (debuggee
  // process got SIGSTOP signal).
  EXPECT_CSTREQ("S13", ReceiveReply());
  *id = thread2_id;
}

void DebugServerTest::FillRegistersWithCrap(CONTEXT* ct) {
  memset(ct, 0, sizeof(*ct));
  ct->EFlags = kReg1;
  ct->SegCs = kReg2;
  ct->SegSs = kReg3;
  ct->SegDs = kReg4;
  ct->SegEs = kReg5;
  ct->SegFs = kReg6;
  ct->SegGs = kReg7;

#ifdef _WIN64
  ct->Rax = kReg64_1;
  ct->Rbx = kReg64_2;
  ct->Rcx = kReg64_3;
  ct->Rdx = kReg64_4;
  ct->Rsi = kReg64_5;
  ct->Rdi = kReg64_6;
  ct->Rbp = kReg64_7;
  ct->Rsp = kReg64_8;
  ct->R8 = kReg64_9;
  ct->R9 = kReg64_10;
  ct->R10 = kReg64_11;
  ct->R11 = kReg64_12;
  ct->R12 = kReg64_13;
  ct->R13 = kReg64_14;
  ct->R14 = kReg64_15;
  ct->R15 = kReg64_16;
  ct->Rip = kReg64_17;

#else
  ct->Eax = kReg32_1;
  ct->Ecx = kReg32_2;
  ct->Edx = kReg32_3;
  ct->Ebx = kReg32_4;
  ct->Esp = kReg32_5;
  ct->Ebp = kReg32_6;
  ct->Esi = kReg32_7;
  ct->Edi = kReg32_8;
  ct->Eip = kReg32_9;
#endif
}

bool DebugServerTest::VerifyRegisters(const CONTEXT& ct) {
#define MY_EXPECT_EQ(a, b) EXPECT_EQ(a, b); if ((a) != (b)) result = false
  bool result = true;
#ifdef _WIN64
  MY_EXPECT_EQ(kReg64_1, ct.Rax);
  MY_EXPECT_EQ(kReg64_2, ct.Rbx);
  MY_EXPECT_EQ(kReg64_3, ct.Rcx);
  MY_EXPECT_EQ(kReg64_4, ct.Rdx);
  MY_EXPECT_EQ(kReg64_5, ct.Rsi);
  MY_EXPECT_EQ(kReg64_6, ct.Rdi);
  MY_EXPECT_EQ(kReg64_7, ct.Rbp);
  MY_EXPECT_EQ(kReg64_8, ct.Rsp);
  MY_EXPECT_EQ(kReg64_9, ct.R8);
  MY_EXPECT_EQ(kReg64_10, ct.R9);
  MY_EXPECT_EQ(kReg64_11, ct.R10);
  MY_EXPECT_EQ(kReg64_12, ct.R11);
  MY_EXPECT_EQ(kReg64_13, ct.R12);
  MY_EXPECT_EQ(kReg64_14, ct.R13);
  MY_EXPECT_EQ(kReg64_15, ct.R14);
  MY_EXPECT_EQ(kReg64_16, ct.R15);
  MY_EXPECT_EQ(kReg64_17, ct.Rip);
#else
  MY_EXPECT_EQ(kReg32_1, ct.Eax);
  MY_EXPECT_EQ(kReg32_2, ct.Ecx);
  MY_EXPECT_EQ(kReg32_3, ct.Edx);
  MY_EXPECT_EQ(kReg32_4, ct.Ebx);
  MY_EXPECT_EQ(kReg32_5, ct.Esp);
  MY_EXPECT_EQ(kReg32_6, ct.Ebp);
  MY_EXPECT_EQ(kReg32_7, ct.Esi);
  MY_EXPECT_EQ(kReg32_8, ct.Edi);
  MY_EXPECT_EQ(kReg32_9, ct.Eip);
#endif

  MY_EXPECT_EQ(kReg2, ct.SegCs);
  MY_EXPECT_EQ(kReg1, ct.EFlags);
  MY_EXPECT_EQ(kReg3, ct.SegSs);
  MY_EXPECT_EQ(kReg4, ct.SegDs);
  MY_EXPECT_EQ(kReg5, ct.SegEs);
  MY_EXPECT_EQ(kReg6, ct.SegFs);
  MY_EXPECT_EQ(kReg7, ct.SegGs);

  return result;
#undef MY_EXPECT_EQ
}

DEBUG_EVENT CreateWinDebugEvent(int code, int exception_code) {
  DEBUG_EVENT de;
  memset(&de, 0, sizeof(de));
  de.dwDebugEventCode = code;
  de.u.Exception.ExceptionRecord.ExceptionCode = exception_code;
  de.dwThreadId = ::GetCurrentThreadId();
  de.dwProcessId = ::GetCurrentProcessId();
  return de;
}

debug::DebugEvent CreateDebugEvent(int code, int exception_code) {
  debug::DebugEvent de;
  de.set_windows_debug_event(CreateWinDebugEvent(code, exception_code));
  return de;
}

void InitDebugEventWithString(const char* str, DEBUG_EVENT* de) {
  memset(de, 0, sizeof(*de));
  *de = CreateWinDebugEvent(OUTPUT_DEBUG_STRING_EVENT);
  de->u.DebugString.lpDebugStringData = const_cast<char*>(str);
  de->u.DebugString.nDebugStringLength = static_cast<WORD>(strlen(str));
}

void FillMemoryWithNicePattern(char* buff, size_t buff_len) {
  const char* kCrap = "cfb8o271415r47s62d5d29d42953112612d";
  memcpy(buff, kCrap, min(strlen(kCrap), buff_len));
  for (size_t i = 0; i < buff_len; i++)
    if (0 != buff[i])
      buff[i] = i & 0xFF;
}

BOOL DebugAPIMock2::ReadProcessMemory(HANDLE hProcess,
                                      LPCVOID lpBaseAddress,
                                      LPVOID lpBuffer,
                                      SIZE_T nSize,
                                      SIZE_T *lpNumberOfBytesRead) {
  debug::DebugAPIMock::ReadProcessMemory(hProcess,
                                         lpBaseAddress,
                                         lpBuffer,
                                         nSize,
                                         lpNumberOfBytesRead);
  if ((NULL != lpBuffer) &&
     (NULL != lpBaseAddress) &&
     (kEntryAddr != lpBaseAddress))
    memcpy(lpBuffer, lpBaseAddress, nSize);
  return TRUE;
}

BOOL DebugAPIMock2::WriteProcessMemory(HANDLE hProcess,
                                       LPVOID lpBaseAddress,
                                       LPCVOID lpBuffer,
                                       SIZE_T nSize,
                                       SIZE_T *lpNumberOfBytesWritten) {
  debug::DebugAPIMock::WriteProcessMemory(hProcess,
                                          lpBaseAddress,
                                          lpBuffer,
                                          nSize,
                                          lpNumberOfBytesWritten);
  if ((NULL != lpBuffer) &&
     (NULL != lpBaseAddress) &&
     (kEntryAddr != lpBaseAddress))
  memcpy(lpBaseAddress, lpBuffer, nSize);
  return TRUE;
}

BOOL DebugAPIMock2::GetThreadContext(HANDLE hThread, LPCONTEXT lpContext) {
  debug::DebugAPIMock::GetThreadContext(hThread, lpContext);
  memcpy(lpContext, &thread_context_, sizeof(*lpContext));
  return TRUE;
}

BOOL DebugAPIMock2::SetThreadContext(HANDLE hThread, CONTEXT* lpContext) {
  debug::DebugAPIMock::SetThreadContext(hThread, lpContext);
  memcpy(&thread_context_, lpContext, sizeof(thread_context_));
  return TRUE;
}

TestableDebugServer::TestableDebugServer(debug::DebugAPIMock* api)
    : debug::DebugServer(api, kListPort),
      received_packets_num_(0) {
}

void TestableDebugServer::OnPacket(const debug::Blob& body,
                                   bool valid_checksum) {
  debug::DebugServer::OnPacket(body, valid_checksum);
  received_packets_num_++;
  assert(valid_checksum);
}

debug::IDebuggeeProcess* TestableDebugServer::GetFocusedProc() {
  return execution_engine_->GetProcess(focused_process_id_);
}

void TestableDebugServer::MakeContinueDecision(
    const debug::DebugEvent& debug_event,
    bool is_nacl_app_thread,
    bool* halt,
    bool* pass_exception) {
  return debug::DebugServer::MakeContinueDecision(debug_event,
                                                  is_nacl_app_thread,
                                                  halt,
                                                  pass_exception);
}

DebugServerTest::~DebugServerTest() {
  if (NULL != srv_)
    delete srv_;
}

bool DebugServerTest::Init(int port, bool compatibility_mode) {
  srv_ = new TestableDebugServer(&mock_debug_api_);
  if (NULL == srv_)
    return false;

  if (compatibility_mode)
    srv_->EnableCompatibilityMode();

  return srv_->Init();
}

bool DebugServerTest::Connect() {
  if (!Init(kListPort, false))
    return false;
  return conn_.ConnectTo("localhost", kListPort);
}

void DebugServerTest::SetNaclMemBase(void* addr) {
  debug::IDebuggeeProcess* proc = srv_->GetFocusedProcess();
  if (NULL != proc)
    proc->set_nexe_mem_base(addr);
}

void DebugServerTest::PostMsg(const char* msg) {
  debug::Blob wire_req;
  rsp::PacketUtils::AddEnvelope(debug::Blob().FromString(msg), &wire_req);
  conn_.WriteAll(wire_req);
}

std::string DebugServerTest::ReceiveReply() {
  debug::Blob rcv;
  debug::Blob wire_rcv;
  clock_t endwait = clock() + ((10 * kWaitMs * CLOCKS_PER_SEC) / 1000);
  while (clock() < endwait) {
    srv_->DoWork(kWaitMs);
    char buff[100];
    size_t rd = conn_.Read(buff, sizeof(buff) - 1, 0);
    if (rd) {
      wire_rcv.Append(debug::Blob(buff, rd));
      if (rsp::PacketUtils::RemoveEnvelope(wire_rcv, &rcv)) {
        break;
      }
    }
  }
  srv_->DoWork(kWaitMs);
  return rcv.ToString();
}

std::string DebugServerTest::RPC(const std::string& request) {
  PostMsg(request.c_str());
  return ReceiveReply();
}

std::string DebugServerTest::RPC(const debug::Blob& request) {
  return RPC(request.ToString());
}

bool DebugServerTest::InitAndStartNaClProc() {
  if (!Connect())
    return false;
  return StartNaClProc();
}

bool DebugServerTest::StartNaClProc() {
  if (!srv_->StartProcess("", NULL))
    return false;
  DEBUG_EVENT wde1 = CreateWinDebugEvent(CREATE_PROCESS_DEBUG_EVENT);
  mock_debug_api_.events_.push_back(wde1);
  srv_->DoWork(0);

  DEBUG_EVENT wde2;
  InitDebugEventWithString(kNexeAppStartMsg, &wde2);
  mock_debug_api_.events_.push_back(wde2);
  srv_->DoWork(0);

  DEBUG_EVENT wde3;
  InitDebugEventWithString(kNexeThreadCreateMsg, &wde3);
  mock_debug_api_.events_.push_back(wde3);
  srv_->DoWork(0);
  return true;
}

bool DebugServerTest::WaitForOneMoreCommand() {
  int num = srv_->received_packets_num_;
  for (int i = 0; i < 1000; i++) {
    srv_->DoWork(20);
    if (srv_->received_packets_num_ >= (num + 1))
      return true;
  }
  return false;
}
}  // namespace

