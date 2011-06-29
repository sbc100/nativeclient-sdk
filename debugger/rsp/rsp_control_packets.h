// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_CONTROL_PACKETS_H_
#define DEBUGGER_RSP_RSP_CONTROL_PACKETS_H_
#include <string>
#include "debugger/rsp/rsp_packets.h"

namespace rsp {
/// Requests the reason the target halted.
/// Example: "?"
class GetStopReasonCommand : public OneWordPacket {
 public:
  GetStopReasonCommand() : OneWordPacket("?") {}
  virtual Packet* Clone() const { return new GetStopReasonCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// Read target memory request.
/// Example: "mc000202e0,12"
class ReadMemoryCommand : public Packet {
 public:
  ReadMemoryCommand();

  virtual Packet* Clone() const { return new ReadMemoryCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  uint64_t addr() const { return addr_; }
  int num_of_bytes() const { return num_of_bytes_; }

  void set_addr(uint64_t ptr) { addr_ = ptr; }
  void set_num_of_bytes(int x) { num_of_bytes_ = x; }

 protected:
  uint64_t addr_;
  int num_of_bytes_;
};

/// Write target memory request.
/// Example: "Mc00020304,1:8b"
class WriteMemoryCommand : public Packet {
 public:
  WriteMemoryCommand();

  virtual Packet* Clone() const { return new WriteMemoryCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  uint64_t addr() const { return addr_; }
  const debug::Blob& data() const { return data_; }

  void set_addr(uint64_t addr) { addr_ = addr; }
  void set_data(const debug::Blob& data) { data_ = data; }
  void set_data(const void* data, size_t size);

 protected:
  uint64_t addr_;
  debug::Blob data_;
};

/// Read general registers command.
/// Example: "g"
class ReadRegistersCommand : public OneWordPacket {
 public:
  ReadRegistersCommand() : OneWordPacket("g") {}
  virtual Packet* Clone() const { return new ReadRegistersCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// Write general registers command.
/// Example: "G00000000000000000000000000000000d8513702000000004"
class WriteRegistersCommand : public Packet {
 public:
  virtual Packet* Clone() const { return new WriteRegistersCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  const debug::Blob& data() const { return data_; }
  void set_data(const debug::Blob& data) { data_ = data; }

 protected:
  debug::Blob data_;
};

/// Continue command
/// Example: "c"
class ContinueCommand : public OneWordPacket {
 public:
  ContinueCommand() : OneWordPacket("c") {}
  virtual Packet* Clone() const { return new ContinueCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// Single (instruction) step command
class StepCommand : public OneWordPacket {
 public:
  StepCommand() : OneWordPacket("s") {}
  virtual Packet* Clone() const { return new StepCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};
}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_CONTROL_PACKETS_H_

