// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_RSP_RSP_INFO_PACKETS_H_
#define DEBUGGER_RSP_RSP_INFO_PACKETS_H_
#include <deque>
#include <string>
#include <utility>
#include "debugger/rsp/rsp_packets.h"

namespace rsp {
/// Tell the remote stub about features supported by gdb, and query the stub
/// for features it supports.
/// Example: "qSupported:xmlRegisters=i386;qRelocInsn+"
class QuerySupportedCommand : public Packet {
 public:
  QuerySupportedCommand() {}

  virtual Packet* Clone() const { return new QuerySupportedCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  /// Adds a feature to the list of features.
  /// @param[in] name name of the feature
  /// @param[in] value value of the feature
  void AddFeature(const std::string& name, const std::string& value);

  /// Serialize features into |message|.
  /// @param[out] message destination for the features
  void SaveFeaturesToBlob(debug::Blob* message) const;

  /// @return number of features
  size_t GetFeaturesNum() const;

  /// @return feature name at |pos|.
  /// @param[in] pos feature position
  std::string GetFeatureName(size_t pos) const;

  /// @return feature value
  /// @param[in] name name of the requested feature
  std::string GetFeature(const std::string& name) const;

 protected:
	std::deque<std::pair<std::string, std::string> > features_;
};

/// Reply to the |QuerySupportedCommand|.
class QuerySupportedReply : public QuerySupportedCommand {
 public:
  QuerySupportedReply() {}

  virtual Packet* Clone() const { return new QuerySupportedReply; }
  virtual void AcceptVisitor(PacketVisitor* vis)  { vis->Visit(this); }
  virtual void ToBlob(debug::Blob* message) const;
};

/// Request to read 'features; file.
/// Example: "qXfer:features:read:target.xml:0,7ca"
class QXferFeaturesReadCommand : public Packet {
 public:
  static const char* kPrefix;

  QXferFeaturesReadCommand();
  virtual Packet* Clone() const { return new QXferFeaturesReadCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  std::string file_name() const { return file_name_; }
  uint32_t offset() const { return offset_; }
  uint32_t length() const { return length_;}
  void set_file_name(std::string name) { file_name_ = name; }
  void set_offset(uint32_t x) { offset_ = x; }
  void set_length(uint32_t x) { length_ = x; }

 protected:
  std::string file_name_;
  uint32_t offset_;
  uint32_t length_;
};

/// Reply to QXferFeaturesReadCommand
/// Example: "l<target><architecture>i386:x86-64</architecture></target>"
class QXferReply : public Packet {
 public:
  QXferReply() : eom_(true) {}
  virtual Packet* Clone() const { return new QXferReply; }
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

/// Get section offsets that the target used when relocating the image.
/// Example: qOffsets -> Text=c00000000;Data=c00000000
class GetOffsetsCommand : public OneWordPacket {
 public:
  GetOffsetsCommand() : OneWordPacket("qOffsets") {}
  virtual Packet* Clone() const { return new GetOffsetsCommand; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
};

/// Reply to GetOffsetsCommand.
/// Example: Text=c00000000;Data=c00000000
class GetOffsetsReply : public Packet {
 public:
  GetOffsetsReply() : text_offset_(0), data_offset_(0) {}
  virtual Packet* Clone() const { return new GetOffsetsReply; }
  virtual void AcceptVisitor(PacketVisitor* vis) { vis->Visit(this); }
  virtual bool FromBlob(const std::string& type, debug::Blob* message);
  virtual void ToBlob(debug::Blob* message) const;

  uint64_t text_offset() const { return text_offset_; }
  uint64_t data_offset() const { return data_offset_; }

  void set_text_offset(uint64_t offs) { text_offset_ = offs; }
  void set_data_offset(uint64_t offs) { data_offset_ = offs; }

 private:
  uint64_t text_offset_;
  uint64_t data_offset_;
};

}  // namespace rsp

#endif  // DEBUGGER_RSP_RSP_INFO_PACKETS_H_

