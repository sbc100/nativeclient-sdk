// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/nacl-gdb_server/debug_server.h"
#include <assert.h>
#include <algorithm>
#include "debugger/core/debug_logger.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debuggee_thread.h"
#include "debugger/nacl-gdb_server/gdb_registers.h"
#include "debugger/nacl-gdb_server/rsp_stop_from_debug_event.h"
#include "debugger/rsp/rsp_common_replies.h"
#include "debugger/rsp/rsp_control_packets.h"
#include "debugger/rsp/rsp_info_packets.h"
#include "debugger/rsp/rsp_packet_utils.h"
#include "debugger/rsp/rsp_packets.h"
#include "debugger/rsp/rsp_threads_packets.h"

namespace {
const int kReadBufferSize = 1024;
const int kVS2008_THREAD_INFO = 0x406D1388;
const int kStopTimeoutMs = 1000;
const int kMaxPacketsToReadAtOnce = 100;

// Error codes returned to the client (debugger).
const int kErrorNoFocusedThread = 1;
const int kErrorNoFocusedProcess = 2;
const int kErrorSetFocusToAllThreadsIsNotSupported = 3;
const int kErrorReadMemoryFailed = 4;
const int kErrorPacketIsTooLarge = 5;
const int kErrorWriteMemoryFailed = 6;
const int kErrorGetThreadContextFailed = 7;
const int kErrorSetThreadContextFailed = 8;
const int kErrorSingleStepFailed = 9;
const int kErrorThreadIsDead = 10;
}  // namespace

