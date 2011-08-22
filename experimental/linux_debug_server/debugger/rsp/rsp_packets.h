// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_PACKETS_H_
#define DEBUGGER_RSP_RSP_PACKETS_H_

#include <deque>
#include <map>
#include <string>
#include <vector>
#include "debugger/base/debug_blob.h"
#include "debugger/rsp/rsp_blob_utils.h"

// GDB RSP (remote serial protocol) description:
// http://sources.redhat.com/gdb/current/onlinedocs/gdb.html#Remote-Protocol

namespace rsp {
static const size_t kMaxRspPacketSize = 0x7cf;

class PacketVisitor;
class EmptyPacket;
class QuerySupportedCommand;
class QuerySupportedReply;
class GetStopReasonCommand;
class StopReply;
class ReadMemoryCommand;
class WriteMemoryCommand;
class BlobReply;
class ReadRegistersCommand;
class WriteRegistersCommand;
class ErrorReply;
class OkReply;
class SetCurrentThreadCommand;
class GetCurrentThreadCommand;
class GetCurrentThreadReply;
class ContinueCommand;
class StepCommand;
class IsThreadAliveCommand;
class QXferFeaturesReadCommand;
class QXferReply;
class GetOffsetsCommand;
class GetOffsetsReply;
class GetThreadInfoCommand;
class GetThreadInfoReply;

/// Root class for all RSP packets.
class Packet {
 public:
  Packet() {}
  virtual ~Packet() {}

  /// Creates a new object on the heap.
  /// Used by |factory_| in the |CreateFromBlob| method.
  virtual Packet* Clone() const = 0;

  /// Create a RSP representation for the packet.
  /// @param[out] message destination for the serialized packet
  virtual void ToBlob(debug::Blob* message) const = 0;

  /// Fills content of the packet from |message|.
  /// @param[in] type type of the message, for example "G" for write general
  /// registers packet
  /// @param[in] message serial RSP representation
  /// Call to this method can modify |message|.
  virtual bool FromBlob(const std::string& type,
                        debug::Blob* message) = 0;

  /// This method calls back the appropriate vis->Visit() method.
  virtual void AcceptVisitor(PacketVisitor* vis) {}

  /// Creates an appropriate object from serial RSP representation.
  /// @param[in] message serial RSP representation
  /// @param[in] type_hint hint to help parser
  ///
  /// Note: this method modifies |message|.
  /// Note: returned obj shall be deleted by caller.
  ///
  /// Why |type_hint|? Because some RSP packets don't carry type field.
  /// Examples:
  /// 1. replies to 'm' (read memory) command:
  /// H>[mc000202e0,12]
  /// T>[554889e583ec204c01fc897dec8975e8c745]
  /// => caller should supply "blob$Reply" as a |type_hint|
  ///
  /// 2. replies to 'g' (read registers) command:
  /// H>[g]
  /// T>[00000000000000000000000000000000d85123...
  /// => caller should supply "blob$Reply" as a |type_hint|
  ///
  /// 3. replies to 'qSupported' command:
  /// H>[qSupported:xmlRegisters=i386;qRelocInsn+]
  /// T>[PacketSize=7cf;qXfer:libraries:read+;qXfer:features:read+]
  /// => caller should supply "qSupported$Reply" as a |type_hint|
  ///
  /// 4. replies to 'qXfer' command:
  /// => caller should supply "qXfer$Reply" as a |type_hint|
  static Packet* CreateFromBlob(debug::Blob* message,
                                const char* type_hint);
  /// Creates packet factory.
  static void InitPacketFactory();

  /// Removes packet factory.
  /// Shall be called when |CreateFromBlob| is not needed anymore,
  /// otherwise memory will be leaked.
  static void FreePacketFactory();
};

// Visitor pattern:
// http://en.wikipedia.org/wiki/Visitor_pattern
class PacketVisitor {
 public:
  virtual ~PacketVisitor() {}

