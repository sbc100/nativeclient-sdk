/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <string>


#include "native_client/src/trusted/debug_stub/debug_packet.h"
#include "native_client/src/trusted/debug_stub/debug_pipe.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"
#include "native_client/src/trusted/debug_stub/debug_stream.h"
#include "native_client/src/trusted/debug_stub/debug_socket.h"
#include "native_client/src/trusted/debug_stub/debug_target.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

using namespace nacl_debug_conn;
using std::string;

                             
const char *nacl_debug_conn::RegisterIndexToName(uint32_t regIndex) {
  switch (regIndex) {
    case 0: return "Rax"; 
    case 1: return "Rbx"; 
    case 2: return "Rcx"; 
    case 3: return "Rdx"; 
    case 4: return "Rsi"; 
    case 5: return "Rdi"; 
    case 6: return "Rbp"; 
    case 7: return "Rsp"; 
    case 8: return "R8"; 
    case 9: return "R9";
    case 10:return "R10"; 
    case 11:return "R11"; 
    case 12:return "R12"; 
    case 13:return "R13"; 
    case 14:return "R14"; 
    case 15:return "R15"; 
    case 16:return "Rip"; 
    case 17:return "EFlags"; 
    case 18:return "SegCs"; 
    case 19:return "SegSs"; 
    case 20:return "SegDs"; 
    case 21:return "SegEs"; 
    case 22:return "SegFs"; 
    case 23:return "SegGs"; 
    default: return "UNKN";
  }
}