namespace debug {
DebugServer::DebugServer(DebugAPI* debug_api)
    : debug_api_(*debug_api),
      logger_(NULL),
      state_(kIdle),
      client_connected_(false),
      execution_engine_(NULL),
      focused_process_id_(0),
      focused_thread_id_(0) {
}

DebugServer::~DebugServer() {
  if (NULL != execution_engine_)
    delete execution_engine_;

  if (NULL != logger_)
    delete logger_;
}

bool DebugServer::Init() {
  rsp_packetizer_.SetPacketConsumer(this);
  execution_engine_ = new ExecutionEngine(&debug_api_);

  debug::TextFileLogger* log = new debug::TextFileLogger();
  log->Open("nacl-gdb_server_log.txt");
  log->EnableStdout(true);
  logger_ = log;
  return (NULL != execution_engine_);
}

bool DebugServer::ListenOnPort(int port) {
  bool res = listening_socket_.Listen(port);
  if (res)
    logger_->Log("TR100.1", "Started listening on port %d ...\n", port);
  return res;
}

bool DebugServer::StartProcess(const char* cmd, const char* work_dir) {
  bool res = execution_engine_->StartProcess(cmd, work_dir);
  if (res) {
    state_ = kStarting;
    logger_->Log("TR100.2", "Starting process [%s]...\n", cmd);
  }
  return res;
}

void DebugServer::HandleNetwork() {
  if (!client_connection_.IsConnected()) {
    if (client_connected_) {
      client_connected_ = false;
      logger_->Log("TR100.3", "Dropped connection from debugger.\n");
    }
    if (listening_socket_.Accept(0, &client_connection_)) {
      client_connected_ = true;
      logger_->Log("TR100.4", "Got connection from debugger.\n");
    }
  } else {
    char buff[kReadBufferSize];
    for (int i = 0; i < kMaxPacketsToReadAtOnce; i++) {
      size_t read_bytes = client_connection_.Read(buff,
                                                  sizeof(buff) - 1,
                                                  0);
      if (read_bytes > 0) {
        buff[read_bytes] = 0;
        logger_->Log("r>", "[%s]\n", buff);
        rsp_packetizer_.OnData(buff, read_bytes);
      } else {
        break;
      }
    }
  }
}

bool DebugServer::ProcessExited() const {
  return kExiting == state_;
}

void DebugServer::HandleExecutionEngine(int wait_ms) {
  if (kExiting == state_)
    return;
  if ((kStarting == state_) && execution_engine_->HasAliveDebuggee())
    state_ = kRunning;
  if ((kRunning == state_) && !execution_engine_->HasAliveDebuggee()) {
    printf("Exiting: no alive debuggee.\n");
    Quit();
    state_ = kExiting;
  }
  int pid = 0;
  if (!execution_engine_->WaitForDebugEventAndDispatchIt(wait_ms, &pid))
    return;

  IDebuggeeProcess* halted_process = execution_engine_->GetProcess(pid);
  if (NULL != halted_process) {
    DebugEvent de = execution_engine_->debug_event();
    DEBUG_EVENT wde = de.windows_debug_event();
    DebuggeeThread* halted_thread = halted_process->GetHaltedThread();

    if ((OUTPUT_DEBUG_STRING_EVENT == wde.dwDebugEventCode) &&
       (DebugEvent::kNotNaClDebugEvent != de.nacl_debug_event_code())) {
      std::string str;
      if (halted_process->ReadDebugString(&str))
        logger_->Log("OutputDebugString",
                     "[%s] pid=%d tid=%d\n",
                     str.c_str(),
                     wde.dwProcessId,
                     wde.dwThreadId);
    }
    if (debug::DebugEvent::kThreadIsAboutToStart ==
        de.nacl_debug_event_code()) {
      if ((NULL != halted_thread) && halted_thread->IsNaClAppThread())
        logger_->Log("TR100.5",
                    "NaClThreadStart mem_base=%p entry_point=%p thread_id=%d\n",
                    halted_process->nexe_mem_base(),
                    halted_process->nexe_entry_point(),
                    halted_thread->id());
    }
    bool halt_debuggee = false;
    bool pass_exception = true;
    MakeContinueDecision(de,
                         halted_thread->IsNaClAppThread(),
                         &halt_debuggee,
                         &pass_exception);
    if (halt_debuggee)
      OnHaltedProcess(halted_process, de);
    else if (pass_exception)
      halted_process->ContinueAndPassExceptionToDebuggee();
    else
      halted_process->Continue();
  }
}

void DebugServer::DoWork(int wait_ms) {
  HandleNetwork();
  HandleExecutionEngine(wait_ms);
}

void DebugServer::Quit() {
  execution_engine_->Stop(kStopTimeoutMs);
  listening_socket_.Close();
  client_connection_.Close();
}

void DebugServer::OnHaltedProcess(IDebuggeeProcess* halted_process,
                                  const DebugEvent& debug_event) {
  char tmp[1000];
  debug_event.ToString(tmp, sizeof(tmp));
  logger_->Log("TR100.6",
              "Halted: pid=%d tid=%d\ndebugevent=%s\n",
              halted_process->id(),
              halted_process->GetHaltedThread()->id(),
              tmp);

  assert(halted_process->GetHaltedThread()->IsNaClAppThread());

  if (0 == focused_process_id_)
    focused_process_id_ = halted_process->id();
  focused_thread_id_ = halted_process->GetHaltedThread()->id();

  if (client_connection_.IsConnected())
    SendRspMessageToClient(rsp::CreateStopReplyPacket(debug_event));
}

void DebugServer::SendRspMessageToClient(const rsp::Packet& msg) {
  Blob text;
  msg.ToBlob(&text);
  Blob wire_msg;
  rsp::PacketUtils::AddEnvelope(text, &wire_msg);
  client_connection_.WriteAll(wire_msg);
  logger_->Log("T>", "[%s]\n", text.ToString().c_str());
  logger_->Log("t>", "[%s]\n", wire_msg.ToString().c_str());
}

IDebuggeeProcess* DebugServer::GetFocusedProcess() {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc)
    SendErrorReply(kErrorNoFocusedProcess);
  return proc;
}

DebuggeeThread* DebugServer::GetFocusedThread() {
  DebuggeeThread* thread = NULL;
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL != proc) {
    thread = proc->GetThread(focused_thread_id_);
    if (NULL == thread)
      SendErrorReply(kErrorNoFocusedThread);
  }
  return thread;
}

void DebugServer::OnUnknownCommand() {
  // Empty RSP packet is returned for unsupported commands.
  SendRspMessageToClient(rsp::EmptyPacket());
}

void DebugServer::OnPacket(const Blob& body, bool valid_checksum) {
  if (valid_checksum)
    client_connection_.WriteAll("+", 1);  // Send low-level RSP acknowledgment.

  Blob msg = body;
  rsp::Packet* command = rsp::Packet::CreateFromBlob(&msg, NULL);
  if (NULL != command) {
    command->AcceptVisitor(this);
    delete command;
  } else {
    OnUnknownCommand();
  }
}

void DebugServer::OnUnexpectedByte(uint8_t unexpected_byte) {
  logger_->Log("WARN21.01",
               "msg='DebugServer::OnUnexpectedChar' c='0x%x'",
               static_cast<int>(unexpected_byte));
}

void DebugServer::SendErrorReply(int error) {
  SendRspMessageToClient(rsp::ErrorReply(error));
}