  virtual void Visit(EmptyPacket* packet) {}
  virtual void Visit(QuerySupportedCommand* packet) {}
  virtual void Visit(QuerySupportedReply* packet) {}
  virtual void Visit(GetStopReasonCommand* packet) {}
  virtual void Visit(StopReply* packet) {}
  virtual void Visit(ReadMemoryCommand* packet) {}
  virtual void Visit(WriteMemoryCommand* packet) {}
  virtual void Visit(BlobReply* packet) {}
  virtual void Visit(ReadRegistersCommand* packet) {}
  virtual void Visit(WriteRegistersCommand* packet) {}
  virtual void Visit(ErrorReply* packet) {}
  virtual void Visit(OkReply* packet) {}
  virtual void Visit(SetCurrentThreadCommand* packet) {}
  virtual void Visit(GetCurrentThreadCommand* packet) {}
  virtual void Visit(GetCurrentThreadReply* packet) {}
  virtual void Visit(ContinueCommand* packet) {}
  virtual void Visit(StepCommand* packet) {}
  virtual void Visit(IsThreadAliveCommand* packet) {}
  virtual void Visit(QXferFeaturesReadCommand* packet) {}
  virtual void Visit(QXferReply* packet) {}
  virtual void Visit(GetThreadInfoCommand* packet) {}
  virtual void Visit(GetThreadInfoReply* packet) {}
  virtual void Visit(GetOffsetsCommand* packet) {}
  virtual void Visit(GetOffsetsReply* packet) {}
};

/// This class is used in |packet_cast| casting template.
class TypingPacketVisitor : public PacketVisitor {
 public:
  TypingPacketVisitor() : type_(0) {}

  virtual void Visit(EmptyPacket* packet) { type_ = 1;}
  virtual void Visit(QuerySupportedCommand* packet) { type_ = 2;}
  virtual void Visit(QuerySupportedReply* packet) { type_ = 3;}
  virtual void Visit(GetStopReasonCommand* packet) { type_ = 4;}
  virtual void Visit(StopReply* packet) { type_ = 5;}
  virtual void Visit(ReadMemoryCommand* packet) { type_ = 6;}
  virtual void Visit(WriteMemoryCommand* packet) { type_ = 7;}
  virtual void Visit(BlobReply* packet) { type_ = 8;}
  virtual void Visit(ReadRegistersCommand* packet) { type_ = 9;}
  virtual void Visit(WriteRegistersCommand* packet) { type_ = 10;}
  virtual void Visit(ErrorReply* packet) { type_ = 11;}
  virtual void Visit(OkReply* packet) { type_ = 12;}
  virtual void Visit(SetCurrentThreadCommand* packet) { type_ = 13;}
  virtual void Visit(GetCurrentThreadCommand* packet) { type_ = 14;}
  virtual void Visit(GetCurrentThreadReply* packet) { type_ = 15;}
  virtual void Visit(ContinueCommand* packet) { type_ = 16;}
  virtual void Visit(StepCommand* packet) { type_ = 17;}
  virtual void Visit(IsThreadAliveCommand* packet) { type_ = 18;}
  virtual void Visit(QXferFeaturesReadCommand* packet) { type_ = 19;}
  virtual void Visit(QXferReply* packet) { type_ = 20;}
  virtual void Visit(GetThreadInfoCommand* packet) { type_ = 21;}
  virtual void Visit(GetThreadInfoReply* packet) { type_ = 22;}
  virtual void Visit(GetOffsetsCommand* packet) { type_ = 23;}
  virtual void Visit(GetOffsetsReply* packet) { type_ = 24;}

  int type_;
};

template <class T>
T* packet_cast(Packet* obj) {
  if (NULL == obj)
    return NULL;

  TypingPacketVisitor vis;
  obj->AcceptVisitor(&vis);
  int obj_type = vis.type_;

  T tmp;
  tmp.AcceptVisitor(&vis);
  if (vis.type_ == obj_type)
    return reinterpret_cast<T*>(obj);
  return NULL;
}

/// Abstract base class for packets types: 'g', '?', 'qC', 's' etc.
class OneWordPacket : public Packet {
 public:
  explicit OneWordPacket(const std::string& word) : word_(word) {}
  virtual bool FromBlob(const std::string& type, debug::Blob* message) {
    return message->size() == 0;
  }
  virtual void ToBlob(debug::Blob* message) const {
    message->FromString(word_);
  }
 protected:
  std::string word_;
};

/// Abstract base class for these packets classes:
/// GetCurrentThreadReply
/// IsThreadAliveCommand
class WordWithIntPacket : public Packet {
 public:
  explicit WordWithIntPacket(const std::string& word) : word_(word) {}

  virtual bool FromBlob(const std::string& type, debug::Blob* message) {
    return PopIntFromFront(message, &value_);
  }
  virtual void ToBlob(debug::Blob* message) const {
    Format(message, "%s%x", word_.c_str(), value_);
  }
  uint32_t value() const { return value_; }
  void set_value(uint32_t x) { value_ = x; }

 protected:
  std::string word_;
  uint32_t value_;
};
}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_PACKETS_H_

