#include "debugger/debug_server/debug_server.h"
#include <assert.h>
#include <algorithm>
#include "debugger/core/debuggee_thread.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/rsp/rsp_packet.h"
#include "debugger/core/debug_logger.h"

namespace {
const int kReadBufferSize = 1024;
const int kVS2008_THREAD_INFO = 0x406D1388;

void MakeContinueDecision(
    const debug::DebugEvent& debug_event,
    debug::DebuggeeThread* thread,
    bool* halt,
    bool* pass_exception);
rsp::StopReply GetStopReply(
    debug::IDebuggeeProcess* halted_process);
int GetSignalNumber(const debug::DebugEvent& debug_event);  

bool ExitedNormally(int ret_code) {
  return ((ret_code >=0) && (ret_code < 256));
}
void PrintCONTEXT(const CONTEXT& ct);
}  // namespace

namespace debug {
DebugServer::DebugServer()
    : connection_was_established_(false),
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
  if (res)
    logger_->Log("TR100.2", "Starting process [%s]...\n", cmd);
  return res;
}

void DebugServer::DoWork(int wait_ms) {
  if (!debugger_connection_.IsConnected()) {
    if (connection_was_established_) {
      connection_was_established_ = false;
      logger_->Log("TR100.3", "Dropped connection from debugger.\n");
    }
    if (listening_socket_.Accept(&debugger_connection_, 0)) {
      connection_was_established_ = true;
      logger_->Log("TR100.4", "Got connection from debugger.\n");
    }
  } else {
    char buff[kReadBufferSize];
    for (int i = 0; i < 100; i++) {
      size_t read_bytes = debugger_connection_.Read(buff,
                                                    sizeof(buff) - 1,
                                                    0);
      if (read_bytes > 0) {
        buff[read_bytes] = 0;
        logger_->Log("R>", "[%s]\n", buff);
        if (strncmp(buff, "+$Xc000206", 10) == 0)
          printf("");

        rsp_packetizer_.OnData(buff, read_bytes);
      } else {
        break;
      }
    }
  }
  int pid = 0;
  execution_engine_->WaitForDebugEventAndDispatchIt(wait_ms, &pid);
  IDebuggeeProcess* halted_process = execution_engine_->GetProcess(pid);
  if (NULL != halted_process) {
    if (debug::DebugEvent::kThreadIsAboutToStart ==
        execution_engine_->debug_event().nacl_debug_event_code()) {
      DebuggeeThread* thread = halted_process->GetHaltedThread();
      if ((NULL != thread) && thread->IsNaClAppThread())
        logger_->Log("TR100.5",
                    "NaClThreadStart mem_base=%p entry_point=%p thread_id=%d\n",
                    halted_process->nexe_mem_base(),
                    halted_process->nexe_entry_point(),
                    thread->id());
    }

    bool halt = false;
    bool pass_exception = true;
    MakeContinueDecision(execution_engine_->debug_event(),
                         halted_process->GetHaltedThread(),
                         &halt,
                         &pass_exception);
    if (halt)
      OnHaltedProcess(halted_process, execution_engine_->debug_event());
    else if (pass_exception)
      halted_process->ContinueAndPassExceptionToDebuggee();
    else
      halted_process->Continue();
  }
}

void DebugServer::Quit() {
  execution_engine_->Stop(300);
  listening_socket_.Close();
  debugger_connection_.Close();
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
  
// TODO: cleanup
  CONTEXT ct;
  halted_process->GetHaltedThread()->GetContext(&ct);
  PrintCONTEXT(ct);

  if (0 == main_nexe_thread_id_) {
    focused_process_id_ = halted_process->id();
    main_nexe_thread_id_ = focused_thread_id_;
  }
  focused_thread_id_ = halted_process->GetHaltedThread()->id();

  if (debugger_connection_.IsConnected())
    PostRspMessage(GetStopReply(halted_process));
}

