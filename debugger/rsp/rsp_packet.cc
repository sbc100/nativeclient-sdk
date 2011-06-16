// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_packet.h"

namespace {
void InitFactory();
std::map<std::string, rsp::Packet*> packet_factory_;
}  // namespace

namespace rsp {
/// Note: this method modifies |message|.
/// Note: returned obj shall be deleted.
///
/// Why |type_hint|? Because some RSP packets don't carry type field.
/// Examples:
/// 1. replies to 'm' (read memory) command:
/// H>[mc000202e0,12]
/// T>[554889e583ec204c01fc897dec8975e8c745]
/// => supply "blob$Reply" as a |type_hint|
/// 
/// 2. replies to 'g' (read registers) command:
/// H>[g]
/// T>[00000000000000000000000000000000d85123...
/// => supply "blob$Reply" as a |type_hint|
///  
/// 3. replies to 'qSupported' command:
/// H>[qSupported:xmlRegisters=i386;qRelocInsn+]
/// T>[PacketSize=7cf;qXfer:libraries:read+;qXfer:features:read+]
/// => supply "qSupported$Reply" as a |type_hint|
///
/// 4. replies to 'qXfer' command:
/// supply "qXfer$Reply" as a |type_hint|
///
Packet* Packet::CreateFromBlob(debug::Blob* message, const char* type_hint) {
  if (packet_factory_.size() == 0)
    InitFactory();

  // defferentiate [Xc00020304,0:] from [X05]
  if ((message->size() > 0) && ('X' == message->Front())) {
    if (message->size() > 3)
      return NULL;
  }

  RemoveSpacesFromBothEnds(message);

  // Empty message can be a reply to 'm' command, or
  // can indicate that command is not supported.
  if ((0 == message->size()) && (NULL == type_hint))
    return new EmptyPacket;

  std::string type;
  bool general_query = false;

  if (NULL != type_hint) {
    type = type_hint;
  } else {
    while (message->size() != 0) {
      char c = message->Front();
      message->PopFront();
      if (general_query && ((0 == c) || (':' == c)))
        break;
      type.append(1, c);

      // Commands starting from 'q' or 'Q' are 'general query', and full command
      // name is terminated by ':' or <eom>.
      // All other commands have no termination, i.e. they are prefixes,
      // and they should be looked up as we scan characters. The shortest command
      // to match wins.
      if (!general_query) {
        if (packet_factory_.find(type) != packet_factory_.end())
          break;
      }
    }
  } 

  Packet* packet_template = packet_factory_[type];
  if (NULL != packet_template) {
    Packet* packet = packet_template->Create();
    if (NULL != packet) {
      message->PopMatchingBytesFromFront(debug::Blob().FromString(" :"));
      if (packet->FromBlob(type, message))
        return packet;
      else
        delete packet;
    }
  }        
  return NULL;
}

//----------------------------------------------------------//
void EmptyPacket::AcceptVisitor(PacketVisitor* vis) {
  vis->Visit(this);
}

Packet* EmptyPacket::Create() const {
  return new EmptyPacket;
}

bool EmptyPacket::FromBlob(const std::string& type, debug::Blob* message) {
  return true;
}

void EmptyPacket::ToBlob(debug::Blob* message) const {
  message->Clear();
}

//----------------------------------------------------------//
Packet* QuerySupportedCommand::Create() const {
  return new QuerySupportedCommand;
}

void QuerySupportedCommand::AcceptVisitor(PacketVisitor* vis) {
  vis->Visit(this);
}

void QuerySupportedCommand::AddFeature(const std::string& name,
                                       const std::string& value) {
  features_.push_back(std::pair<std::string, std::string>(name, value));
} 

bool QuerySupportedCommand::FromBlob(const std::string& type,
                                     debug::Blob* message) {
  std::deque<debug::Blob> statements;
  message->Split(debug::Blob().FromString(";"), &statements);
  std::deque<debug::Blob>::iterator it = statements.begin();
  while (it != statements.end()) {
    std::deque<debug::Blob> tokens;
    it->Split(debug::Blob().FromString("="), &tokens);
    it++;
    RemoveSpacesFromBothEnds(&tokens);
    if (tokens.size() >= 2) {
      AddFeature(tokens[0].ToString(), tokens[1].ToString());
    } else if (tokens.size() > 0) {
      debug::Blob& name = tokens[0];
      if (name.size() > 0) {
        char last_char = name.Back();
        name.PopBack();
        std::string value;
        value.append(1, last_char);
        AddFeature(name.ToString(), value);
      }
    }
  }
  return true;
}

size_t QuerySupportedCommand::GetFeaturesNum() const {
  return features_.size();
}

std::string QuerySupportedCommand::GetFeatureName(size_t pos) const {
  if (pos >= GetFeaturesNum())
    return "";
  return features_[pos].first;
}

std::string QuerySupportedCommand::GetFeature(const std::string& name) const {
  for (size_t i = 0; i < GetFeaturesNum(); i++) {
    if (name == GetFeatureName(i))
      return features_[i].second;
  }
  return "";
}

void QuerySupportedCommand::SaveFeaturesToBlob(debug::Blob* message) const {
  size_t num = features_.size();
  for (size_t i = 0; i < num; i++ ) {
    const std::pair<std::string, std::string>& feature = features_[i];
    if (0 != i)
      message->Append(debug::Blob().FromString(";"));
    message->Append(debug::Blob().FromString(feature.first));
    if (("+" != feature.second) &&
       ("-" != feature.second) &&
       ("?" != feature.second))
      message->Append(debug::Blob().FromString("="));
    message->Append(debug::Blob().FromString(feature.second));
  }
}

void QuerySupportedCommand::ToBlob(debug::Blob* message) const {
  message->Append(debug::Blob().FromString("qSupported:"));
  SaveFeaturesToBlob(message);
}

//----------------------------------------------------------//
Packet* QuerySupportedReply::Create() const {
  return new QuerySupportedReply;
}

void QuerySupportedReply::AcceptVisitor(PacketVisitor* vis) {
  vis->Visit(this);
}

void QuerySupportedReply::ToBlob(debug::Blob* message) const {
  SaveFeaturesToBlob(message);
}

//----------------------------------------------------------//
StopReply::StopReply()
    : stop_reason_(STILL_RUNNING), 
      signal_number_(0),
      exit_code_(0),
      pid_(0) {
}

StopReply :: StopReply(StopReason stop_reason)
    : stop_reason_(stop_reason),
      signal_number_(0),
      exit_code_(0),
      pid_(0) {
}

Packet* StopReply::Create() const {
  return new StopReply(stop_reason_);
}

void StopReply::AcceptVisitor(PacketVisitor* vis) {
  vis->Visit(this);
}

bool StopReply::FromBlob(const std::string& type, debug::Blob* message) {
  if ("S" == type) {
    stop_reason_ = SIGNALED;
    std::string s = message->ToString();
    signal_number_ = PopInt8FromFront(message);
  } else if ("W" == type) {
    stop_reason_ = StopReply::EXITED;
    exit_code_ = PopInt8FromFront(message);
  } else if ("X" == type) {
    stop_reason_ = StopReply::TERMINATED;
    signal_number_ = PopInt8FromFront(message);
  } else if ("O" == type) {
    stop_reason_ = StopReply::STILL_RUNNING;
  } else {
    return false;
  }

  if (("W" == type) || ("X" == type)) {
    message->PopMatchingBytesFromFront(debug::Blob().FromString(";"));
    std::deque<debug::Blob> tokens;
    message->Split(debug::Blob().FromString(":"), &tokens);
    if ((tokens.size() >= 2) &&
       (tokens[0] == debug::Blob().FromString("process"))) {
      std::string s = tokens[1].ToString();
      pid_ = PopInt32FromFront(&tokens[1]);
    }
  }

  return true;
}

void StopReply::ToBlob(debug::Blob* message) const {
  message->Clear();
  switch (stop_reason_) {
    case SIGNALED: {
      Format(message, "S%0.2x", signal_number_);
      break;
    }
    case TERMINATED: {
      Format(message, "X%0.2x;process:%x", signal_number_, pid_);
      break;
    }
    case EXITED: {
      Format(message, "W%0.2x;process:%x", exit_code_, pid_);
      break;
    }
    case STILL_RUNNING: {
      message->FromString("O");
      break;
    }
  }
}

//------------------------------------------------//
ReadMemoryCommand::ReadMemoryCommand() 
    : addr_(0),
      num_of_bytes_(0) {
}

Packet* ReadMemoryCommand::Create() const {
  return new ReadMemoryCommand;
}

void ReadMemoryCommand::AcceptVisitor(PacketVisitor* vis) {
  vis->Visit(this);
}

bool ReadMemoryCommand::FromBlob(const std::string& type,
                                 debug::Blob* message) {
// example: H>[mcffffff80,40] (with first 'm' removed)
  std::deque<debug::Blob> tokens;
  message->Split(debug::Blob().FromString(","), &tokens);
  if (tokens.size() < 2)
    return false;

  addr_ = PopInt64FromFront(&tokens[0]);
  num_of_bytes_ = static_cast<int>(PopInt32FromFront(&tokens[1]));
  return true;
}

void ReadMemoryCommand::ToBlob(debug::Blob* message) const {
  Format(message, "m%I64x,%x", addr_, num_of_bytes_);
}

WriteMemoryCommand::WriteMemoryCommand()
    : addr_(0) {
}

Packet* WriteMemoryCommand::Create() const {
  return new WriteMemoryCommand;
}

void WriteMemoryCommand::AcceptVisitor(PacketVisitor* vis) {
  vis->Visit(this);
}

bool WriteMemoryCommand::FromBlob(const std::string& type,
                                  debug::Blob* message) {
  // example: H>[Mc00020304,1:8b]
  std::deque<debug::Blob> tokens;
  message->Split(debug::Blob().FromString(","), &tokens);
  if (tokens.size() < 2)
    return false;

  addr_ = PopInt64FromFront(&tokens[0]);
  debug::Blob len_and_data = tokens[1];
  len_and_data.Split(debug::Blob().FromString(":"), &tokens);
  if (tokens.size() < 2)
    return false;
  return data_.FromHexString(tokens[1].ToString());
}

void WriteMemoryCommand::ToBlob(debug::Blob* message) const {
  std::string hex_blob = data_.ToHexStringNoLeadingZeroes();
  Format(message,"M%I64x,%x:%s", addr_, data_.size(), hex_blob.c_str());
}

Packet* BlobReply::Create() const {
  return new BlobReply;
}

void BlobReply::AcceptVisitor(PacketVisitor* vis) {
  vis->Visit(this);
}

bool BlobReply::FromBlob(const std::string& type, debug::Blob* message) {
  return data_.FromHexString(message->ToString());
}

void BlobReply::ToBlob(debug::Blob* message) const {
  message->FromString(data_.ToHexString());
}

bool WriteRegistersCommand::FromBlob(const std::string& type,
                                     debug::Blob* message) {
  return data_.FromHexString(message->ToString());
}

void WriteRegistersCommand::ToBlob(debug::Blob* message) const {
  message->FromString("G");
  message->Append(debug::Blob().FromString(data_.ToHexString()));
}

bool SetCurrentThreadCommand::FromBlob(const std::string& type, debug::Blob* message) {
  if ((message->size() > 0) &&
     ('-' == message->GetAt(0)) &&
     ('1' == message->GetAt(1)))
    thread_id_ = -1;  // all threads
  else 
    thread_id_ = PopInt32FromFront(message);
  return true;
}

void SetCurrentThreadCommand::ToBlob(debug::Blob* message) const {
  if (FOR_READ == subtype_)
    message->FromString("Hg");
  else
    message->FromString("Hc");

  if (0 == thread_id_) {
    message->PushBack('0');
  } else if (-1 == thread_id_) {
    message->PushBack('-');
    message->PushBack('1');
  } else {
    debug::Blob tmp;
    Format(&tmp, "%x", thread_id_);
    message->Append(tmp);
  }
}

const char* QXferFeaturesReadCommand::kPrefix = "qXfer:features:read:";

QXferFeaturesReadCommand::QXferFeaturesReadCommand()
    : offset_(0),
      length_(0) {
}

bool QXferFeaturesReadCommand::FromBlob(const std::string& type,
                                        debug::Blob* message) {
  // example: target.xml:0,7ca
  debug::Blob file_name =
      message->PopBlobFromFrontUntilBytes(debug::Blob().FromString(":"));
  debug::Blob offs =
      message->PopBlobFromFrontUntilBytes(debug::Blob().FromString(","));

  file_name_ = file_name.ToString();
  offset_ = PopInt32FromFront(&offs);
  length_ = PopInt32FromFront(message);
  return true;
}

void QXferFeaturesReadCommand::ToBlob(debug::Blob* message) const {
  Format(message, "%s%s:%x,%x", kPrefix, file_name_.c_str(), offset_, length_);
}

bool QXferReply::FromBlob(const std::string& type, debug::Blob* message) {
  if (message->size() < 1)
    return false;
  char cmd = message->PopFront();
  eom_ = ('l' == cmd);
  body_ = message->ToString();
  return true;
}

void QXferReply::ToBlob(debug::Blob* message) const {
  Format(message, (eom_ ? "l%s" : "m%s"), body_.c_str());
}

bool GetThreadInfoCommand::FromBlob(const std::string& type,
                                    debug::Blob* message) {
  get_more_ = ("qsThreadInfo" == type);
  return true;
}

void GetThreadInfoCommand::ToBlob(debug::Blob* message) const {
  if (get_more_)
    message->FromString("qsThreadInfo");
  else
    message->FromString("qfThreadInfo");
}

bool GetThreadInfoReply::FromBlob(const std::string& type, debug::Blob* message) {
  if (message->size() < 1)
    return false;
  char cmd = message->PopFront();
  eom_ = ('l' == cmd);

  std::deque<debug::Blob> tokens;
  message->Split(debug::Blob().FromString(","), &tokens);

  for (size_t i = 0; i < tokens.size(); i++) {
    uint32_t id = PopInt32FromFront(&tokens[i]);
    threads_ids_.push_back(static_cast<int>(id));
  }
  return true;
}

void GetThreadInfoReply::ToBlob(debug::Blob* message) const {
  if (eom_ || (0 == threads_ids_.size()))
    message->FromString("l");
  else
    message->FromString("m");
  size_t num = threads_ids_.size();
  for (size_t i = 0; i < num; i++) {
    debug::Blob tmp;
    Format(&tmp, "%x", threads_ids_[i]);
    message->Append(tmp);
    if ((i + 1) != num)
      message->PushBack(',');
  }
}
}  // namespace rsp

namespace {
void InitFactory() {
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
      new rsp::SetCurrentThreadCommand(rsp::SetCurrentThreadCommand::FOR_CONTINUE);
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
}  // namespace
