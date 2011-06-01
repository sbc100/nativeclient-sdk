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

  /// Copy constructor.
  /// @param other object to be copied
  Blob(const Blob& other);

  /// Initializes object with data from |buff|.
  /// @param[in] buff pointer to the data to be copied into Blob.
  /// @param[in] buff_sz number of bytes to copy.
  /// Data in |buff| can be modified right after constructor call.
  Blob(const void* buff, size_t buff_sz);

  /// Initializes object with data from |buff|.
  /// @param[in] buff pointer to zero-terminated data to be copied into Blob.
  /// Data in |buff| can be modified right after constructor call.
  Blob(const char* buff);

  /// Initializes object with data from |buff|.
  /// @param[in] str data to be copied into Blob.
  Blob(const std::string& str);

  virtual ~Blob();
  Blob& operator = (const Blob& other);
  bool operator == (const Blob& other) const;
  
  /// @return number of bytes in the Blob.
  size_t size() const { return value_.size(); }

  /// @return a byte at |position|.
  byte operator[] (size_t position) const;

  /// @return a byte at |position|.
  byte GetAt(size_t position) const;

  /// @return a byte at zero position.
  byte Front() const;

  /// @return last byte.
  byte Back() const;

  /// Removes first byte.
  /// @return removed byte.
  byte PopFront();

  /// Removes last byte.
  /// @return removed byte.
  byte PopBack();

  /// Inserts byte at beginning.
  void PushFront(byte c);

  /// Appends byte.
  void PushBack(byte c);

  /// Appends Blob.
  /// @param[in] other Blob to append.
  void Append(const Blob& other);

  /// Removes all bytes.
  void Clear();

  /// Writes formatted data to Blob.
  /// @param[in] string that contains the text to be written to the Blob.
  void Format(const char* fmt, ...);

  /// @return std::string with elements equal to elements of the Blob.
  /// Note: Blob shall not have zero elements.
  /// example: {0x43, 0x34} -> "C4"
  std::string ToString() const;

  /// @return std::string with hex representation of the Blob.
  /// example: {0x43, 0x3a} -> "433a"
  std::string ToHexString(bool remove_leading_zeroes=true) const;

  void* ToCBuffer() const;

  /// @return number of bytes copied into the |buff|.
  size_t Peek(size_t offset, void* buff, size_t buff_sz) const;

  /// Converts string with hex representation to raw data.
  bool LoadFromHexString(const std::string& hex_str);

  /// Reverses the order of the bytes.
  void Reverse();

  /// @return true if elements are the same, and number of them is the same.
  /// @param blob Blob to compare with.
  /// @param to_length number of bytes to compare. If |to_length| is
  /// equal to -1, all bytes of |blob| are compared.
  bool Compare(const Blob& blob, size_t to_length = -1) const;

  /// @return true, if |prefix| bytes equal bytes in the begining of this Blob.
  bool IsPrefix(const Blob& prefix) const;

  /// Removes bytes from the front of this blob, until first byte of this Blob
  /// is contained in the |chars|.
  /// @return Blob with bytes removed from this Blob.
  Blob PopBlobFromFrontUnilChars(const char* chars);

  /// Removes bytes from the front of this blob, until first byte of this Blob
  /// is not contained in the |chars|.
  /// @return Blob with bytes removed from this Blob.
  void PopMatchingCharsFromFront(const char* chars);

  /// Removes byte from the front of this blob, converts it to integer
  /// assumes hex test representation is in the blob.
  /// example: {0x
  /// @return
  unsigned int PopInt8FromFront();

  /// Removes bytes from the front of this blob, converts it to 32 bit integer
  /// @return
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