void DebugServer::PostRspMessage(const rsp::Packet& msg) {
  Blob text;
  msg.ToBlob(&text);
  Blob wire_msg;
  rsp::PacketUtil::AddEnvelope(text, &wire_msg);
  debugger_connection_.WriteAll(wire_msg);
  logger_->Log("T>", "[%s]\n", text.ToString().c_str());
}

void DebugServer::OnUnknownCommand() {
  PostRspMessage(rsp::EmptyPacket());
}

void DebugServer::OnPacket(const Blob& body, bool valid_checksum) {
  if (valid_checksum)
    debugger_connection_.WriteAll("+");

  Blob msg = body;
  rsp::Packet* command = rsp::Packet::CreateFromBlob(&msg);
  if (NULL != command) {
    command->AcceptVisitor(this);
    delete command;
  } else {
    PostRspMessage(rsp::EmptyPacket());
  }
}

void DebugServer::OnUnexpectedChar(char unexpected_char) {
  logger_->Log("WARN21.01", "msg='DebugServer::OnUnexpectedChar' c='0x%x'", static_cast<int>(unexpected_char));
}

void DebugServer::OnBreak() {
  if (0 != focused_process_id_) {
    //TODO : implement
    OnUnknownCommand();
  } else {
    PostRspMessage(rsp::ErrorReply(1));
  }
}

void DebugServer::Visit(rsp::GetStopReasonCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    PostRspMessage(rsp::ErrorReply(2));
    return;
  }
  DebugEvent debug_event = proc->last_debug_event();
  DEBUG_EVENT wde = debug_event.windows_debug_event();
  if (wde.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) {
    if (wde.dwThreadId == main_nexe_thread_id_) {
      int ret_code = wde.u.ExitThread.dwExitCode;
      if (ExitedNormally(ret_code)) {
        rsp::StopReply reply(rsp::StopReply::EXITED);
        reply.set_exit_code(ret_code);
        PostRspMessage(reply);
      } else {
        rsp::StopReply reply(rsp::StopReply::TERMINATED);
        reply.set_signal_number(ret_code);
        PostRspMessage(reply);
      }
      return;
    }
  }
  if (proc->IsHalted())
    PostRspMessage(GetStopReply(proc));
  else if (DebuggeeProcess::kDead == proc->state())
    PostRspMessage(rsp::ErrorReply(5));
  else  // Process is running.
    PostRspMessage(rsp::StopReply(rsp::StopReply::STILL_RUNNING));
}

void DebugServer::Visit(rsp::ContinueCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc)
    PostRspMessage(rsp::ErrorReply(4));
  else
    proc->Continue();
}

void DebugServer::Visit(rsp::QuerySupportedCommand* packet) {
  rsp::QuerySupportedReply reply;
  reply.AddFeature("PacketSize", "7cf");
  reply.AddFeature("qXfer:libraries:read", "+");
  reply.AddFeature("qXfer:features:read", "+");
  PostRspMessage(reply);
}

void DebugServer::Visit(rsp::QXferFeaturesReadCommand* packet) {
  //qXfer:features:read:target.xml:0,7ca
  if (packet->file_name() == "target.xml") {
    rsp::QXferReply reply;
    reply.set_body("<target><architecture>i386:x86-64</architecture></target>");
    reply.set_eom(true);
    PostRspMessage(reply);
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
    PostRspMessage(rsp::OkReply());
  else
    PostRspMessage(rsp::ErrorReply(3));
}

void DebugServer::Visit(rsp::ReadMemoryCommand* packet) {
  int sz = packet->num_of_bytes();
  if (0 == sz) {
    PostRspMessage(rsp::ErrorReply(4));
    return;
  }
  // We advertize max size of RSP packet as 1999
  char buff[3000];
  if (sizeof(buff) < sz) {
    PostRspMessage(rsp::ErrorReply(5));
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
      PostRspMessage(reply);
    } else {
      PostRspMessage(rsp::ErrorReply(6));
    }
  } else {
    PostRspMessage(rsp::ErrorReply(7));
  }
}

