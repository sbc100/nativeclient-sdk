// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/nacl-gdb_server/debug_server.h"
#include <stdio.h>
#include <signal.h>
#include <algorithm>
#include "debugger/core/debuggee_thread.h"
#include "debugger/rsp/rsp_common_replies.h"
#include "debugger/rsp/rsp_control_packets.h"
#include "debugger/rsp/rsp_info_packets.h"
#include "debugger/rsp/rsp_packet_utils.h"
#include "debugger/rsp/rsp_packets.h"
#include "debugger/rsp/rsp_threads_packets.h"
#include "debugger/nacl-gdb_server/gdb_registers.h"

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
const int kErrorThreadNotFound = 11;

rsp::StopReply CreateStopReplyFromDebugEvent(const debug::DebugEvent& de) {
	switch (de.process_state_) {
		case debug::RUNNING: return rsp::StopReply(rsp::StopReply::STILL_RUNNING);
		case debug::PROCESS_STOPPED: {
			rsp::StopReply msg(rsp::StopReply::SIGNALED);
			msg.set_signal_number(de.signal_no_);
			return msg;
		}
		case debug::PROCESS_TERMINATED: {
			rsp::StopReply msg(rsp::StopReply::TERMINATED);
			msg.set_signal_number(de.signal_no_);
			return msg;
		}
		case debug::PROCESS_EXITED: {
			rsp::StopReply msg(rsp::StopReply::EXITED);
			msg.set_exit_code(de.exit_code_);
			return msg;
		}
	}
	return rsp::StopReply(rsp::StopReply::EXITED);
}

#define AAA(x) printf("\t%s = 0x%lx\n", #x, context.x)

void PrintContext(const user_regs_struct& context) {
	printf("Context {\n");
	AAA(r15);
	AAA(r14);
	AAA(r13);
	AAA(r12);
	AAA(rbp);
	AAA(rbx);
	AAA(r11);
	AAA(r10);
	AAA(r9);
	AAA(r8);
	AAA(rax);
	AAA(rcx);
	AAA(rdx);
	AAA(rsi);
	AAA(rdi);
	AAA(rip);
	AAA(cs);
	AAA(eflags);
	AAA(rsp);
	AAA(ss);
	AAA(ds);
	AAA(es);
	AAA(fs);
	AAA(gs);
	printf("}\n");
}

}  // namespace

