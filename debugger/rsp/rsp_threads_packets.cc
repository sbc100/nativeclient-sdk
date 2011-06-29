// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_threads_packets.h"

namespace rsp {
bool SetCurrentThreadCommand::FromBlob(const std::string& type,
                                       debug::Blob* message) {
  if ((message->size() > 0) &&
     ('-' == message->GetAt(0)) &&
     ('1' == message->GetAt(1))) {
    thread_id_ = -1;  // all threads
    return true;
  }
  return PopIntFromFront(message, &thread_id_);
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

bool GetThreadInfoReply::FromBlob(const std::string& type,
                                  debug::Blob* message) {
  if (message->size() < 1)
    return false;
  char cmd = message->PopFront();
  eom_ = ('l' == cmd);

  std::deque<debug::Blob> tokens;
  message->Split(debug::Blob().FromString(","), &tokens);

  for (size_t i = 0; i < tokens.size(); i++) {
    uint32_t id = 0;
    if (!PopIntFromFront(&tokens[i], &id))
      return false;
    threads_ids_.push_back(id);
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

