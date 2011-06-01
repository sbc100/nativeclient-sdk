// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_DEBUG_SERVER_DEBUG_SERVER_H_
#define DEBUGGER_DEBUG_SERVER_DEBUG_SERVER_H_

#include "debugger/base/debug_socket.h"
#include "debugger/core/debug_api.h"
#include "debugger/core/debug_execution_engine.h"
#include "debugger/core/debug_logger.h"
#include "debugger/rsp/rsp_packetizer.h"
#include "debugger/rsp/rsp_packet.h"

namespace debug {
class DebugEvent;
class DebuggeeProcess;

class DebugServer : public rsp::PacketConsumer, public rsp::PacketVisitor {
public:
  DebugServer();
  ~DebugServer();

  bool ListenOnPort(int port);
  bool StartProcess(const char* cmd, const char* work_dir);
  void DoWork(int wait_ms);
  void Quit();

 private:
  // inherited from rsp::PacketConsumer
  virtual void OnPacket(const Blob& body, bool valid_checksum);
  virtual void OnUnexpectedChar(char unexpected_char);
  virtual void OnBreak();

  // inherited from rsp::PacketVisitor
  virtual void Visit(rsp::GetStopReasonCommand* packet);
  virtual void Visit(rsp::ContinueCommand* packet);
  virtual void Visit(rsp::QuerySupportedCommand* packet);
  virtual void Visit(rsp::QXferFeaturesReadCommand* packet);
  virtual void Visit(rsp::SetCurrentThreadCommand* packet);
  virtual void Visit(rsp::ReadMemoryCommand* packet);
  virtual void Visit(rsp::WriteMemoryCommand* packet);
  virtual void Visit(rsp::ReadRegistersCommand* packet);
  virtual void Visit(rsp::WriteRegistersCommand* packet);
  virtual void Visit(rsp::GetCurrentThreadCommand* packet);
  virtual void Visit(rsp::StepCommand* packet);
  virtual void Visit(rsp::IsThreadAliveCommand* packet);
  virtual void Visit(rsp::GetThreadInfoCommand* packet);

  void OnHaltedProcess(IDebuggeeProcess* halted_process,
                       const DebugEvent& debug_event);
  void PostRspMessage(const rsp::Packet& msg);
  void OnUnknownCommand();
  bool ReadGdbRegisters(Blob* blob);
  void GdbRegistersToCONTEXT(const Blob& gdb_regs, CONTEXT* ct);

 private:
  bool connection_was_established_;
  ListeningSocket listening_socket_;
  Socket debugger_connection_;
  rsp::Packetizer rsp_packetizer_;
  ExecutionEngine* execution_engine_;
  DebugAPI debug_api_;
  Logger* logger_;

  int focused_process_id_;
  int focused_thread_id_;
  int main_nexe_thread_id_;
};
}  // namespace debug
#endif  // DEBUGGER_DEBUG_SERVER_DEBUG_SERVER_H_
