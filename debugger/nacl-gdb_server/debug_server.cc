#include "debugger/debug_server/debug_server.h"
#include <assert.h>
#include <algorithm>
#include "debugger/core/debug_logger.h"
#include "debugger/core/debuggee_thread.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/rsp/rsp_common_replies.h"
#include "debugger/rsp/rsp_control_packets.h"
#include "debugger/rsp/rsp_info_packets.h"
#include "debugger/rsp/rsp_packet_utils.h"
#include "debugger/rsp/rsp_packets.h"
#include "debugger/rsp/rsp_threads_packets.h"
#include "rsp_registers.h"
#include "rsp_stop_from_debug_event.h"

namespace {
const int kReadBufferSize = 1024;
const int kVS2008_THREAD_INFO = 0x406D1388;
}  // namespace

namespace debug {
DebugServer::DebugServer(DebugAPI* debug_api)
    : state_(kIdle),
      connection_was_established_(false),
      debug_api_(*debug_api),
      focused_process_id_(0),
      focused_thread_id_(0),
      main_nexe_thread_id_(0) {
  rsp_packetizer_.SetPacketConsumer(this);
  execution_engine_ = new ExecutionEngine(&debug_api_);

#ifdef COMN_LOG
  logger_ = Logger::Get();
#else
  debug::TextFileLogger* log = new debug::TextFileLogger();
  log->Open("nacl_gdbserver_log.txt");
  log->EnableStdout(true); 
  logger_ = log;
#endif
}

DebugServer::~DebugServer() {
  delete execution_engine_;
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
    if (connection_was_established_) {
      connection_was_established_ = false;
      logger_->Log("TR100.3", "Dropped connection from debugger.\n");
    }
    if (listening_socket_.Accept(0, &client_connection_)) {
      connection_was_established_ = true;
      logger_->Log("TR100.4", "Got connection from debugger.\n");
    }
  } else {
    char buff[kReadBufferSize];
    for (int i = 0; i < 100; i++) {
      size_t read_bytes = client_connection_.Read(buff,
                                                    sizeof(buff) - 1,
                                                    0);
      if (read_bytes > 0) {
        buff[read_bytes] = 0;
        logger_->Log("R>", "[%s]\n", buff);
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
    MakeContinueDecision(de, halted_thread, &halt_debuggee, &pass_exception);

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
  execution_engine_->Stop(300);
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
  
  if (0 == main_nexe_thread_id_) {
    focused_process_id_ = halted_process->id();
    main_nexe_thread_id_ = focused_thread_id_;
  }
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

void DebugServer::OnBreak() {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL != proc)
    proc->Break();
  else
    SendRspMessageToClient(rsp::ErrorReply(1));
}

void DebugServer::Visit(rsp::GetStopReasonCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    SendRspMessageToClient(rsp::ErrorReply(2));
    return;
  }
  DebugEvent debug_event = proc->last_debug_event();
  DEBUG_EVENT wde = debug_event.windows_debug_event();
  if (proc->IsHalted())
    SendRspMessageToClient(rsp::CreateStopReplyPacket(debug_event));
  else if (DebuggeeProcess::kDead == proc->state())
    SendRspMessageToClient(rsp::ErrorReply(3));
  else  // Process is running.
    SendRspMessageToClient(rsp::StopReply(rsp::StopReply::STILL_RUNNING));
}

void DebugServer::Visit(rsp::ContinueCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc)
    SendRspMessageToClient(rsp::ErrorReply(4));
  else
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
  //qXfer:features:read:target.xml:0,7ca
  if (packet->file_name() == "target.xml") {
    rsp::QXferReply reply;
#ifdef _WIN64
    reply.set_body("<target><architecture>i386:x86-64</architecture></target>");
#else
    reply.set_body("<target><architecture>i386:x86-32</architecture></target>");
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
    SendRspMessageToClient(rsp::ErrorReply(5));
}

void DebugServer::Visit(rsp::ReadMemoryCommand* packet) {
  int sz = packet->num_of_bytes();
  if (0 == sz) {
    SendRspMessageToClient(rsp::ErrorReply(6));
    return;
  }
  char buff[rsp::kMaxRspPacketSize];
  if (sizeof(buff) < sz) {
    SendRspMessageToClient(rsp::ErrorReply(7));
    return;
  }
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
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
      SendRspMessageToClient(rsp::ErrorReply(8));
    }
  } else {
    SendRspMessageToClient(rsp::ErrorReply(9));
  }
}

