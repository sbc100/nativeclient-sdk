// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_THREADS_PACKETS_H_
#define DEBUGGER_RSP_RSP_THREADS_PACKETS_H_
#include <deque>
#include <string>
#include "debugger/rsp/rsp_packets.h"
namespace rsp {
/// Set thread for subsequent operations
/// Example:
/// "Hg-1" - all threads
/// "Hc1234" - select thread with tid=4660
class SetCurrentThreadCommand : public Packet {
 public:
  enum Subtype {FOR_READ, FOR_CONTINUE};

  SetCurrentThreadCommand() : subtype_(FOR_CONTINUE) {}
  explicit SetCurrentThreadCommand(Subtype subtype) : subtype_(subtype) {}
  virtual Packet* Clone() const {
    return new SetCurrentThreadCommand(subtype_);
  }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  Subtype subtype() const { return subtype_; }
  void set_subtype(Subtype s) { subtype_ = s; }
  uint32_t thread_id() const { return thread_id_; }
  void set_thread_id(uint32_t id) { thread_id_ = id; }

 protected:
  Subtype subtype_;
  uint32_t thread_id_;
};

/// Get current thread command.
/// Example: "qC"
class GetCurrentThreadCommand : public OneWordPacket {
 public:
  GetCurrentThreadCommand() : OneWordPacket("qC") {}
  virtual Packet* Clone() const { return new GetCurrentThreadCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// Reply to get current thread command.
/// Example: "QC1234" - current thread tid is 4660
class GetCurrentThreadReply : public WordWithIntPacket {
 public:
  GetCurrentThreadReply() : WordWithIntPacket("QC") {}
  virtual Packet* Clone() const { return new GetCurrentThreadReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// Find out if the thread is alive
/// Example: "T1234"
class IsThreadAliveCommand : public WordWithIntPacket {
 public:
  IsThreadAliveCommand() : WordWithIntPacket("T") {}
  virtual Packet* Clone() const { return new IsThreadAliveCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// Request to get list of threads
/// Example: "qsThreadInfo"
class GetThreadInfoCommand : public Packet {
 public:
  GetThreadInfoCommand() : get_more_(false) {}
  virtual Packet* Clone() const { return new GetThreadInfoCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  bool get_more() const { return get_more_; }
  void set_get_more(bool more) { get_more_ = more; }

 private:
  bool get_more_;
};

/// Reply to GetThreadInfoCommand
/// Example: "l1234,a34"
class GetThreadInfoReply : public Packet {
 public:
  GetThreadInfoReply() : eom_(true) {}
  virtual Packet* Clone() const { return new GetThreadInfoReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  bool eom() const { return eom_; }
  const std::deque<int>& threads_ids() const { return threads_ids_; }

  void set_eom(bool eom) { eom_ = eom; }
  void set_threads_ids(const std::deque<int>& threads_ids) {
      threads_ids_ = threads_ids; }

 private:
  bool eom_;
  std::deque<int> threads_ids_;
};
}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_THREADS_PACKETS_H_