void DebugServer::OnBreak() {
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL != proc)
    proc->Break();
}

void DebugServer::Visit(rsp::GetStopReasonCommand* packet) {
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL == proc)
    return;

  DebugEvent debug_event = proc->last_debug_event();
  DEBUG_EVENT wde = debug_event.windows_debug_event();
  if (proc->IsHalted()) {
    SendRspMessageToClient(rsp::CreateStopReplyPacket(debug_event));
  } else if (DebuggeeProcess::kDead == proc->state()) {
    SendErrorReply(kErrorNoFocusedProcess);
  } else  {  // Process is running.
    SendRspMessageToClient(rsp::StopReply(rsp::StopReply::STILL_RUNNING));
  }
}

void DebugServer::Visit(rsp::ContinueCommand* packet) {
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL != proc)
    proc->Continue();
}

void DebugServer::Visit(rsp::QuerySupportedCommand* packet) {
  rsp::QuerySupportedReply reply;
  reply.AddFeature("PacketSize", rsp::kMaxRspPacketSizeStr);
  reply.AddFeature("qXfer:libraries:read", "+");
  reply.AddFeature("qXfer:features:read", "+");
  SendRspMessageToClient(reply);
}

void DebugServer::Visit(rsp::QXferFeaturesReadCommand* packet) {
  // qXfer:features:read:target.xml:0,7ca
  if (packet->file_name() == "target.xml") {
    rsp::QXferReply reply;
#ifdef _WIN64
    reply.set_body("<target><architecture>i386:x86-64</architecture></target>");
#elif defined(_WIN32)
    reply.set_body("<target><architecture>i386:x86-32</architecture></target>");
#else
  #error
#endif
    reply.set_eom(true);
    SendRspMessageToClient(reply);
  } else {
    OnUnknownCommand();
  }
}

void DebugServer::Visit(rsp::SetCurrentThreadCommand* packet) {
  int tid = static_cast<int>(packet->thread_id());
  bool res = false;
  if (-1 == tid) {  // all threads
    res = false;
  } else if (0 == tid) {  // any thread
    res = true;
  } else {
    IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
    if (NULL != proc) {
      DebuggeeThread* thread = proc->GetThread(tid);
      if (NULL != thread) {
        res = true;
        focused_thread_id_ = tid;
      }
    }
  }
  if (res)
    SendRspMessageToClient(rsp::OkReply());
  else
    SendErrorReply(kErrorSetFocusToAllThreadsIsNotSupported);
}

void DebugServer::Visit(rsp::ReadMemoryCommand* packet) {
  int sz = packet->num_of_bytes();
  if (0 == sz) {
    SendRspMessageToClient(rsp::EmptyPacket());
    return;
  }
  char buff[rsp::kMaxRspPacketSize / 2];  // 2 characters per byte.
  if (sizeof(buff) < sz)
    sz = sizeof(buff);

  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL != proc) {
    char* addr = reinterpret_cast<char*>(packet->addr());
    // massage address to support gdb
    if (addr < proc->nexe_mem_base())
      addr += reinterpret_cast<size_t>(proc->nexe_mem_base());

    if (proc->ReadMemory(addr, sz, buff)) {
      rsp::BlobReply reply;
      reply.set_data(buff, sz);
      SendRspMessageToClient(reply);
    } else {
      SendErrorReply(kErrorReadMemoryFailed);
    }
  }
}

void DebugServer::Visit(rsp::WriteMemoryCommand* packet) {
  const debug::Blob& data = packet->data();
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL != proc) {
    char tmp[rsp::kMaxRspPacketSize];
    if (data.size() > sizeof(tmp)) {
      SendErrorReply(kErrorPacketIsTooLarge);
      return;
    }
    data.Peek(0, tmp, data.size());
    char* addr = reinterpret_cast<char*>(packet->addr());
    // massage address to support gdb
    if (addr < proc->nexe_mem_base())
      addr += reinterpret_cast<size_t>(proc->nexe_mem_base());

    bool res = proc->WriteMemory(addr, data.size(), tmp);
    if (res)
      SendRspMessageToClient(rsp::OkReply());
    else
      SendErrorReply(kErrorWriteMemoryFailed);
  }
}