void DebugServer::Visit(rsp::WriteMemoryCommand* packet) {
  const debug::Blob& data = packet->data();
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL != proc) {
    void* tmp = data.ToCBuffer();
    if (NULL == tmp) {
      PostRspMessage(rsp::ErrorReply(8));
      return;
    }

    char* addr = reinterpret_cast<char*>(packet->addr());
    // massage address to support gdb
    if (addr < proc->nexe_mem_base())
      addr += reinterpret_cast<size_t>(proc->nexe_mem_base());

    bool res = proc->WriteMemory(addr, data.size(), tmp);
    free(tmp);
    if (res)
      PostRspMessage(rsp::OkReply());
    else
      PostRspMessage(rsp::ErrorReply(9));
  } else {
    logger_->Log("WARN21.02", "Can't find process pid=%d", focused_process_id_);
    PostRspMessage(rsp::ErrorReply(10));
  }
}

void DebugServer::Visit(rsp::ReadRegistersCommand* packet) {
  Blob registers_blob;
  if (ReadGdbRegisters(&registers_blob)) {
    rsp::BlobReply reply;
    reply.set_data(registers_blob);
    PostRspMessage(reply);
  } else {
    logger_->Log("WARN21.03", "DebugServer::ReadGdbRegisters pid=%d tid=%d -> false", focused_process_id_, focused_thread_id_);
    PostRspMessage(rsp::ErrorReply(113));
  }
}

void DebugServer::Visit(rsp::WriteRegistersCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    PostRspMessage(rsp::ErrorReply(14));
    return;
  }
  DebuggeeThread* thread = proc->GetThread(focused_thread_id_);
  if (NULL == thread) {
    PostRspMessage(rsp::ErrorReply(15));
    return;
  }
  CONTEXT ct;
  if (!thread->GetContext(&ct)) {
    PostRspMessage(rsp::ErrorReply(16));
    return;
  }
  GdbRegistersToCONTEXT(packet->data(), &ct);
  if (!thread->SetContext(ct))
    PostRspMessage(rsp::ErrorReply(16));
  else
    PostRspMessage(rsp::OkReply());
}

#define X86_32_REGS \
  REG(Eax); \
  REG(Ecx); \
  REG(Edx); \
  REG(Ebx); \
  REG(Esp); \
  REG(Ebp); \
  REG(Esi); \
  REG(Edi); \
  REG(Eip); \
  REG(EFlags); \
  REG(SegCs); \
  REG(SegSs); \
  REG(SegDs); \
  REG(SegEs); \
  REG(SegFs); \
  REG(SegGs)

#define X86_64_REGS \
  REG(Rax); \
  REG(Rbx); \
  REG(Rcx); \
  REG(Rdx); \
  REG(Rsi); \
  REG(Rdi); \
  REG(Rbp); \
  REG(Rsp); \
  REG(R8); \
  REG(R9); \
  REG(R10); \
  REG(R11); \
  REG(R12); \
  REG(R13); \
  REG(R14); \
  REG(R15); \
  REG(Rip); \
  REG(EFlags); \
  REG(SegCs); \
  REG(SegSs); \
  REG(SegDs); \
  REG(SegEs); \
  REG(SegFs); \
  REG(SegGs)

char* names_of_bad_regs = "EFlags,SegCs,SegSs,SegDs,SegEs,SegFs,SegGs,";

size_t WriteReg(const Blob& blob, size_t offset, void* dst, int reg_size, const char* name) {
  char* pp = strstr(names_of_bad_regs,name);
  size_t name_len = strlen(name);
  if ((NULL != pp) && (',' == *(pp + name_len))) {
    unsigned long reg = 0;
    reg_size = sizeof(reg);
    blob.Peek(offset, &reg, sizeof(reg));
    memcpy(dst, &reg, 2);
  } else {
    blob.Peek(offset, dst, reg_size);
  }
  return offset + reg_size;
}

