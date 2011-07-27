// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/base/debug_blob.h"
#include <ctype.h>
#include <algorithm>
#include <string>

namespace debug {
Blob::Blob() {
}

Blob::Blob(const Blob& other) {
  value_ = other.value_;
}

Blob::Blob(const void* buff, size_t buff_sz) {
  const uint8_t* char_buff = static_cast<const uint8_t*>(buff);
  for (size_t i = 0; i < buff_sz; i++ )
    PushBack(*char_buff++);
}

Blob& Blob::FromString(const std::string& str) {
  Clear();
  for (size_t i = 0; i < str.size(); i++ )
    PushBack(str[i]);
  return *this;
}

Blob::~Blob() {
}

Blob& Blob::operator = (const Blob& other) {
  value_ = other.value_;
  return *this;
}

bool Blob::operator == (const Blob& other) const {
  return value_ == other.value_;
}

uint8_t Blob::operator[] (size_t position) const {
  return value_[position];
}

uint8_t& Blob::operator[] (size_t position) {
  return value_[position];
}

uint8_t Blob::GetAt(size_t position) const {
  return value_[position];
}

uint8_t Blob::Front() const {
  return value_.front();
}

uint8_t Blob::Back() const {
  return value_.back();
}

uint8_t Blob::PopFront() {
  uint8_t c = value_.front();
  value_.pop_front();
  return c;
}

uint8_t Blob::PopBack() {
  uint8_t c = value_.back();
  value_.pop_back();
  return c;
}

void Blob::PushFront(uint8_t c) {
  value_.push_front(c);
}

void Blob::PushBack(uint8_t c) {
  value_.push_back(c);
}

void Blob::Clear() {
  value_.clear();
}

std::string Blob::ToString() const {
  std::string result;
  for (size_t i = 0; i < size(); i++)
    result.append(1, value_[i]);
  return result;
}

std::string Blob::ToHexString() const {
  std::string result;
  size_t num = size();
  for (size_t i = 0; i < num; i++) {
    char c = GetHexDigit(value_[i], 1);
    result.append(1, c);
    c = GetHexDigit(value_[i], 0);
    result.append(1, c);
  }
  return result;
}

std::string Blob::ToHexStringNoLeadingZeroes() const {
  std::string result = ToHexString();
  while (result[0] == '0')
    result.erase(0, 1);
  return result;
}

size_t Blob::Peek(size_t offset, void* buff, size_t buff_sz) const {
  uint8_t* out = static_cast<uint8_t*>(buff);
  size_t copied_bytes_ts = 0;
  for (size_t i = 0; i < buff_sz; i++) {
    size_t pos = offset + i;
    if (pos >= size())
      break;
    out[i] = GetAt(pos);
    copied_bytes_ts++;
  }
  return copied_bytes_ts;
}

bool Blob::FromHexString(const std::string& hex_str) {
  Clear();
  size_t num = hex_str.size();
  for (size_t i = num; i > 0; i -= 2) {
    if (isspace(hex_str[i - 1])) {
      i++;
    } else if (i >= 2) {
      unsigned int c1 = 0;
      unsigned int c2 = 0;
      if (!HexCharToInt(hex_str[i - 1], &c1)) return false;
      if (!HexCharToInt(hex_str[i - 2], &c2)) return false;
      unsigned int c = c1 + (c2 << 4);
      PushFront(c);
    } else {
      unsigned int c = 0;
      if (!HexCharToInt(hex_str[i - 1], &c)) return false;
      PushFront(c);
      break;
    }
  }
  return true;
}

void Blob::Append(const Blob& blob) {
  for (size_t i = 0; i < blob.size(); i++)
    PushBack(blob[i]);
}

void Blob::Reverse() {
  std::reverse(value_.begin(), value_.end());
}

void Blob::Split(const Blob& delimiters, std::deque<Blob>* tokens) const {
  tokens->clear();
  Blob token;
  std::deque<uint8_t>::const_iterator it = value_.begin();
  while (value_.end() != it) {
    uint8_t c = *it++;
    if (delimiters.HasByte(c)) {
      tokens->push_back(token);
      token.Clear();
    } else {
      token.PushBack(c);
    }
  }
  if (0 != token.size())
    tokens->push_back(token);
}

void Blob::PopMatchingBytesFromFront(const Blob& bytes) {
  while (size()) {
    char c = Front();
    if (!bytes.HasByte(c))
      break;
    else
      PopFront();
  }
}

debug::Blob Blob::PopBlobFromFrontUntilBytes(const Blob& bytes) {
  debug::Blob result;
  while (size()) {
    char c = PopFront();
    if (bytes.HasByte(c))
      break;
    else
      result.PushBack(c);
  }
  return result;
}

bool Blob::HasByte(char c) const {
  for (size_t i = 0; i < size(); i++)
    if (c == value_[i])
      return true;
  return false;
}

bool Blob::HexCharToInt(unsigned char c, unsigned int* result) {
  if (('0' <= c) && ('9' >= c)) {
    *result = c - '0';
  } else if (('A' <= c) && ('F' >= c)) {
    *result = c - 'A' + 10;
  } else if (('a' <= c) && ('f' >= c)) {
    *result = c - 'a' + 10;
  } else {
    return false;
  }
  return true;
}

char Blob::GetHexDigit(unsigned int value, int digit_position) {
  const char* kDigitStrings = "0123456789abcdef";
  unsigned int digit = (value >> (4 * digit_position)) & 0xF;
  return kDigitStrings[digit];
}
}  // namespace debug

