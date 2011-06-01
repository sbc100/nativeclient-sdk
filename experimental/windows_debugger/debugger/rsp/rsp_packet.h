// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEBUGGER_RSP_RSP_PACKET_H_
#define DEBUGGER_RSP_RSP_PACKET_H_

#include <string>
#include <vector>
#include <deque>
#include <map>
#include "debugger/base/debug_blob.h"

namespace rsp {
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
class GetThreadInfoCommand;
class GetThreadInfoReply;

class Packet {
public:
  Packet() {}
  virtual ~Packet() {}

  /// Creates a new object on the heap.
  /// Shall be overwritten by descendants.
  /// Used by |factory_| in the |CreateFromBlob| method.
  virtual Packet* Create() const {return new Packet;}

  /// Shall be overwritten in descendants.
  virtual void ToBlob(debug::Blob* message) const {}

  /// Shall be overwritten in descendants.
  virtual bool FromBlob(const std::string& type,
                        debug::Blob* message) {return true;}

  /// Shall be overwritten in descendants.
  virtual void AcceptVisitor(PacketVisitor* vis) {}

  static Packet* CreateFromBlob(debug::Blob* message,
                                const char* type_hint = NULL);

protected:
};

class PacketVisitor {
 public:
  ~PacketVisitor() {}

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
};

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
  int t_type = vis.type_;

  if (t_type == obj_type)
    return reinterpret_cast<T*>(obj);
  return NULL;
}

class EmptyPacket : public Packet {
 public:
  EmptyPacket() {}

  virtual void AcceptVisitor(PacketVisitor* vis);
  virtual Packet* Create() const;
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;
};

class QuerySupportedCommand : public Packet {
 public:
  QuerySupportedCommand() {}

  virtual Packet* Create() const;
  virtual void AcceptVisitor(PacketVisitor* vis);
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  void AddFeature(const std::string& name, const std::string& value);
  void SaveFeaturesToBlob(debug::Blob* message) const;

  size_t GetFeaturesNum() const;
  std::string GetFeatureName(size_t pos) const;
  std::string GetFeature(const std::string& name) const;

 protected:
  std::deque<std::pair<std::string, std::string>> features_;
};

class QuerySupportedReply : public QuerySupportedCommand {
 public:
  QuerySupportedReply() {}

  virtual Packet* Create() const;
  virtual void AcceptVisitor(PacketVisitor* vis);
  virtual void ToBlob(debug::Blob* message) const;
};

// Abstract class - base for packets types: 'g', '?', 'qC', 's' etc.
class OneWordPacket : public Packet {
 public:
  OneWordPacket(const std::string& word) : word_(word) {}

  virtual Packet* Create() const = 0;
  virtual bool FromBlob(const std::string& type, debug::Blob* message) {
    return true;
  }
  virtual void ToBlob(debug::Blob* message) const {
    *message = word_;
  }

 protected:
  std::string word_;
};

// Abstract class - base for packets types: 
class WordWithIntPacket : public Packet {
 public:
  WordWithIntPacket(const std::string& word) : word_(word) {}

  virtual Packet* Create() const = 0;
  virtual bool FromBlob(const std::string& type, debug::Blob* message) {
    value_ = message->PopInt32FromFront();
    return true;
  }
  virtual void ToBlob(debug::Blob* message) const {
    message->Format("%s%x", word_.c_str(), value_);
  }
  u_int32_t value() const { return value_; }
  void set_value(u_int32_t x) { value_ = x; }

 protected:
  std::string word_;
  u_int32_t value_;
};

