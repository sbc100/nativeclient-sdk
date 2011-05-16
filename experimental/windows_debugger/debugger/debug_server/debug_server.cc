#include "debugger/debug_server/debug_server.h"
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
rsp::StopReply::StopReason GetStopReason(
    debug::DebuggeeProcess* halted_process,
    const debug::DebugEvent& debug_event);
int GetSignalNumber(const debug::DebugEvent& debug_event);  
}  // namespace

namespace debug {
DebugServer::DebugServer()
    : connection_was_established_(false) {
  rsp_packetizer_.SetPacketConsumer(this);
  execution_engine_ = new ExecutionEngine(debug_api_);
}

DebugServer::~DebugServer() {
  delete execution_engine_;
}

bool DebugServer::ListenOnPort(int port) {
  bool res = listening_socket_.Listen(port);
  if (res)
    printf("Started listening on port %d ...\n", port);
  return res;
}

bool DebugServer::StartProcess(const char* cmd, const char* work_dir) {
  bool res = execution_engine_->StartProcess(cmd, work_dir);
  if (res)
    printf("Starting process [%s]...\n", cmd);
  return res;
}

void DebugServer::DoWork(int wait_ms) {
  if (!debugger_connection_.IsConnected()) {
    if (connection_was_established_) {
      connection_was_established_ = false;
      printf("Dropped connection from debugger.\n");
    }
    if (listening_socket_.Accept(&debugger_connection_, 0)) {
      connection_was_established_ = true;
      printf("Got connection from debugger.\n");
    }
  } else {
    char buff[kReadBufferSize];
    for (int i = 0; i < 100; i++) {
      size_t read_bytes = debugger_connection_.Read(buff,
                                                    sizeof(buff) - 1,
                                                    0);
      if (read_bytes > 0) {
        buff[read_bytes] = 0;
        printf("h>%s\n", buff);
        rsp_packetizer_.OnData(buff, read_bytes);
      } else {
        break;
      }
    }
  }
  DebuggeeProcess* halted_process = NULL;
  execution_engine_->DoWork(wait_ms, &halted_process);
  if (NULL != halted_process) {
    if (debug::DebugEvent::kThreadIsAboutToStart ==
        execution_engine_->debug_event().nacl_debug_event_code_) {
      DebuggeeThread* thread = halted_process->GetHaltedThread();
      if ((NULL != thread) && thread->is_nexe())
        printf("NaClThreadStart mem_base=%p entry_point=%p thread_id=%d\n",
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
  execution_engine_->Stop();
  listening_socket_.Close();
  debugger_connection_.Close();
}

void DebugServer::OnHaltedProcess(DebuggeeProcess* halted_process,
                                  const DebugEvent& debug_event) {
  char tmp[1000];
  debug_event.ToString(tmp, sizeof(tmp));
  printf("Halted: pid=%d tid=%d\ndebugevent=%s\n",
         halted_process->id(),
         halted_process->GetHaltedThread()->id(),
         tmp);

  DBG_LOG("TR21.01", "msg='DebugServer::OnHaltedProcess' pid='%d'", halted_process->id());
  if (debugger_connection_.IsConnected()) {
    rsp::StopReply msg(GetStopReason(halted_process, debug_event));
    if ((rsp::StopReply::SIGNALED == msg.stop_reason()) ||
       ((rsp::StopReply::TERMINATED == msg.stop_reason())))
      msg.set_signal_number(GetSignalNumber(debug_event));
    else
      msg.set_exit_code(debug_event.windows_debug_event_.u.ExitProcess.dwExitCode);
    msg.set_pid(halted_process->id());
    SendMessage(msg);
  }
}

void DebugServer::SendMessage(const rsp::Packet& msg) {
  Blob text;
  msg.ToBlob(&text);
  Blob wire_msg;
  rsp::PacketUtil::AddEnvelope(text, &wire_msg);
  debugger_connection_.WriteAll(wire_msg);
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
    rsp::EmptyPacket reply;
    SendMessage(reply);
  }
}

void DebugServer::OnUnexpectedChar(char unexpected_char) {
  DBG_LOG("WARN21.02", "msg='DebugServer::OnUnexpectedChar' c='0x%x'", static_cast<int>(unexpected_char));
}

void DebugServer::OnBreak() {
  //TODO : implement
}

void DebugServer::Visit(rsp::GetStopReasonCommand* packet) {
  //TODO : implement
}

void DebugServer::Visit(rsp::StopReply* packet) {
  //TODO : implement
}

void DebugServer::Visit(rsp::ContinueCommand* packet) {
  //TODO : implement
}

}  // namespace debug

namespace {
void MakeContinueDecision(
    const debug::DebugEvent& debug_event,
    debug::DebuggeeThread* thread,
    bool* halt,
    bool* pass_exception) {
  if (OUTPUT_DEBUG_STRING_EVENT == debug_event.windows_debug_event_.dwDebugEventCode) {
      *halt = false;
      *pass_exception = false;
  }

  if (debug::DebugEvent::kNotNaClDebugEvent != debug_event.nacl_debug_event_code_) {
    *halt = true;
    return;
  }
  int exception_code = debug_event.windows_debug_event_.u.Exception.ExceptionRecord.ExceptionCode;

  if (EXCEPTION_DEBUG_EVENT == debug_event.windows_debug_event_.dwDebugEventCode) {
    if (kVS2008_THREAD_INFO == exception_code) {
      *halt = false;
    } else {
      bool is_nexe = false;
      if (NULL != thread)
        is_nexe = thread->is_nexe();

      *pass_exception = true;

      if (EXCEPTION_BREAKPOINT == exception_code)
        *pass_exception = false;

      if (is_nexe) {
        //*halt = true;  // TODO: turn it on
      } else {
        *halt = false;
      }
    }
  }
}

rsp::StopReply::StopReason GetStopReason(
    debug::DebuggeeProcess* halted_process,
    const debug::DebugEvent& debug_event) {
  int exception_code = debug_event.windows_debug_event_.u.Exception.ExceptionRecord.ExceptionCode;
  switch (debug_event.windows_debug_event_.dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT:
    case CREATE_THREAD_DEBUG_EVENT:
    case EXCEPTION_DEBUG_EVENT:
    case LOAD_DLL_DEBUG_EVENT:
    case OUTPUT_DEBUG_STRING_EVENT:
    case UNLOAD_DLL_DEBUG_EVENT:
        return rsp::StopReply::SIGNALED;

    case RIP_EVENT: rsp::StopReply::TERMINATED;

    case EXIT_PROCESS_DEBUG_EVENT: {
      if ((exception_code > 1000) || (exception_code < -1000))
        return rsp::StopReply::TERMINATED;
      return rsp::StopReply::EXITED;
    }
    // TODO: verify that gdb_server acts this way
    case EXIT_THREAD_DEBUG_EVENT: {
      int ex_code = debug_event.windows_debug_event_.u.ExitThread.dwExitCode;
      if ((exception_code > 1000) || (exception_code < -1000))
        return rsp::StopReply::TERMINATED;
      return rsp::StopReply::EXITED;
    }
  }
  return rsp::StopReply::SIGNALED;
}

int GetSignalNumber(const debug::DebugEvent& debug_event) {
  const int SIGCHLD = 17;
  const int SIGSEGV = 11;
  const int SIGSYS = 31;
  const int SIGTRAP = 5;
  const int SIGBUS = 7;
  const int SIGFPE = 8;
  const int SIGILL = 4;
  const int SIGINT = 2;

  switch (debug_event.windows_debug_event_.dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT:
    case CREATE_THREAD_DEBUG_EVENT:
    case LOAD_DLL_DEBUG_EVENT:
    case OUTPUT_DEBUG_STRING_EVENT:
    case UNLOAD_DLL_DEBUG_EVENT:
        return SIGCHLD;

    case EXCEPTION_DEBUG_EVENT: {
      switch (debug_event.windows_debug_event_.u.Exception.ExceptionRecord.ExceptionCode) {
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
}  // namespace
