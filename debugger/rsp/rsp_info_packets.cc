// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_info_packets.h"

namespace rsp {
void QuerySupportedCommand::AddFeature(const std::string& name,
                                       const std::string& value) {
  features_.push_back(std::pair<std::string, std::string>(name, value));
}

/// Example: "qSupported" + "xmlRegisters=i386;qRelocInsn+"
bool QuerySupportedCommand::FromBlob(const std::string& type,
                                     debug::Blob* message) {
  // First, split into "name=value" statements.
  std::deque<debug::Blob> statements;
  message->Split(debug::Blob().FromString(";"), &statements);

  std::deque<debug::Blob>::iterator it = statements.begin();
  while (it != statements.end()) {
    std::deque<debug::Blob> tokens;
    // Now, split each "name=value" statement into "name" + "value".
    it->Split(debug::Blob().FromString("="), &tokens);
    it++;
    RemoveSpacesFromBothEnds(&tokens);
    if (tokens.size() == 2) {
      AddFeature(tokens[0].ToString(), tokens[1].ToString());
    } else if (tokens.size() == 1) {
      // Handles case of "name+" statements.
      debug::Blob& name = tokens[0];
      if (name.size() > 0) {
        // Pop '+' or '-'
        char last_char = name.PopBack();
        if (('-' == last_char) || ('+' == last_char)) {
          std::string value;
          value.append(1, last_char);
          AddFeature(name.ToString(), value);
        } else {
          return false;
        }
      }
    } else {
      return false;
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
  for (size_t i = 0; i < num; i++) {
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

void QuerySupportedReply::ToBlob(debug::Blob* message) const {
  SaveFeaturesToBlob(message);
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
  bool r1 = PopIntFromFront(&offs, &offset_);
  bool r2 = PopIntFromFront(message, &length_);
  return (r1 && r2);
}

void QXferFeaturesReadCommand::ToBlob(debug::Blob* message) const {
  Format(message, "%s%s:%x,%x", kPrefix, file_name_.c_str(), offset_, length_);
}

bool QXferReply::FromBlob(const std::string& type, debug::Blob* message) {
  if (message->size() < 2)
    return false;
  uint8_t cmd = message->PopFront();
  if (('l' != cmd) && ('m' != cmd))
    return false;

  eom_ = ('l' == cmd);
  body_ = message->ToString();
  return true;
}

void QXferReply::ToBlob(debug::Blob* message) const {
  Format(message, (eom_ ? "l%s" : "m%s"), body_.c_str());
}
}  // namespace rsp

