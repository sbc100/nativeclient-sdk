// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef NACL_SDK_BUILD_TOOLS_DEBUG_SERVER_COMMON_DEBUG_BLOB_H_
#define NACL_SDK_BUILD_TOOLS_DEBUG_SERVER_COMMON_DEBUG_BLOB_H_

#include <deque>
#include <string>

typedef unsigned char byte;

// I'm not sure where to get these definitions,
// so I defined it here - for now.
typedef unsigned long long u_int32_t;
typedef unsigned long long u_int64_t;

/// Class for working with raw binary data.
/// Blob == Binary Large Object, an acronim from database world. 
/// Used mostly in Remote Serial Protocol (RSP) code.
///
/// It's a wrapper of std::deque<unsigned char>.
/// Another way to look at it as a string that can have zero 
/// elements. Note that RSP supports binary packets.
/// Why not use std::string instead?
/// I pop characters from the front sometimes, std::deque is more efficient
/// that std::vector (that's how std::string is implemented).
namespace debug {
class Blob {
 public:
  Blob();
  Blob(const Blob& other);
  Blob(const void* buff, size_t buff_sz);
  Blob(const char* buff);
  Blob(const std::string& str);
  virtual ~Blob();
  Blob& operator = (const Blob& other);
  bool operator == (const Blob& other) const;
  
  size_t size() const { return value_.size(); }
  byte operator[] (size_t position) const;
  byte GetAt(size_t position) const;
  byte Front() const;  // Returns first byte.
  byte Back() const;  // Returns last byte.
  byte PopFront();  // Delete first byte.
  byte PopBack();  // Delete last byte.
  void PushFront(byte c);  // Insert byte at beginning.
  void PushBack(byte c);  // Add byte at the end.
  void Append(const Blob& other);
  void Clear();
  void Format(const char* fmt, ...);

  /// example: {0x43, 0x34} -> "C4"
  std::string ToString() const;

  /// example: {0x43, 0x34} -> "4334"
  std::string ToHexString(bool remove_leading_zeroes=true) const;
  bool LoadFromHexString(const std::string& hex_str);
  bool LoadFromHexString(const Blob& hex_str);
  void Reverse();
  bool Compare(const Blob& blob, size_t to_length = -1) const;
  bool HasPrefix(const std::string& prefix) const;
  Blob PopBlobFromFrontUnilChars(const char* chars);

  void PopMatchingCharsFromFront(const char* chars);
  unsigned int PopInt8FromFront();
  u_int32_t PopInt32FromFront();
  u_int64_t PopInt64FromFront();
  void PopSpacesFromBothEnds();

  unsigned int ToInt() const;
  void Split(const char* delimiters, std::deque<Blob>* tokens) const;

  static bool HexCharToInt(byte c, unsigned int* result);
  static char GetHexDigit(unsigned int value, int digit_position);

 protected:
  std::deque<byte> value_;
};

class BlobUniTest {
 public:
  BlobUniTest();
  int Run(std::string* error);  // returns 0 if success, error code if failed.
   
};

}  // namespace debug
#endif  // NACL_SDK_BUILD_TOOLS_DEBUG_SERVER_COMMON_DEBUG_BLOB_H_