void DebugServer::GdbRegistersToCONTEXT(const Blob& gdb_regs, CONTEXT* ct) {
  size_t offset = 0;
#define REG(name) ct->name = 0; offset = WriteReg(gdb_regs, offset, &ct->name, sizeof(ct->name), #name)
#ifdef _WIN64
  X86_64_REGS;
#else
  X86_32_REGS;
#endif  
#undef REG
}

void ReadReg(Blob* blob, void* src, int reg_size, const char* name) {
  char* pp = strstr(names_of_bad_regs,name);
  size_t name_len = strlen(name);
  if ((NULL != pp) && (',' == *(pp + name_len))) {
    unsigned long reg = 0;
    memcpy(&reg, src, 2);
    blob->Append(Blob(&reg, sizeof(reg)));
  } else {
    blob->Append(Blob(src, reg_size));
  }
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

#define REG(name)  ReadReg(blob, &ct.name, sizeof(ct.name), #name)
#ifdef _WIN64
  X86_64_REGS;
#else
  X86_32_REGS;
#endif  
#undef REG

  logger_->Log("TRACE21.08", "DebugServer::ReadGdbRegisters pid=%d tid=%d -> true", focused_process_id_, focused_thread_id_);
  return true;
}

void DebugServer::Visit(rsp::GetCurrentThreadCommand* packet) {
  rsp::GetCurrentThreadReply reply;
  reply.set_value(focused_thread_id_);
  PostRspMessage(reply);
}

void DebugServer::Visit(rsp::StepCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc)
    PostRspMessage(rsp::ErrorReply(20));
  else if (!proc->SingleStep())
    PostRspMessage(rsp::ErrorReply(21));
}

void DebugServer::Visit(rsp::IsThreadAliveCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    PostRspMessage(rsp::ErrorReply(22));
    return;
  }
  std::deque<int> tids;
  proc->GetThreadIds(&tids);
  if (std::find(tids.begin(), tids.end(), packet->value()) != tids.end()) {
    PostRspMessage(rsp::OkReply());
    // found it
  } else {
    PostRspMessage(rsp::ErrorReply(23));  // TODO: make error codes declared constants
  }
}

void DebugServer::Visit(rsp::GetThreadInfoCommand* packet) {
  IDebuggeeProcess* proc = execution_engine_->GetProcess(focused_process_id_);
  if (NULL == proc) {
    PostRspMessage(rsp::ErrorReply(24));
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
  PostRspMessage(reply);
}

}  // namespace debug