void DebugServer::Visit(rsp::WriteMemoryCommand* packet) {
  const debug::Blob& data = packet->data();
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL != proc) {
    char tmp[rsp::kMaxRspPacketSize];
    size_t rd = data.Peek(0, tmp, data.size());
    if (data.size() != rd) {
      SendRspMessageToClient(rsp::ErrorReply(10));
      return;
    }
    char* addr = reinterpret_cast<char*>(packet->addr());
    // massage address to support gdb
    if (addr < proc->nexe_mem_base())
      addr += reinterpret_cast<size_t>(proc->nexe_mem_base());

    bool res = proc->WriteMemory(addr, data.size(), tmp);
    if (res)
      SendRspMessageToClient(rsp::OkReply());
    else
      SendRspMessageToClient(rsp::ErrorReply(11));
  } else {
    logger_->Log("WARN21.02", "Can't find process pid=%d", focused_process_id_);
    SendRspMessageToClient(rsp::ErrorReply(12));
  }
}

void DebugServer::Visit(rsp::ReadRegistersCommand* packet) {
  Blob registers_blob;
  if (ReadGdbRegisters(&registers_blob)) {
    rsp::BlobReply reply;
    reply.set_data(registers_blob);
    SendRspMessageToClient(reply);
  } else {
    logger_->Log("WARN21.03",
                 "DebugServer::ReadGdbRegisters pid=%d tid=%d -> false",
                 focused_process_id_,
                 focused_thread_id_);
    SendRspMessageToClient(rsp::ErrorReply(13));
  }
}

void DebugServer::Visit(rsp::WriteRegistersCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    SendRspMessageToClient(rsp::ErrorReply(14));
    return;
  }
  DebuggeeThread* thread = proc->GetThread(focused_thread_id_);
  if (NULL == thread) {
    SendRspMessageToClient(rsp::ErrorReply(15));
    return;
  }
  CONTEXT ct;
  if (!thread->GetContext(&ct)) {
    SendRspMessageToClient(rsp::ErrorReply(16));
    return;
  }
  rsp::GdbRegistersToCONTEXT(packet->data(), &ct);
  if (!thread->SetContext(ct))
    SendRspMessageToClient(rsp::ErrorReply(17));
  else
    SendRspMessageToClient(rsp::OkReply());
}

bool DebugServer::ReadGdbRegisters(Blob* blob) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    logger_->Log("WARN21.05", "Can't find process pid=%d", focused_process_id_);
    return false;
  }
  DebuggeeThread* thread = proc->GetThread(focused_thread_id_);
  if (NULL == thread) {
    logger_->Log("WARN21.06", "Can't find thread pid=%d tid=%d", focused_process_id_, focused_thread_id_);
    return false;
  }
  CONTEXT ct;
  if (!thread->GetContext(&ct)) {
    logger_->Log("WARN21.07", "thread->GetContext failed. pid=%d tid=%d", focused_process_id_, focused_thread_id_);
    return false;
  }
  rsp::CONTEXTToGdbRegisters(ct, blob);
  logger_->Log("TRACE21.08", "DebugServer::ReadGdbRegisters pid=%d tid=%d -> true", focused_process_id_, focused_thread_id_);
  return true;
}

void DebugServer::Visit(rsp::GetCurrentThreadCommand* packet) {
  rsp::GetCurrentThreadReply reply;
  reply.set_value(focused_thread_id_);
  SendRspMessageToClient(reply);
}

void DebugServer::Visit(rsp::StepCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc)
    SendRspMessageToClient(rsp::ErrorReply(18));
  else if (!proc->SingleStep())
    SendRspMessageToClient(rsp::ErrorReply(19));
}

void DebugServer::Visit(rsp::IsThreadAliveCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    SendRspMessageToClient(rsp::ErrorReply(20));
    return;
  }
  std::deque<int> tids;
  proc->GetThreadIds(&tids);
  if (std::find(tids.begin(), tids.end(), packet->value()) != tids.end()) {
    SendRspMessageToClient(rsp::OkReply());
    // found it
  } else {
    SendRspMessageToClient(rsp::ErrorReply(21));
    // TODO: create enum for error codes
  }
}

void DebugServer::Visit(rsp::GetThreadInfoCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    SendRspMessageToClient(rsp::ErrorReply(22));
    return;
  }
  rsp::GetThreadInfoReply reply;
  if (packet->get_more()) {
    reply.set_eom(true);  // TODO(garianov): add support for multy packet replies.
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
                                       debug::DebuggeeThread* thread,
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
      bool is_nexe = false;
      if (NULL != thread)
        is_nexe = thread->IsNaClAppThread();

      if (EXCEPTION_BREAKPOINT == exception_code)
        *pass_exception = false;

      if (is_nexe)
        *halt = true;
      else
        *halt = false;
    }
  } else if (EXIT_THREAD_DEBUG_EVENT == wde.dwDebugEventCode) {
    if ((NULL != thread) &&  (thread->IsNaClAppThread()))
      *halt = true;
  }
}
}  // namespace debug
