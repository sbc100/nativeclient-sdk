/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <string>
#include <sstream>

#include "debug_conn/debug_packet.h"
#include "debug_conn/debug_pipe.h"
#include "debug_conn/debug_socket.h"
#include "debug_conn/debug_stream.h"
#include "debug_conn/debug_util.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"

using namespace nacl_debug_conn;
using std::string;
using std::stringstream;


DebugPipe::DebugPipe(DebugStream *io_ptr)
  : io_(0),
    seq_(0) {
  io_ = io_ptr;
}

DebugPipe::~DebugPipe() {
  if (io_) {
    delete io_;
    io_ = 0;
  }
}

DebugStream *DebugPipe::GetIO() {
  return io_;
}

void DebugPipe::SetName(const char *name) {
  name_ = name;
}

const char *DebugPipe::GetName() const {
  return name_.data();
}

bool DebugPipe::DataAvail() const {
  if (io_)
    return io_->DataAvail();

  return false;
}


DebugPipe::DPResult DebugPipe::GetChar(char *ch) {
  int32_t len = GetIO()->Read(ch, 1);
  if (len == -1)
    return DPR_ERROR;

  if (len == 0)
    return DPR_NO_DATA;

  return DPR_OK;
}


DebugPipe::DPResult DebugPipe::SendPacket(DebugPacket *pkt) {
  DebugPipe::DPResult res;
  char ch;

  // If we are ignoring ACKs..
  if (GetFlags() & DPF_IGNORE_ACK)
    return SendPacketOnly(pkt);

  do {
    res = SendPacketOnly(pkt);
    // Verify we sent OK
    if (res != DPR_OK)
      break;

    // If ACKs are off, we are done.
    if (GetFlags() & DPF_IGNORE_ACK)
      break;
    // Otherwise, poll for '+'
    if (GetChar(&ch) == DPR_ERROR)
      return DPR_ERROR;
    // Retry if we didn't get a '+'
  } while (ch != '+');

  return res;
}

DebugPipe::DPResult DebugPipe::SendPacketOnly(DebugPacket *pkt) {
  const unsigned char *ptr = (const unsigned char*)(pkt->GetPayload());
  unsigned char ch;
  stringstream outstr;
  const char* curr_function = "DebugPipe::SendPacketOnly";
  unsigned char run_xsum = 0;
  int32_t seq;

  if ((pkt->GetSequence(&seq) == false) && (GetFlags() & DPF_USE_SEQ)) {
    pkt->SetSequence(seq_++);
  }
  debug_log_info("Inside %s\n", curr_function);
  // Signal start of response
  outstr << '$';

  // Note: we are no longer looking for sequence numbers.

  // Send the main payload
  int offs = 0;
  while (ch = ptr[offs++]) {
    outstr << ch;
    run_xsum += ch;
  }

  // Send XSUM as two nible 8bit value preceeded by '#'
  outstr << '#';
  ch = debug_int_to_nibble(run_xsum >> 4);
  outstr << ch;
  ch = debug_int_to_nibble(run_xsum & 0xF);
  outstr << ch;
  delete[] ptr;
  return SendStream(outstr.str().data());
}

DebugPipe::DPResult DebugPipe::SendStream(const char *out) {
  int32_t len = static_cast<int32_t>(strlen(out));
  int32_t sent= 0;

  while (sent < len) {
    char *cur = const_cast<char *>(&out[sent]);
    int32_t tx = GetIO()->Write(cur, len - sent);

    if (tx <= 0) {
      debug_log_warning("DebugPipe::SendStream %d bytes : '%s' failed.\n",
                        len, out);
      return DPR_ERROR;
    }

    sent += tx;
  }

  if (GetFlags() & DPF_DEBUG_SEND)
    debug_log_info("TX %s:%s\n", name_.c_str(), out);
  return DPR_OK;
}


