// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_BASE_DEBUG_BLOB_H_
#define DEBUGGER_BASE_DEBUG_BLOB_H_

#include <deque>
#include <string>

// I'm not sure from where to get these definitions,
// so I defined them here - for now.
typedef unsigned char uint8_t;
typedef unsigned long long uint32_t;  // NOLINT
typedef unsigned long long uint64_t;  // NOLINT

/// Class for working with raw binary data.
/// Blob == Binary Large Object, an acronym from database world.
///
/// It's a wrapper of std::deque<unsigned char>.
/// Another way to look at it as a string that can have zero elements.
/// Why not use std::string instead?
/// I pop characters from the front sometimes, std::deque is more efficient
/// that std::string (aka std::string).
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

  virtual ~Blob();
  Blob& operator = (const Blob& other);
  bool operator == (const Blob& other) const;

  /// @return number of bytes in the Blob.
  size_t size() const { return value_.size(); }

  /// @param position offset of the byte to be returned
  /// @return a byte at |position|.
  uint8_t operator[] (size_t position) const;

  /// @param position offset of the byte to be returned
  /// @return a byte at |position|.
  uint8_t GetAt(size_t position) const;

  /// @return a byte at zero position.
  uint8_t Front() const;

  /// @return last byte.
  uint8_t Back() const;

  /// Removes first byte.
  /// @return removed byte.
  uint8_t PopFront();

  /// Removes last byte.
  /// @return removed byte.
  uint8_t PopBack();

  /// Inserts byte at beginning.
  void PushFront(uint8_t c);

  /// Appends byte.
  /// @param c byte to append
  void PushBack(uint8_t c);

  /// Appends Blob.
  /// @param[in] other Blob to append.
  void Append(const Blob& other);

  /// Removes all byte. Drops size to zero.
  void Clear();

  /// Copies data from blob to |buff|.
  /// @param[in] offset offset of data in the blob
  /// @param[out] buff buffer where data is to be copied.
  /// @param[in] buff_sz size of |buff|.
  /// @return number of bytes copied into the |buff|.
  size_t Peek(size_t offset, void* buff, size_t buff_sz) const;

  /// @return std::string with elements equal to elements of the Blob.
  /// Note: Blob shall not have zero elements.
  /// example: {0x43, 0x34} -> "C4"
  std::string ToString() const;

  /// @return std::string with hex representation of the Blob.
  /// example: {0x03, 0x3a} -> "033a"
  std::string ToHexString() const;

  /// @return std::string with hex representation of the Blob.
  /// example: {0x03, 0x3a} -> "33a"
  std::string ToHexStringNoLeadingZeroes() const;

  /// Converts string with hex representation to raw data.
  /// example: "c46a" -> {0xc4, 0x6a}
  /// @param hex_str with hex-represented data
  bool FromHexString(const std::string& hex_str);

  /// Initializes object with data from |str|.
  /// @param[in] str data to be copied into Blob.
  Blob& FromString(const std::string& str);

  /// Reverses the order of the byte.
  void Reverse();

  /// Splits blob into tokens.
  /// @param[in] delimiters blob with bytes that act as token separators
  /// @param[out] tokens resulting tokens.
  void Split(const Blob& delimiters, std::deque<Blob>* tokens) const;

  /// Removes bytes from the front of this blob, until first byte of this Blob
  /// is contained in the |bytes|.
  /// @param bytes[in] blob with terminating bytes
  /// @return Blob with bytes removed from this Blob.
  debug::Blob PopBlobFromFrontUntilBytes(const Blob& bytes);

  /// Removes bytes from the front of this blob, until first byte of this Blob
  /// is not contained in the |bytes|.
  /// @param bytes[in] blob with matching bytes
  void PopMatchingBytesFromFront(const Blob& bytes);

  /// Converts hex letter to integer.
  /// Example: '2' -> 2, 'c' -> 12
  /// @param[in] c hex letter.
  /// @param[out] result integer corresponding to |c|
  /// @return false if |c| is not in [0..9a..f]
  static bool HexCharToInt(uint8_t c, unsigned int* result);

  /// Converts part of the integer info hex letter.
  /// Example: GetHexDigit(0xa5, 0) -> '5', GetHexDigit(0xa5, 1) -> 'a'
  /// @param[in] value integer to be converted
  /// @param[in] digit_position position of the hex letter, 0 or 1.
  /// @return hex letter
  static char GetHexDigit(unsigned int value, int digit_position);

 protected:
  bool HasByte(char c) const;

  std::deque<uint8_t> value_;
};
}  // namespace debug
#endif  // DEBUGGER_BASE_DEBUG_BLOB_H_