namespace {
void MakeContinueDecision(
    const debug::DebugEvent& debug_event,
    debug::DebuggeeThread* thread,
    bool* halt,
    bool* pass_exception) {
  if (OUTPUT_DEBUG_STRING_EVENT == debug_event.windows_debug_event().dwDebugEventCode) {
      *halt = false;
      *pass_exception = false;
  }

  if (debug::DebugEvent::kNotNaClDebugEvent != debug_event.nacl_debug_event_code()) {
    *halt = true;
    return;
  }
  int exception_code = debug_event.windows_debug_event().u.Exception.ExceptionRecord.ExceptionCode;

  if (EXCEPTION_DEBUG_EVENT == debug_event.windows_debug_event().dwDebugEventCode) {
    if (kVS2008_THREAD_INFO == exception_code) {
      *halt = false;
    } else {
      bool is_nexe = false;
      if (NULL != thread)
        is_nexe = thread->IsNaClAppThread();

      if (is_nexe)
        printf("");

      *pass_exception = true;

      if (EXCEPTION_BREAKPOINT == exception_code)
        *pass_exception = false;

      if (is_nexe) {
        *halt = true;
      } else {
        *halt = false;
      }
    }
  }
}

rsp::StopReply GetStopReply(debug::IDebuggeeProcess* halted_process) {
  debug::DebugEvent debug_event = halted_process->last_debug_event();
  DEBUG_EVENT wde = debug_event.windows_debug_event();

  int exception_code = wde.u.Exception.ExceptionRecord.ExceptionCode;
  switch (wde.dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT:
    case CREATE_THREAD_DEBUG_EVENT:
    case EXCEPTION_DEBUG_EVENT:
    case LOAD_DLL_DEBUG_EVENT:
    case OUTPUT_DEBUG_STRING_EVENT:
    case EXIT_THREAD_DEBUG_EVENT:
    case UNLOAD_DLL_DEBUG_EVENT: {
        rsp::StopReply reply(rsp::StopReply::SIGNALED);
        reply.set_signal_number(GetSignalNumber(debug_event));
        return reply;
    }
    case RIP_EVENT: {
        rsp::StopReply reply(rsp::StopReply::TERMINATED);
        reply.set_signal_number(GetSignalNumber(debug_event));
        return reply;
    }
    case EXIT_PROCESS_DEBUG_EVENT: {
      // NaClAppThread terminated or exited: 
      int ret_code = wde.u.ExitThread.dwExitCode;
      if (ExitedNormally(ret_code)) {
        rsp::StopReply reply(rsp::StopReply::EXITED);
        reply.set_exit_code(ret_code);
        return reply;
      } else {
        rsp::StopReply reply(rsp::StopReply::TERMINATED);
        reply.set_signal_number(ret_code);
        return reply;
      }
    }
  }
  return rsp::StopReply::SIGNALED;
}

int GetSignalNumber(const debug::DebugEvent& debug_event) {
  const int SIGSEGV = 11;
  const int SIGSYS = 31;
  const int SIGTRAP = 5;
  const int SIGBUS = 7;
  const int SIGFPE = 8;
  const int SIGILL = 4;
  const int SIGINT = 2;
  const int SIGSTOP = 19;

  switch (debug_event.windows_debug_event().dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT:
    case CREATE_THREAD_DEBUG_EVENT:
    case LOAD_DLL_DEBUG_EVENT:
    case OUTPUT_DEBUG_STRING_EVENT:
    case UNLOAD_DLL_DEBUG_EVENT:
    case EXIT_THREAD_DEBUG_EVENT:
    case EXIT_PROCESS_DEBUG_EVENT:
        return SIGSTOP;

    case EXCEPTION_DEBUG_EVENT: {
      switch (debug_event.windows_debug_event().u.Exception.ExceptionRecord.ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_STACK_OVERFLOW:
            return SIGSEGV;
        case EXCEPTION_BREAKPOINT:
        case EXCEPTION_SINGLE_STEP:
            return SIGTRAP;

        case EXCEPTION_DATATYPE_MISALIGNMENT: return SIGBUS;

        case EXCEPTION_FLT_DENORMAL_OPERAND:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_FLT_INEXACT_RESULT:
        case EXCEPTION_FLT_INVALID_OPERATION:
        case EXCEPTION_FLT_OVERFLOW:
        case EXCEPTION_FLT_STACK_CHECK:
        case EXCEPTION_FLT_UNDERFLOW: 
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
        case EXCEPTION_INT_OVERFLOW:
            return SIGFPE;

        case EXCEPTION_ILLEGAL_INSTRUCTION:
        case EXCEPTION_PRIV_INSTRUCTION:
            return SIGILL;
        case DBG_CONTROL_C: return SIGINT;
      }
    }
  }
  return SIGSYS;
}

void PrintCONTEXT(const CONTEXT& ct) {
  printf("CONTEXT = {\n");
#define REG(name)  printf("\t%s=0x%x\n", #name, ct.name)
#ifdef _WIN64
  X86_64_REGS;
#else
  X86_32_REGS;
#endif  
#undef REG
  printf("}\n");
}

}  // namespace