class GetStopReasonCommand : public OneWordPacket {
 public:
  GetStopReasonCommand() : OneWordPacket("?") {}
  virtual Packet* Create() const { return new GetStopReasonCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

class StopReply : public Packet {
 public:
  enum StopReason {SIGNALED, TERMINATED, EXITED, STILL_RUNNING};

  StopReply();
  StopReply(StopReason stop_reason);

  virtual Packet* Create() const;
  virtual void AcceptVisitor(PacketVisitor* vis);
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  StopReason stop_reason() const { return stop_reason_; }
  int signal_number() const { return signal_number_; }
  int exit_code() const { return exit_code_; }
  u_int32_t pid() const { return pid_; }

  void set_stop_reason(StopReason x) { stop_reason_ = x; }
  void set_signal_number(int x) { signal_number_ = x; }
  void set_exit_code(int x) { exit_code_ = x; }
  void set_pid(u_int32_t x) { pid_ = x; }

public:
  StopReason stop_reason_;
  int signal_number_;
  int exit_code_;
  u_int32_t pid_;
};

class ReadMemoryCommand : public Packet {
 public:
  ReadMemoryCommand();

  virtual Packet* Create() const;
  virtual void AcceptVisitor(PacketVisitor* vis);
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  u_int64_t addr() const { return addr_; }
  int num_of_bytes() const { return num_of_bytes_; }

  void set_addr(u_int64_t ptr) { addr_ = ptr; }
  void set_num_of_bytes(int x) { num_of_bytes_ = x; }

 protected:
  u_int64_t addr_;
  int num_of_bytes_;
};

class WriteMemoryCommand : public Packet {
 public:
  WriteMemoryCommand();

  virtual Packet* Create() const;
  virtual void AcceptVisitor(PacketVisitor* vis);
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

 u_int64_t addr() const { return addr_; }
 const debug::Blob& data() const { return data_; }

 void set_addr(u_int64_t addr) { addr_ = addr; }
 void set_data(const debug::Blob& data) { data_ = data; }
 void set_data(const void* data, size_t size);

 protected:
  u_int64_t addr_;
  debug::Blob data_;
};

class BlobReply : public Packet {
 public:
  virtual Packet* Create() const;
  virtual void AcceptVisitor(PacketVisitor* vis);
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

 const debug::Blob& data() const { return data_; }
 void set_data(const debug::Blob& data) { data_ = data; }
 void set_data(const void* data, size_t size) { data_ = debug::Blob(data, size); }

 protected:
  debug::Blob data_;
};

class ReadRegistersCommand : public OneWordPacket {
 public:
  ReadRegistersCommand() : OneWordPacket("g") {}
  virtual Packet* Create() const { return new ReadRegistersCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

class WriteRegistersCommand : public Packet {
 public:
  virtual Packet* Create() const { return new WriteRegistersCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

 const debug::Blob& data() const { return data_; }
 void set_data(const debug::Blob& data) { data_ = data; }

 protected:
  debug::Blob data_;
};

/// "E02"
class ErrorReply : public Packet {
 public:
  ErrorReply() : error_code_(0) {}
  explicit ErrorReply(int code) : error_code_(code) {}

  virtual Packet* Create() const { return new ErrorReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message) {
    error_code_ = message->PopInt32FromFront();
    return true;
  }
  virtual void ToBlob(debug::Blob* message) const {
    message->Format("E%0.2x", error_code_);
  }
  u_int32_t error_code() const { return error_code_; }
  void set_error_code(u_int32_t x) { error_code_ = x; }

 protected:
  u_int32_t error_code_;
};

/// "OK"
class OkReply : public OneWordPacket {
 public:
  OkReply() : OneWordPacket("OK") {}
  virtual Packet* Create() const { return new OkReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// "Hg" or "Hc"
class SetCurrentThreadCommand : public Packet {
 public:
  enum Subtype {FOR_READ, FOR_CONTINUE};

  SetCurrentThreadCommand() : subtype_(FOR_CONTINUE) {}
  explicit SetCurrentThreadCommand(Subtype subtype) : subtype_(subtype) {}
  virtual Packet* Create() const { return new SetCurrentThreadCommand(subtype_); }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  Subtype subtype() const { return subtype_; }
  void set_subtype(Subtype s) { subtype_ = s; }
  u_int32_t thread_id() const { return thread_id_; }
  void set_thread_id(u_int32_t id) { thread_id_ = id; }

 protected:
  Subtype subtype_; 
  u_int32_t thread_id_;
};

class GetCurrentThreadCommand : public OneWordPacket {
 public:
  GetCurrentThreadCommand() : OneWordPacket("qC") {}
  virtual Packet* Create() const { return new GetCurrentThreadCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

class GetCurrentThreadReply : public WordWithIntPacket {
 public:
  GetCurrentThreadReply() : WordWithIntPacket("QC") {}
  virtual Packet* Create() const { return new GetCurrentThreadReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

class ContinueCommand : public OneWordPacket {
 public:
  ContinueCommand() : OneWordPacket("c") {}
  virtual Packet* Create() const { return new ContinueCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

class StepCommand : public OneWordPacket {
 public:
  StepCommand() : OneWordPacket("s") {}
  virtual Packet* Create() const { return new StepCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

class IsThreadAliveCommand : public WordWithIntPacket {
 public:
  IsThreadAliveCommand() : WordWithIntPacket("T") {}
  virtual Packet* Create() const { return new IsThreadAliveCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// qXfer:features:read
/// TODO: Change to 'ReadObjectCommand' + 
/// 'object'
/// 'annex'
class QXferFeaturesReadCommand : public Packet {
 public:
  static const char* kPrefix;

  QXferFeaturesReadCommand();
  virtual Packet* Create() const { return new QXferFeaturesReadCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  std::string file_name() const { return file_name_; }
  u_int32_t offset() const { return offset_; }
  u_int32_t length() const { return length_;}
  void set_file_name(std::string name) { file_name_ = name; }
  void set_offset(u_int32_t x) { offset_ = x; }
  void set_length(u_int32_t x) { length_ = x; }

 protected:
  std::string file_name_;
  u_int32_t offset_;
  u_int32_t length_;
};

class QXferReply : public Packet {
 public:
  QXferReply() : eom_(true) {}
  virtual Packet* Create() const { return new QXferReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  bool eom() const { return eom_; }
  std::string body() const { return body_; }

  void set_eom(bool eom) { eom_ = eom; }
  void set_body(const std::string& body) { body_ = body; }

 protected:
  bool eom_;
  std::string body_;
};

class GetThreadInfoCommand : public Packet {
 public:
  GetThreadInfoCommand() : get_more_(false) {}
  virtual Packet* Create() const { return new GetThreadInfoCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  bool get_more() const { return get_more_; }
  void set_get_more(bool more) { get_more_ = more; }

 private:
  bool get_more_;
};

class GetThreadInfoReply : public Packet {
 public:
  GetThreadInfoReply() : eom_(true) {}
  virtual Packet* Create() const { return new GetThreadInfoReply; }
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
}  //namespace rsp

#endif  // DEBUGGER_RSP_RSP_PACKET_H_

