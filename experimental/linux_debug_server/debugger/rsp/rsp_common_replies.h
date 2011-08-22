// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_COMMON_REPLIES_H_
#define DEBUGGER_RSP_RSP_COMMON_REPLIES_H_
#include <string>
#include "debugger/rsp/rsp_packets.h"

namespace rsp {
/// Empty message can be a reply to 'm' command, or
/// can indicate that command is not supported.
/// Example: ""
class EmptyPacket : public Packet {
 public:
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual Packet* Clone() const { return new EmptyPacket; }
  virtual bool FromBlob(const std::string& type, debug::Blob* message) {
    return message->size() == 0;
  }
  virtual void ToBlob(debug::Blob* message) const { message->Clear(); }
};

/// Stop reply packet.
/// Examples:
/// "S05" - the program received signal number 5
/// "T05" - the same
/// "W66" - program exited with return code 102
/// "X12;process:1234" - program terminated due to signal 18, pid=4660
/// "O" - program is running
class StopReply : public Packet {
 public:
  enum StopReason {SIGNALED, TERMINATED, EXITED, STILL_RUNNING};

  StopReply();
  explicit StopReply(StopReason stop_reason);

  virtual Packet* Clone() const { return new StopReply(stop_reason_); }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  StopReason stop_reason() const { return stop_reason_; }
  int signal_number() const { return signal_number_; }
  int exit_code() const { return exit_code_; }
  uint32_t pid() const { return pid_; }

  void set_stop_reason(StopReason x) { stop_reason_ = x; }
  void set_signal_number(int x) { signal_number_ = x; }
  void set_exit_code(int x) { exit_code_ = x; }
  void set_pid(uint32_t x) { pid_ = x; }

 protected:
  StopReason stop_reason_;
  uint8_t signal_number_;
  uint8_t exit_code_;
  uint32_t pid_;
};

/// Reply to read memory request, and for read registers request.
/// Example: "554889e583ec204c01fc897dec8975e8c745"
class BlobReply : public Packet {
 public:
  virtual Packet* Clone() const { return new BlobReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  const debug::Blob& data() const { return data_; }
  void set_data(const debug::Blob& data) { data_ = data; }
  void set_data(const void* data, size_t size) {
    data_ = debug::Blob(data, size);
  }

 protected:
  debug::Blob data_;
};

/// Error reply.
/// Example: "E02"
class ErrorReply : public Packet {
 public:
  ErrorReply() : error_code_(0) {}
  explicit ErrorReply(int code) : error_code_(code) {}

  virtual Packet* Clone() const { return new ErrorReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message) {
    return PopIntFromFront(message, &error_code_);
  }
  virtual void ToBlob(debug::Blob* message) const {
    Format(message, "E%0.2x", error_code_);
  }
  uint32_t error_code() const { return error_code_; }
  void set_error_code(uint32_t x) { error_code_ = x; }

 protected:
  uint32_t error_code_;
};

/// Success reply.
/// Example: "OK"
class OkReply : public OneWordPacket {
 public:
  OkReply() : OneWordPacket("OK") {}
  virtual Packet* Clone() const { return new OkReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};
}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_COMMON_REPLIES_H_

