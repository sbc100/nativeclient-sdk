// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_NACL_GDB_SERVER_RSP_STOP_FROM_DEBUG_EVENT_H_
#define DEBUGGER_NACL_GDB_SERVER_RSP_STOP_FROM_DEBUG_EVENT_H_

#include "debugger/rsp/rsp_common_replies.h"
#include "debugger/core/debug_event.h"

namespace rsp {
static const int kSIGNO = 0;  // no signal
static const int kSIGSEGV = 11;  // segmentation fault
static const int kSIGSYS = 31;  // bad argument to a system call
static const int kSIGTRAP = 5;  // trace trap
static const int kSIGBUS = 7;  // bus error
static const int kSIGFPE = 8;  // erroneous arithmetic operation, does not
                               // necessarily involve floating-point arithmetic
static const int kSIGILL = 4;  // illegal instruction
static const int kSIGINT = 2;  // user wishes to interrupt the process
static const int kSIGSTOP = 19;  // process is paused

/// @param[in] debug_event debug event to be converted to rsp::StopReply
/// @return rsp::StopReply representation of |debug_event|.
StopReply CreateStopReplyPacket(const debug::DebugEvent& debug_event);

/// @param[in] debug_event debug event to be converted to GDB signal number
/// @return GDB signal number
int GetSignalNumber(const debug::DebugEvent& debug_event);

/// @param[in] Windows exception code to be converted to GDB signal number
/// @return GDB signal number
///
/// For possible Window exceptions, look here:
/// http://msdn.microsoft.com/en-us/library/aa363082%28v=VS.85%29.aspx
///
/// For GDB signals look here:
/// <gdb_source_root>/include/gdb/signals.h
/// or here: http://en.wikipedia.org/wiki/Signal_%28computing%29
int GetSignalNumberForException(int exception_code);
}  // namespace rsp

#endif  // DEBUGGER_NACL_GDB_SERVER_RSP_STOP_FROM_DEBUG_EVENT_H_

