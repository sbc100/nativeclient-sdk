// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_NACL_GDB_SERVER_DEBUG_SERVER_H_
#define DEBUGGER_NACL_GDB_SERVER_DEBUG_SERVER_H_

#include "debugger/base/debug_socket.h"
#include "debugger/core/debug_api.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/rsp/rsp_packetizer.h"
#include "debugger/rsp/rsp_packets.h"

namespace debug {
class DebugEvent;
class DebuggeeThread;

/// Class implementing the GDB RSP debug server for the NaCl applications.
/// It creates illusion to the client (debugger), that it debugs NaCl
/// application (aka nexe), hidding all other intermediate processes,
/// such as chrome + sel_ldr. It also hides all non-nexe threads.
/// For example of how to use it, look in nacl-gdb_server.cc.
///
/// Note: current implementation has roughly the same set of features as
/// in-process debug stub (major difference is the support of Win32).
/// In order to be compatible with current VisualStudio debugger plugin,
/// it's also limited to one nexe per session.
class DebugServer : public rsp::PacketConsumerInterface,
    public rsp::PacketVisitor {
 public:
  /// @param[in] debug_api pointer to DebugAPI object.
  /// DebugServer  doesn't take ownership of |debug_api|.
  /// @param[in] listen_port port to listen on (for RSP client).
  explicit DebugServer(DebugAPI* debug_api, int listen_port);
  ~DebugServer();

  /// Initializes internal objects.
  /// @return true if operation succeeds.
  bool Init();

  /// Performs duty of debug server for no more than |wait_ms| milliseconds.
  /// 1) Checks for incoming connection from the client (i.e. debugger).
  /// 2) If connection is up, checks for incoming messages from the client.
  /// 3) If RSP message has arrived, calls one of the rsp::PacketVisitor::Visit
  ///    methods (indirectly, see Visitor pattern).
  /// 4) Calls ExecutionEngine::WaitForDebugEventAndDispatchIt method, handles
  ///    incoming debugger events.
	void DoWork();

 protected:
  /// Starts listening for incoming RSP connection.
  /// @return true if operation succeeds.
  bool ListenForRspConnection();

	void HandleNetwork();
	void HandleExecutionEngine();
  void SendRspMessageToClient(const rsp::Packet& msg);
  void SendErrorReply(int error);

  /// @return thread that has a user focus, i.e. thread that is used
  /// in the processing of RSP messages.
  /// Side effects: if there's no focused thread, sends error replies to
  /// the client (i.e. debugger).
  DebuggeeThread* GetFocusedThread();

  void OnUnknownCommand();

  // Inherited from rsp::PacketConsumer, see rsp/packets.h for more information.
  virtual void OnPacket(const Blob& body, bool valid_checksum);
  virtual void OnUnexpectedByte(uint8_t unexpected_byte);
  virtual void OnBreak();

  // Inherited from rsp::PacketVisitor
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
  virtual void Visit(rsp::GetOffsetsCommand* packet);

 protected:
	DebugAPI* debug_api_;
  ListeningSocket listening_socket_;
  Socket client_connection_;
  bool client_connected_;
  rsp::Packetizer rsp_packetizer_;
	DebuggeeProcess* debuggee_process_;
  int focused_thread_id_;
  int listen_port_;

  // So that we don't send unsolicited rsp::StopReply.
  bool continue_from_halt_;
};
}  // namespace debug
#endif  // DEBUGGER_NACL_GDB_SERVER_DEBUG_SERVER_H_

