/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_THREAD_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_THREAD_H_ 1

#include "debug_flags.h"

struct NaClThreadContext;
struct NaClAppThread;

namespace nacl_debug_stub {

class DebugInst;
class DebugThread;

class DebugThread : public nacl_debug_conn::DebugFlags {
public:
  typedef uint32_t ThreadId_t;
  typedef void* ThreadHandle_t;   

  enum DTFlags {
    DTF_SYSCALL = 1
  };
  
  enum DTResult {
    DTR_WAIT = 0,
    DTR_KILL = 1,
    DTR_STEP = 2,
    DTR_CONT = 3,
  };

  enum DTArch {
    DTA_UNKNOWN= 0,
    DTA_X86_32 = 1,
    DTA_X86_64 = 2
  };

public:
  explicit DebugThread(ThreadId_t id, ThreadHandle_t hdl, intptr_t fun, void *cookie);
  ~DebugThread();

public:
  bool SetStep(bool step);
  bool Break();

  void *GetContextPtr();
  uint32_t GetContextSize() const;
  ThreadId_t GetID() const;
  ThreadHandle_t GetHandle() const;
  
  struct NaClThreadContext *GetUserCtx();
  NaClAppThread *GetNaClAppThread();
  void SetNaClAppThread(NaClAppThread * natp);


private:
  intptr_t start_fn_;
  void* cookie_;
  NaClAppThread *natp_;

  ThreadId_t id_;     
  ThreadHandle_t handle_;
  DTArch arch_;

public:
  void *registers_;  
  volatile uint32_t sig_;
  volatile DTResult res_;

  friend class DebugInst;
};


//
// Platform dependant helpers
// 
uint32_t NaClDebugStubThreadContextSize();
bool NaClDebugStubThreadPause(DebugThread *dtp);
bool NaClDebugStubThreadResume(DebugThread *dtp);
bool NaClDebugStubThreadGetContext(DebugThread *dtp);
bool NaClDebugStubThreadSetContext(DebugThread *dtp);

} // namespace nacl_debug_stub
#endif