void DebugServer::Visit(rsp::ReadRegistersCommand* packet) {
  Blob blob;
  DebuggeeThread* thread = GetFocusedThread();
  if (NULL != thread) {
    CONTEXT ct;
    if (!thread->GetContext(&ct)) {
      logger_->Log("WARN21.07",
                   "thread->GetContext failed. pid=%d tid=%d",
                   focused_process_id_,
                   focused_thread_id_);
      SendErrorReply(kErrorGetThreadContextFailed);
      return;
    }
    rsp::CONTEXTToGdbRegisters(ct, &blob);
    rsp::BlobReply reply;
    reply.set_data(blob);
    SendRspMessageToClient(reply);
  }
}

void DebugServer::Visit(rsp::WriteRegistersCommand* packet) {
  DebuggeeThread* thread = GetFocusedThread();
  if (NULL != thread) {
    CONTEXT ct;
    if (!thread->GetContext(&ct)) {
      SendErrorReply(kErrorGetThreadContextFailed);
      return;
    }
    rsp::GdbRegistersToCONTEXT(packet->data(), &ct);
    if (!thread->SetContext(ct))
      SendErrorReply(kErrorSetThreadContextFailed);
    else
      SendRspMessageToClient(rsp::OkReply());
  }
}

void DebugServer::Visit(rsp::GetCurrentThreadCommand* packet) {
  rsp::GetCurrentThreadReply reply;
  reply.set_value(focused_thread_id_);
  SendRspMessageToClient(reply);
}

void DebugServer::Visit(rsp::StepCommand* packet) {
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL != proc) {
    if (!proc->SingleStep())
      SendErrorReply(kErrorSingleStepFailed);
  }
}

void DebugServer::Visit(rsp::IsThreadAliveCommand* packet) {
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL == proc)
    return;

  std::deque<int> tids;
  proc->GetThreadIds(&tids);
  if (std::find(tids.begin(), tids.end(), packet->value()) != tids.end()) {
    SendRspMessageToClient(rsp::OkReply());
  } else {
    SendErrorReply(kErrorThreadIsDead);
  }
}

void DebugServer::Visit(rsp::GetThreadInfoCommand* packet) {
  IDebuggeeProcess* proc = GetFocusedProcess();
  if (NULL == proc)
    return;

  rsp::GetThreadInfoReply reply;
  if (packet->get_more()) {
    // TODO(garianov): add support for multi-packet replies.
    reply.set_eom(true);
  } else {
    std::deque<int> tids;
    proc->GetThreadIds(&tids);

    std::deque<int> nexe_tids;
    for (size_t i = 0; i < tids.size(); i++) {
      int tid = tids[i];
      DebuggeeThread* thread = proc->GetThread(tid);
      if ((NULL != thread) && (thread->IsNaClAppThread()))
        nexe_tids.push_back(tid);
    }
    reply.set_threads_ids(nexe_tids);
    reply.set_eom(false);
  }
  SendRspMessageToClient(reply);
}

// 0. By default, don't stop and pass exception to debuggee
// 1. Don't stop on OUTPUT_DEBUG_STRING_EVENT
// 2. Stop on NaCl debug events, except DebugEvent::kAppStarted
// 3. Don't pass EXCEPTION_BREAKPOINT to debuggee
// 4. Stop on EXCEPTION_BREAKPOINT in nexe threads, don't stop on
//    EXCEPTION_BREAKPOINT in other threads
// 5. Don't stop on kVS2008_THREAD_INFO exception, pass it to debuggee
//
void DebugServer::MakeContinueDecision(const debug::DebugEvent& debug_event,
                                       bool is_nacl_app_thread,
                                       bool* halt,
                                       bool* pass_exception) {
  *halt = false;
  *pass_exception = true;

  DEBUG_EVENT wde = debug_event.windows_debug_event();
  if (OUTPUT_DEBUG_STRING_EVENT == wde.dwDebugEventCode) {
      *halt = false;
  }
  int nacl_de_id = debug_event.nacl_debug_event_code();
  if (debug::DebugEvent::kNotNaClDebugEvent != nacl_de_id) {
    if (debug::DebugEvent::kAppStarted != nacl_de_id)
      *halt = true;
    return;
  }
  if (EXCEPTION_DEBUG_EVENT == wde.dwDebugEventCode) {
    int exception_code = wde.u.Exception.ExceptionRecord.ExceptionCode;
    if (kVS2008_THREAD_INFO != exception_code) {
      if (EXCEPTION_BREAKPOINT == exception_code)
        *pass_exception = false;

      *halt = is_nacl_app_thread;
    }
  } else if (EXIT_THREAD_DEBUG_EVENT == wde.dwDebugEventCode) {
      *halt = is_nacl_app_thread;
  }
}
}  // namespace debug

