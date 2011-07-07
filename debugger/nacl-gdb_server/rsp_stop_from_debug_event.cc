// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/nacl-gdb_server/rsp_stop_from_debug_event.h"

namespace {
/// @return true if process exited by calling exit(), or false
/// if process was terminated.
bool ExitedNormally(int ret_code) {
  return ((ret_code >=0) && (ret_code < 256));
}
}  // namespace

namespace rsp {
rsp::StopReply CreateStopReplyPacket(const debug::DebugEvent& debug_event) {
  DEBUG_EVENT wde = debug_event.windows_debug_event();

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
      int ret_code = wde.u.ExitThread.dwExitCode;
      if (ExitedNormally(ret_code)) {
        rsp::StopReply reply(rsp::StopReply::EXITED);
        reply.set_exit_code(ret_code);
        return reply;
      } else {
        rsp::StopReply reply(rsp::StopReply::TERMINATED);
        reply.set_signal_number(GetSignalNumberForException(ret_code));
        return reply;
      }
    }
  }
  rsp::StopReply reply(rsp::StopReply::SIGNALED);
  reply.set_signal_number(kSIGSTOP);
  return reply;
}

int GetSignalNumber(const debug::DebugEvent& debug_event) {
  DEBUG_EVENT wde = debug_event.windows_debug_event();
  switch (wde.dwDebugEventCode) {
    case CREATE_PROCESS_DEBUG_EVENT:
    case CREATE_THREAD_DEBUG_EVENT:
    case LOAD_DLL_DEBUG_EVENT:
    case OUTPUT_DEBUG_STRING_EVENT:
    case UNLOAD_DLL_DEBUG_EVENT:
    case EXIT_THREAD_DEBUG_EVENT:
    case EXIT_PROCESS_DEBUG_EVENT:
      return kSIGSTOP;

    case EXCEPTION_DEBUG_EVENT: {
      int exception_code = wde.u.Exception.ExceptionRecord.ExceptionCode;
      return GetSignalNumberForException(exception_code);
    }
    case RIP_EVENT:
      return kSIGSYS;
  }
  return kSIGNO;
}

int GetSignalNumberForException(int exception_code) {
  switch (exception_code) {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_STACK_OVERFLOW:
      return kSIGSEGV;

    case EXCEPTION_BREAKPOINT:
    case EXCEPTION_SINGLE_STEP:
      return kSIGTRAP;

    case EXCEPTION_DATATYPE_MISALIGNMENT:
      return kSIGBUS;

    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
      return kSIGFPE;

    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_PRIV_INSTRUCTION:
      return kSIGILL;

    case DBG_CONTROL_C:
      return kSIGINT;
  }
  return kSIGNO;
}

}  // namespace rsp

