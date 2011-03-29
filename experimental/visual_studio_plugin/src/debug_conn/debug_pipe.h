/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_PIPE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_PIPE_H_ 1

/*
 * This module provides interfaces for transmitting and receiving
 * packets that conform to the GDB serial line debug protocol.  
 *
 * The packet are transmitted and received in the form of:
 * $[<SQ>:]<C>[<..data...>]#<XS>
 *
 *
 *  Where
 *	<SQ>  : is an optional two hex digit sequence number followed by ':'
 *  <C>   : is a single character Command
 *  <data>: is the optinal paramters, payload, etc... 
 *  <XS>  : is an 8 bit sum as two hex digits preceeded by '#'
 *
 *  Upon receit, the receiver will reply with either
 *  -      : to signal a bad XSUM
 *  +[SQ]  : to signal valid packet with the sequence if provided
 *
 *
 */

#include <string>
#include <sstream>

#include "native_client/src/include/portability.h"
#include "debug_conn/debug_flags.h"

namespace nacl_debug_conn {

class DebugPipe;
class DebugPacket;
class DebugStream;

class DebugPipe : public DebugFlags {
public:
  explicit DebugPipe(DebugStream *io_ptr);
  ~DebugPipe();

  enum DPResult {
    DPR_ERROR = -1,   // IO error on the stream, close the pipe
    DPR_NO_DATA = 0,  // No data availible
    DPR_OK = 1        // Completed OK
  };

  enum {
    DPF_IGNORE_ACK = 1,
    DPF_USE_SEQ = 2,
    DPF_DEBUG_SEND = 4,
    DPF_DEBUG_RECV = 8,
    DPF_DEBUG_MASK = (DPF_DEBUG_SEND | DPF_DEBUG_RECV)
  };

public:
  void SetName(const char *name);
  const char *GetName() const;

  DPResult SendPacketOnly(DebugPacket *packet);
  DPResult SendPacket(DebugPacket *packet);
  DPResult GetPacket(DebugPacket *packet);

  bool DataAvail() const;

protected:
  DebugStream *GetIO();

  DPResult GetChar(char *ch);
  DPResult SendStream(const char *str);

private:
  std::string name_;
  char outxsum_;
  uint8_t seq_;

  DebugStream *io_;
};

} // namespace nacl_debug_conn

#endif