namespace debug {
DebugServer::DebugServer(DebugAPI* debug_api, int listen_port)
		: debug_api_(debug_api),
      client_connected_(false),
			debuggee_process_(NULL),
      focused_thread_id_(0),
      listen_port_(listen_port),
      continue_from_halt_(false) {
}

DebugServer::~DebugServer() {
	if (NULL != debuggee_process_) {
		delete debuggee_process_;
		debuggee_process_ = NULL;
	}
}

bool DebugServer::Init() {
  rsp_packetizer_.SetPacketConsumer(this);
	debuggee_process_ = new DebuggeeProcess(debug_api_);
	if (NULL == debuggee_process_)
    return false;

	return ListenForRspConnection();
}

bool DebugServer::ListenForRspConnection() {
  bool res = listening_socket_.Listen(listen_port_);
  if (res)
		printf("Started listening on port %d ...\n", listen_port_);
  else
		printf("DebugServer::ListenForRspConnection failed port=%d",
					 listen_port_);

  return res;
}

void DebugServer::HandleNetwork() {
  if (!client_connection_.IsConnected()) {
    if (client_connected_) {
      client_connected_ = false;
			printf("Dropped connection from debugger.\n");
    }
		if (listening_socket_.Accept(20, &client_connection_)) {
      client_connected_ = true;
			printf("Got connection from debugger.\n");
    }
  } else {
    char buff[kReadBufferSize];
    for (int i = 0; i < kMaxPacketsToReadAtOnce; i++) {
      size_t read_bytes = client_connection_.Read(buff,
                                                  sizeof(buff) - 1,
																									20);
      if (read_bytes > 0) {
        buff[read_bytes] = 0;
				printf("r>[%s]\n", buff);
        rsp_packetizer_.OnData(buff, read_bytes);
      } else {
        break;
      }
    }
  }
}

void DebugServer::HandleExecutionEngine() {
	DebugEvent de;
	if (debuggee_process_->WaitForDebugEventAndDispatchIt(&de)) {
		// Now, we have a halted NaCl process on our hands.
		if (0 == focused_thread_id_)
			focused_thread_id_ = de.pid_;
		if (continue_from_halt_) {
			continue_from_halt_ = false;
			SendRspMessageToClient(CreateStopReplyFromDebugEvent(de));
		}
	}
}

void DebugServer::DoWork() {
  HandleNetwork();
	HandleExecutionEngine();
}

void DebugServer::SendRspMessageToClient(const rsp::Packet& msg) {
  Blob text;
  msg.ToBlob(&text);
  Blob wire_msg;
  rsp::PacketUtils::AddEnvelope(text, &wire_msg);
  client_connection_.WriteAll(wire_msg);
	printf("T>[%s]\n", text.ToString().c_str());
	printf("t>[%s]\n", wire_msg.ToString().c_str());
}

DebuggeeThread* DebugServer::GetFocusedThread() {
	DebuggeeThread* thread = debuggee_process_->GetThread(focused_thread_id_);
	if (NULL == thread)
		SendErrorReply(kErrorNoFocusedThread);
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
	printf("msg='DebugServer::OnUnexpectedChar' c='0x%x'",
				 static_cast<int>(unexpected_byte));
}

void DebugServer::SendErrorReply(int error) {
  SendRspMessageToClient(rsp::ErrorReply(error));
}

void DebugServer::OnBreak() {
	debug_api_->PostSignal(focused_thread_id_, SIGSTOP);
}

void DebugServer::Visit(rsp::GetStopReasonCommand* packet) {
	DebuggeeThread* thread = GetFocusedThread();
	if (NULL == thread)
    return;
	SendRspMessageToClient(CreateStopReplyFromDebugEvent(thread->last_debug_event()));
}

void DebugServer::Visit(rsp::ContinueCommand* packet) {
	DebuggeeThread* thread = GetFocusedThread();
	if (NULL != thread) {
    continue_from_halt_ = true;
		debuggee_process_->Continue(focused_thread_id_);
  }
}

void DebugServer::Visit(rsp::QuerySupportedCommand* packet) {
  rsp::QuerySupportedReply reply;
	reply.AddFeature("PacketSize", "7cf");
  reply.AddFeature("qXfer:libraries:read", "+");
  reply.AddFeature("qXfer:features:read", "+");
  SendRspMessageToClient(reply);
}

void DebugServer::Visit(rsp::QXferFeaturesReadCommand* packet) {
  // qXfer:features:read:target.xml:0,7ca
  if (packet->file_name() == "target.xml") {
    rsp::QXferReply reply;
    reply.set_body("<target><architecture>i386:x86-64</architecture></target>");
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
    res = true;
  } else if (0 == tid) {  // any thread
    res = true;
  } else {
		DebuggeeThread* thread = debuggee_process_->GetThread(tid);
		if (NULL != thread) {
			res = true;
			focused_thread_id_ = tid;
		}
  }
  if (res)
    SendRspMessageToClient(rsp::OkReply());
  else
    SendErrorReply(kErrorThreadNotFound);
}

void DebugServer::Visit(rsp::ReadMemoryCommand* packet) {
	size_t sz = packet->num_of_bytes();
  if (0 == sz) {
    SendRspMessageToClient(rsp::EmptyPacket());
    return;
  }
  char buff[rsp::kMaxRspPacketSize / 2];  // 2 characters per byte.
  if (sizeof(buff) < sz)
    sz = sizeof(buff);

	DebuggeeThread* thread = GetFocusedThread();
	if (NULL != thread) {
		uint64_t addr = packet->addr();
    // massage address to support gdb
		if (addr < debuggee_process_->nexe_mem_base())
			addr += debuggee_process_->nexe_mem_base();

		size_t rd = 0;
		if (debug_api_->ReadMemory(focused_thread_id_, addr, buff, sz, &rd)) {
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
	DebuggeeThread* thread = GetFocusedThread();
	if (NULL != thread) {
    char tmp[rsp::kMaxRspPacketSize];
    if (data.size() > sizeof(tmp)) {
      SendErrorReply(kErrorPacketIsTooLarge);
      return;
    }
    data.Peek(0, tmp, data.size());
		uint64_t addr = packet->addr();
    // massage address to support gdb
		if (addr < debuggee_process_->nexe_mem_base())
			addr += debuggee_process_->nexe_mem_base();

		size_t wr = 0;
		bool res = debug_api_->WriteMemory(focused_thread_id_, addr, tmp, data.size(), &wr);
    if (res)
      SendRspMessageToClient(rsp::OkReply());
    else
      SendErrorReply(kErrorWriteMemoryFailed);
  }
}

void DebugServer::Visit(rsp::ReadRegistersCommand* packet) {
	DebuggeeThread* thread = GetFocusedThread();
  if (NULL != thread) {
	 user_regs_struct context;
	 if (!debug_api_->ReadThreadContext(thread->id(), &context)) {
		 SendErrorReply(kErrorGetThreadContextFailed);
	 } else {
		 PrintContext(context);
		 Blob blob;
		 rsp::CONTEXTToGdbRegisters(context, &blob);
		 rsp::BlobReply reply;
		 reply.set_data(blob);
		 SendRspMessageToClient(reply);
	 }
  }
}

void DebugServer::Visit(rsp::WriteRegistersCommand* packet) {
  DebuggeeThread* thread = GetFocusedThread();
  if (NULL != thread) {
		user_regs_struct context;
		if (!debug_api_->ReadThreadContext(thread->id(), &context)) {
      SendErrorReply(kErrorGetThreadContextFailed);
      return;
    }
		rsp::GdbRegistersToCONTEXT(packet->data(), &context);
		if (!debug_api_->WriteThreadContext(thread->id(), &context))
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
	DebuggeeThread* thread = GetFocusedThread();
	if (NULL != thread) {
		if (!debug_api_->SingleStep(focused_thread_id_)) {
      SendErrorReply(kErrorSingleStepFailed);
		} else {
			thread->Continue();
			continue_from_halt_ = true;
		}
  }
}

void DebugServer::Visit(rsp::IsThreadAliveCommand* packet) {
	if (NULL != debuggee_process_->GetThread(packet->value()))
    SendRspMessageToClient(rsp::OkReply());
	else
    SendErrorReply(kErrorThreadNotFound);
}

void DebugServer::Visit(rsp::GetThreadInfoCommand* packet) {
  rsp::GetThreadInfoReply reply;
  if (packet->get_more()) {
    // TODO(garianov): add support for multi-packet replies.
    reply.set_eom(true);
  } else {
    std::deque<int> tids;
		debuggee_process_->GetThreadsIds(&tids);
		reply.set_threads_ids(tids);
    reply.set_eom(false);
  }
  SendRspMessageToClient(reply);
}

void DebugServer::Visit(rsp::GetOffsetsCommand* packet) {
	uint64_t mb = debuggee_process_->nexe_mem_base();
  rsp::GetOffsetsReply reply;
  reply.set_data_offset(mb);
  reply.set_text_offset(mb);

  SendRspMessageToClient(reply);
}

}  // namespace debug

