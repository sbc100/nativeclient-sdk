// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_common_replies.h"
#include "debugger/rsp/rsp_control_packets.h"
#include "debugger/rsp/rsp_info_packets.h"
#include "debugger/rsp/rsp_packets.h"
#include "debugger/rsp/rsp_threads_packets.h"

namespace {
std::map<std::string, rsp::Packet*> packet_factory_;

/// Pops type field from the front of |message|.
/// @param[in,out] message RSP packet
/// @param[in] type_hint optional type hint or NULL
/// @return string with message type
std::string PopPacketType(debug::Blob* message, const char* type_hint) {
  if (packet_factory_.size() == 0)
    rsp::Packet::InitPacketFactory();

  // If |type_hint| is supplied, then message don't have type field,
  // and there's no point in trying to find it.
  if (NULL != type_hint)
    return type_hint;

  // Most commands have no termination, i.e. they are prefixes,
  // and they should be looked up as we scan characters. The shortest
  // command to match wins.
  std::string type;
  while (message->size() != 0) {
    char c = message->Front();
    message->PopFront();
    type.append(1, c);
    if (packet_factory_.find(type) != packet_factory_.end())
      break;
  }
  // If packet type is terminated by ':', we should pop it off.
  message->PopMatchingBytesFromFront(debug::Blob().FromString(" :"));
  return type;
}
}  // namespace

namespace rsp {
Packet* Packet::CreateFromBlob(debug::Blob* message, const char* type_hint) {
  RemoveSpacesFromBothEnds(message);

  // Empty message can be a reply to 'm' command, or
  // can indicate that command is not supported.
  if ((0 == message->size()) && (NULL == type_hint))
    return new EmptyPacket;

  // To differentiate [Xc00020304,0:] from [X05] or [X21;process:138c]
  // I.e. we need to detect if it's a 'X' command (write data to memory, where
  // the data is transmitted in binary) or 'X' reply (the process terminated
  // with signal).
  if ((message->size() > 0) &&
     ('X' == message->Front()) &&
     (message->HasByte(',')))
    return NULL;  // we don't support 'X' command.

  // To differentiate it from 'O' packet (StopReply).
  if (*message == debug::Blob().FromString("OK"))
    return new OkReply;

  std::string type = PopPacketType(message, type_hint);
  if (packet_factory_.find(type) == packet_factory_.end())
    return NULL;

  Packet* packet = packet_factory_[type]->Clone();
  if ((NULL != packet) && (!packet->FromBlob(type, message))) {
    delete packet;
    packet = NULL;
  }
  return packet;
}

void Packet::InitPacketFactory() {
  packet_factory_["qSupported"] = new rsp::QuerySupportedCommand;
  packet_factory_["qSupported$Reply"] = new rsp::QuerySupportedReply;
  packet_factory_["?"] = new rsp::GetStopReasonCommand;
  packet_factory_["S"] = new rsp::StopReply(rsp::StopReply::SIGNALED);
  packet_factory_["X"] = new rsp::StopReply(rsp::StopReply::TERMINATED);
  packet_factory_["W"] = new rsp::StopReply(rsp::StopReply::EXITED);
  packet_factory_["O"] = new rsp::StopReply(rsp::StopReply::STILL_RUNNING);
  packet_factory_["m"] = new rsp::ReadMemoryCommand;
  packet_factory_["M"] = new rsp::WriteMemoryCommand;
  packet_factory_["blob$Reply"] = new rsp::BlobReply;
  packet_factory_["g"] = new rsp::ReadRegistersCommand;
  packet_factory_["G"] = new rsp::WriteRegistersCommand;
  packet_factory_["E"] = new rsp::ErrorReply;
  packet_factory_["OK"] = new rsp::OkReply;
  packet_factory_["Hg"] =
      new rsp::SetCurrentThreadCommand(rsp::SetCurrentThreadCommand::FOR_READ);
  packet_factory_["Hc"] =
      new rsp::SetCurrentThreadCommand(
          rsp::SetCurrentThreadCommand::FOR_CONTINUE);
  packet_factory_["qC"] = new rsp::GetCurrentThreadCommand;
  packet_factory_["QC"] = new rsp::GetCurrentThreadReply;
  packet_factory_["c"] = new rsp::ContinueCommand;
  packet_factory_["s"] = new rsp::StepCommand;
  packet_factory_["T"] = new rsp::IsThreadAliveCommand;
  packet_factory_["qXfer:features:read:"] = new rsp::QXferFeaturesReadCommand;
  packet_factory_["qXfer$Reply"] = new rsp::QXferReply;
  packet_factory_["qfThreadInfo"] = new rsp::GetThreadInfoCommand;
  packet_factory_["qsThreadInfo"] = new rsp::GetThreadInfoCommand;
  packet_factory_["GetThreadInfo$Reply"] = new rsp::GetThreadInfoReply;
}

void Packet::FreePacketFactory() {
  std::map<std::string, rsp::Packet*>::const_iterator it =
      packet_factory_.begin();
  while (it != packet_factory_.end()) {
    delete it->second;
    ++it;
  }
  packet_factory_.clear();
}
}  // namespace rsp