// Attempt to receive a packet
DebugPipe::DPResult DebugPipe::GetPacket(DebugPacket *pkt) {
  const char *curr_function = "DebugPipe::GetPacket";
  char run_xsum, fin_xsum, ch;
  stringstream in;
  int has_seq, offs;

  // If nothing is waiting, return NONE
  if (GetIO()->DataAvail() == DPR_NO_DATA) {
    debug_log_warning("%s return DPR_NO_DATA\n", curr_function);
    return DPR_NO_DATA;
  }

  // Toss characters until we see a start of command
  do {
    if (GetChar(&ch) == DPR_ERROR) {
      debug_log_warning("%s return DPR_ERROR\n", curr_function);
      return DPR_ERROR;
    }
    in << ch;
  } while (ch != '$');

 retry:
  has_seq = 1;
  offs    = 0;

  // If nothing is waiting, return NONE
  if (GetIO()->DataAvail() == DPR_NO_DATA) {
    debug_log_warning("%s return DPR_NO_DATA 2\n", curr_function);
    return DPR_NO_DATA;
  }

  // Clear the stream
  pkt->Clear();

  // Prepare XSUM calc
  run_xsum = 0;
  fin_xsum = 0;

  // Stream in the characters
  while (1) {
    if (GetChar(&ch) == DPR_ERROR) {
      debug_log_warning("%s return DPR_ERROR 3\n", curr_function);
      return DPR_ERROR;
    }

    in << ch;
    // Check SEQ statemachine  xx:
    switch(offs) {
      case 0:
      case 1:
        if (debug_nibble_to_int(ch) == -1)
          has_seq = 0;
        break;

      case 2:
        if (ch != ':')
          has_seq = 0;
        break;
    }
    offs++;

    // If we see a '#' we must be done with the data
    if (ch == '#')
      break;

    // If we see a '$' we must have missed the last cmd
    if (ch == '$') {
      debug_log_info("%s RX Missing $, retry.\n", curr_function);
      goto retry;
    }
    // Keep a running XSUM
    run_xsum += ch;
    pkt->AddRawChar(ch);
  }

  // Get two Nibble XSUM
  if (GetChar(&ch) == DPR_ERROR) {
    debug_log_warning("%s return DPR_ERROR 5\n", curr_function);
    return DPR_ERROR;
  }
  in << ch;
  fin_xsum  = debug_nibble_to_int(ch) << 4;

  if (GetChar(&ch) == DPR_ERROR)
    return DPR_ERROR;
  in << ch;
  fin_xsum |= debug_nibble_to_int(ch);

  if (GetFlags() & DPF_DEBUG_RECV) {
    string str = in.str();
    debug_log_info("RX [%s] [%s]\n",
                   str.c_str(),
                   curr_function);
  }

  // FIXME -- dead code below? No packets should have a sequence number
  // Pull off the sequence number if we have one
  if (has_seq) {
    uint8_t seq;
    char ch;

    pkt->GetByte(&seq);
    pkt->SetSequence(seq);
    pkt->GetRawChar(&ch);
    debug_log_error("ERROR IN %s has seq is TRUE\n", curr_function);
    if (ch != ':') {
      debug_log_error("ERROR %s RX mismatched SEQ seq=%d ch=%d.\n",
                      curr_function, seq, ch);
      return DPR_ERROR;
    }
  }

  // If the XSUMs don't match, signal bad packet
  if (fin_xsum == run_xsum) {
    char out[4] = { '+', 0, 0, 0};

    // We are no longer checking for sequence number and adding one
    // on if needed.
    debug_log_info("%s calling SendStream [%s]\n", curr_function, out);
    return SendStream(out);
  } else {
    // Resend a bad XSUM and look for retransmit
    SendStream("-");
    debug_log_info("%s RX Bad XSUM, retry\n", curr_function);
    goto retry;
  }

  debug_log_info("%s returning DPR_OK at bottom\n", curr_function);
  return DPR_OK;
}


