// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_common_replies.h"

namespace rsp {
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

bool StopReply::FromBlob(const std::string& type, debug::Blob* message) {
  if ("S" == type) {
    stop_reason_ = SIGNALED;
    return PopIntFromFront(message, &signal_number_);
  } else if ("W" == type) {
    stop_reason_ = StopReply::EXITED;
    if (!PopIntFromFront(message, &exit_code_))
      return false;
  } else if ("X" == type) {
    stop_reason_ = StopReply::TERMINATED;
    if (!PopIntFromFront(message, &signal_number_))
      return false;
  } else if ("O" == type) {
    stop_reason_ = StopReply::STILL_RUNNING;
    return true;
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
      return PopIntFromFront(&tokens[1], &pid_);
    }
  }
  return false;
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

bool BlobReply::FromBlob(const std::string& type, debug::Blob* message) {
  return data_.FromHexString(message->ToString());
}

void BlobReply::ToBlob(debug::Blob* message) const {
  message->FromString(data_.ToHexString());
}
}  // namespace rsp

