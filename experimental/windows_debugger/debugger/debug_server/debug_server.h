// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_DEBUG_SERVER_DEBUG_SERVER_H_
#define DEBUGGER_DEBUG_SERVER_DEBUG_SERVER_H_

#include "debugger/base/debug_socket.h"
#include "debugger/core/debug_api.h"
#include "debugger/core/debug_execution_engine.h"
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
  virtual void Visit(rsp::StopReply* packet);
  virtual void Visit(rsp::ContinueCommand* packet);

  void OnHaltedProcess(DebuggeeProcess* halted_process,
                       const DebugEvent& debug_event);
  void SendMessage(const rsp::Packet& msg);

  bool connection_was_established_;
  ListeningSocket listening_socket_;
  Socket debugger_connection_;
  rsp::Packetizer rsp_packetizer_;
  ExecutionEngine* execution_engine_;
  DebugAPI debug_api_;
};
}  // namespace debug
#endif  // DEBUGGER_DEBUG_SERVER_DEBUG_SERVER_H_
