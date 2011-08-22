// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_control_packets.h"

namespace rsp {
ReadMemoryCommand::ReadMemoryCommand()
    : addr_(0),
      num_of_bytes_(0) {
}

// Example: "m" + "cffffff80,40"
bool ReadMemoryCommand::FromBlob(const std::string& type,
                                 debug::Blob* message) {
  std::deque<debug::Blob> tokens;
  message->Split(debug::Blob().FromString(","), &tokens);
  if (tokens.size() < 2)
    return false;

  bool r1 = PopIntFromFront(&tokens[0], &addr_);
  bool r2 = PopIntFromFront(&tokens[1], &num_of_bytes_);
  return r1 && r2;
}

void ReadMemoryCommand::ToBlob(debug::Blob* message) const {
  Format(message, "m%I64x,%x", addr_, num_of_bytes_);
}

WriteMemoryCommand::WriteMemoryCommand()
    : addr_(0) {
}

bool WriteMemoryCommand::FromBlob(const std::string& type,
                                  debug::Blob* message) {
  // example: H>[Mc00020304,1:8b]
  std::deque<debug::Blob> tokens;
  message->Split(debug::Blob().FromString(","), &tokens);
  if (tokens.size() < 2)
    return false;

  if (!PopIntFromFront(&tokens[0], &addr_))
    return false;
  debug::Blob len_and_data = tokens[1];
  len_and_data.Split(debug::Blob().FromString(":"), &tokens);
  if (tokens.size() < 2)
    return false;
  return data_.FromHexString(tokens[1].ToString());
}

void WriteMemoryCommand::ToBlob(debug::Blob* message) const {
  std::string hex_blob = data_.ToHexStringNoLeadingZeroes();
  Format(message, "M%I64x,%x:%s", addr_, data_.size(), hex_blob.c_str());
}

bool WriteRegistersCommand::FromBlob(const std::string& type,
                                     debug::Blob* message) {
  return data_.FromHexString(message->ToString());
}

void WriteRegistersCommand::ToBlob(debug::Blob* message) const {
  message->FromString("G");
  message->Append(debug::Blob().FromString(data_.ToHexString()));
}
}  // namespace rsp

