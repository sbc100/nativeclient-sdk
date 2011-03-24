/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_PACKET_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_PACKET_H_ 1

#include <sstream>

/*
 * This module provides interfaces for creating and viewing debug
 * packets that conform to the GDB serial line debug protocol. The packet
 * must not contain the special characters '$' or '#' which are used by
 * the transport/framing layer to denote start and end of packets.
 *
 * All binary is expected to be cooked and convered into a pair of hex
 * nibbles per byte.  Data is stored as a stream, where the first char
 * is expected to be and uncooked command ID, followed by optional
 * arguments which may be raw or cooked.
 *
 * In addition, packets may be sequenced by setting an 8 bit sequence number,
 * which helps both sides detect when packets have been lost.  By default the
 * sequence number is not set.
 */

#include "native_client/src/include/nacl_base.h"
#include "native_client/src/include/portability.h"

namespace nacl_debug_conn {

class DebugPacket {
public:
  DebugPacket();

public:
  void Clear();
  void Rewind();

  void AddRawChar(char ch);
  void AddByte(uint8_t ch);
  void AddBlock(void *ptr, int len);
  void AddString(const char *str);
  void AddHexString(const char *str);
  void AddPointer(void *ptr);
  void AddWord16(uint16_t val);
  void AddWord32(uint32_t val);
  void AddWord64(uint64_t val);
  void AddNumberSep(uint64_t val, char sep);

public:
  bool GetRawChar(char *ch);
  bool GetByte(uint8_t *ch);
  bool GetBlock(void *ptr, int len);
  bool GetString(const char **str);
  bool PeekString(const char **ppstr);
  bool PeekChar(char *ch);
  bool GetHexString(const char **str);
  bool GetPointer(void **ptr);
  bool GetWord16(uint16_t *val);
  bool GetWord32(uint32_t *val);
  bool GetWord64(uint64_t *val);
  bool GetNumberSep(uint64_t *val, char *sep);

  int Read(void *ptr, int len);
public:
  // GetSequence and SetSequence are DEPRECATED.  We are not sending sequence
  // numbers or checking them anymore
  bool GetSequence(int32_t *seq) const;
  void SetSequence(int32_t seq);

public:
  const char *GetPayload() const;

private:
  int seq;
  std::stringstream data;
};

} // namespace nacl_debug_conn

#endif