/*
 * Copyright 2011 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "debug_conn/debug_packet.h"
#include "debug_conn/debug_stub_vsx.h"
#include "debug_conn/debug_util.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include <sstream>

using namespace nacl_debug_conn;
using namespace std;

DebugPacket::DebugPacket() : seq(-1) {}

void DebugPacket::Clear() {
  data.str("");
  seq = -1;
}

void DebugPacket::Rewind() {
  data.seekg(0, ios::beg);
  data.seekp(0, ios::beg);
}

void DebugPacket::AddRawChar(char ch) {
  data << ch;
}

void DebugPacket::AddByte(uint8_t ch) {
  char seq1 = debug_int_to_nibble(ch >> 4);
  char seq2 = debug_int_to_nibble(ch & 0xF);

  AddRawChar(seq1);
  AddRawChar(seq2);
}

void DebugPacket::AddBlock(void *ptr, int len) {
  const char *p = (const char *) ptr;

  for (int offs = 0; offs < len; offs++)
    AddByte(p[offs]);
}

void DebugPacket::AddWord16(uint16_t val) {
  AddBlock(&val, sizeof(val));
}

void DebugPacket::AddWord32(uint32_t val) {
  AddBlock(&val, sizeof(val));
}

void DebugPacket::AddWord64(uint64_t val) {
  AddBlock(&val, sizeof(val));
}

void DebugPacket::AddPointer(void *ptr) {
  return AddBlock(&ptr, sizeof(ptr));
}

void DebugPacket::AddString(const char *str) {
  data << str;
}

void DebugPacket::AddHexString(const char *str) {
  while (*str) {
    AddByte(*str);
    str++;
  }
}

void DebugPacket::AddNumberSep(uint64_t val, char sep) {
  char out[sizeof(val) * 2];
  int nibbles = 0;
  int a;

  // Check for -1 optimization
  if (val == -1) {
    AddRawChar('-');
    AddRawChar('1');
  }
  else {
    // Assume we have the valuse 0x00001234
    for (a=0; a < sizeof(val); a++) {
      uint8_t byte = val & 0xFF;
      // Stream in with bytes reverse, starting at least significant
      // So we store 4, then 3, 2, 1
      out[nibbles++] = debug_int_to_nibble(byte & 0xF);
      out[nibbles++] = debug_int_to_nibble(byte >> 4);

      // Get the next 8 bits;
      val >>= (intptr_t) 8;

      // Supress leading zeros, so we are done when val hits zero
      if (val == 0)
        break;
    }

    // Strip the high zero for this byte if needed
    if ((nibbles > 2) && (out[nibbles-1] == '0'))
      nibbles--;

    // Now write it out reverse to correct the order
    while (nibbles) {
      nibbles--;
      AddRawChar(out[nibbles]);
    }
  }

  // If we asked for a sperator, insert it
  if (sep)
    AddRawChar(sep);
}

bool DebugPacket::GetNumberSep(uint64_t *val, char *sep) {
  uint64_t out = 0;
  char ch;
  if (!GetRawChar(&ch))
    return false;

  // Check for -1
  if (ch == '-') {
    if (!GetRawChar(&ch))
      return false;
    if (ch == '1') {
      *val = -1;
      ch = 0;
      GetRawChar(&ch);
       if (sep)
          *sep = ch;
       return true;
    }
    return false;
  }

  do {
    uint64_t nib = debug_nibble_to_int(ch);

    // If we can't translate, this must be the separator
    if (nib == (uint64_t) -1)
      break;

    out = (out << (uint64_t) 4) + nib;

    // Get the next character (if availible)
    ch = 0;
    if (!GetRawChar(&ch))
      break;

  } while (1);

  // Set the value;
  *val = out;

  // If the user want it...
  if (sep)
    *sep = ch;

  return true;
}

bool DebugPacket::GetHexString(const char **sep) {
  string out;
  char ch;

  while (GetRawChar(&ch))
    out += ch;

  *sep = strdup(out.data());
  return true;
}

bool DebugPacket::GetRawChar(char *ch) {
  if (data.eof())
    return DS_NONE;

  data >> *ch;
  return DS_OK;
}

bool DebugPacket::GetByte(uint8_t *ch) {
  char seq1, seq2;
  bool res;

  res = GetRawChar(&seq1);
  res = GetRawChar(&seq2);
  *ch  = debug_nibble_to_int(seq1) << 4;
  *ch += debug_nibble_to_int(seq2);

  return res;
}

bool DebugPacket::GetBlock(void *ptr, int len) {
  uint8_t *p = (uint8_t *) ptr;
  bool res;

  for (int offs = 0; offs < len; offs++) {
    res = GetByte(&p[offs]);
    if (false == res)
      break;
  }

  return res;
}

bool DebugPacket::GetWord16(uint16_t *ptr) {
  return GetBlock(ptr, sizeof(*ptr));
}

bool DebugPacket::GetWord32(uint32_t *ptr) {
  return GetBlock(ptr, sizeof(*ptr));
}

bool DebugPacket::GetWord64(uint64_t *ptr) {
  return GetBlock(ptr, sizeof(*ptr));
}

bool DebugPacket::GetPointer(void **ptr) {
  return GetBlock(ptr, sizeof(*ptr));
}

bool DebugPacket::PeekString(const char **ppstr) {
  string str = "";
  if (data.eof())
    return DS_NONE;
  str = data.str();
  *ppstr = strdup(str.data());
  return true;
}

bool DebugPacket::PeekChar(char *ch) {
  string str = "";
  if (data.eof())
    return false;
  str = data.str();
  *ch = str[0];
  return true;
}

bool DebugPacket::GetString(const char **ppstr) {
  string str = "";
  if (data.eof())
    return DS_NONE;

  const int bufsz = 127;
  char buf[bufsz + 1];
  while (!data.eof()) {
    data.getline(buf, bufsz);
    str += buf;
  }
  *ppstr = strdup(str.data());
  return true;
}

const char *DebugPacket::GetPayload() const {
  string str = data.str();
  const char *out = str.data();
  return strdup(out);
}

bool DebugPacket::GetSequence(int32_t *ch) const {
  // If we don't have one, try and read it from the stream
  if (seq == -1) {

  }

  if (seq != -1) {
    *ch = seq;
    return true;
  }

  return false;
}

void DebugPacket::SetSequence(int32_t val) {
  seq = val;
}


int nacl_debug_conn::DebugPacket::Read( void *ptr, int len ) {
  uint8_t *p = (uint8_t *) ptr;
  int offs = 0;

  for (offs = 0; offs < len; offs++) {
    bool res = GetByte(&p[offs]);
    if (false == res)
      break;
  }

  return offs;
}