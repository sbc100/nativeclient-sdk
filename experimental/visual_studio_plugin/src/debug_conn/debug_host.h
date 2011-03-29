/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_HOST_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_HOST_H_ 1

#include <map>
#include "native_client/src/include/portability.h"
#include "debug_conn/debug_flags.h"
/*
 * This module provides interfaces for the host side of the
 * connection.  
 *
 */


namespace nacl_debug_conn {

class DebugPipe;
class DebugPacket;

class DebugHost : public DebugFlags {
 private:
  explicit DebugHost(DebugPipe *pipe);

 public:
  enum DHResult {
    DHR_BUSY   = -4,   // Target is busy (running)
    DHR_FAILED = -3,   // Transaction completed with failure
    DHR_LOST = -2,     // Lost connection during transaction
    DHR_TIMEOUT =-1,   // Transaction Timed out
    DHR_PENDING = 0,   // Transaction is pending as expected
    DHR_OK = 1         // Transaction Succeeded
  };

  enum {
    DHF_RUNNING = 1  // Thread info is stale, so reload.
  };

 public:
   typedef void (__stdcall *DHAsync)(DHResult res, void *obj);
   typedef void (__stdcall *DHAsyncStr)(DHResult res, void *obj, const char *str);
   typedef void (__stdcall *DHAsyncMem)(DHResult res, void *obj, void* data, uint32_t len);

 public:
  ~DebugHost();
  static DebugHost *SocketConnect(const char *addr);

 public:
  DHResult SendStringAsync(const char *str, DHAsyncStr reply, void *obj);  
  DHResult SendString(const char *str, const char **ppReply);
  void FreeString(const char *pstr);

  bool IsRunning();
  void SetOutputAsync(DHAsyncStr reply, void *obj);
  void SetStopAsync(DHAsync reply, void *obj);
  
  DHResult GetPathAsync(DHAsyncStr reply, void *obj);
  DHResult GetArchAsync(DHAsyncStr reply, void *obj);
  DHResult GetThreadsAsync(DHAsyncStr reply, void *obj);
  
public:  
  DHResult GetLastSig(int *sig);

  DHResult GetMemory(uint64_t offs, void *buf, uint32_t max);
  DHResult SetMemory(uint64_t offs, void *data, uint32_t max);

  DHResult GetRegisters(void *data, uint32_t max);
  DHResult SetRegisters(void *data, uint32_t size);

  DHResult RequestBreak();    // Attempts to immediately break 
  DHResult RequestContinue(); // Continues execution, blocking until next exception.
  DHResult RequestStep();     // Continues exectuion for one step or until next exception.

  DHResult RequestContinueBackground(); // Continues execution, returns immediately
  DHResult RequestStepBackground();     // Attempts to step but returns immediately

  struct BreakpointRecord {
    uint64_t offs;
    bool enabled;
    bool suspended;
    char previousContents;
  };

  bool HasBreakpoint(uint64_t offs);
  DHResult AddBreakpoint(uint64_t offs);
  DHResult RemoveBreakpoint(uint64_t offs);
  DHResult EnableBreakpoint(uint64_t offs);
  DHResult DisableBreakpoint(uint64_t offs);
  DHResult SuspendBreakpoint(uint64_t offs);
  DHResult ResumeBreakpoint(uint64_t offs);
  DHResult QueryBreakpoint(uint64_t offs, BreakpointRecord* out_result);

protected:
  DHResult FetchThreadInfo();
  DHResult Transact(DebugPacket *outPkt, DebugPacket *in);
  DHResult SendAndWaitForBreak(const char *str, bool wait);
  DHResult BreakpointStatusChanged(uint64_t offs);

private:
  DebugPipe *pipe_;
  std::map<uint64_t, BreakpointRecord> breaks_;
  DHAsyncStr outputFunc_;
  void *outputObj_;
  DHAsync stopFunc_;
  void *stopObj_;

};


} // namespace nacl_debug_conn


#endif