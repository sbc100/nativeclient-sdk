/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_INST_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_INST_H_ 1

#include <map>

#include "native_client/src/shared/platform/nacl_sync.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"

namespace nacl_debug_conn {
class DebugPacket;
class DebugPipe;
class DebugSocket;
} // namespace nacl_debug_conn

namespace nacl_debug_stub {

using nacl_debug_conn::DebugPacket;
using nacl_debug_conn::DebugPipe;
using nacl_debug_conn::DebugSocket;

class DebugThread;
class DebugInst;

class DebugInst {
 public:
  enum {
    DIF_BREAK_START = 1,
    DIF_BROKEN = 2
  };

  typedef uint64_t ThreadId_t;
  typedef std::map<ThreadId_t, DebugThread*> ThreadMap_t;
  typedef ThreadMap_t::const_iterator ThreadMapCItr_t;

  typedef std::map<uint64_t, char> BreakMap_t;

public:
  static DebugInst *DebugStub();
  void Launch(void *cookie);

public:
  bool SetEnvironment(DAPISetEnvironment_t *env);

  const char *GetLauncher() const;
  const char *GetNexe() const;

  uint64_t GetOffset() const;
  uint64_t GetStart() const;

  uint32_t GetFlags() const; 
  void SetFlagMask(uint32_t flags);
  void ClearFlagMask(uint32_t flags);

  bool HasPipe() const;

public:
  // Thread Access Functions
  DebugThread *GetCurrentThread();
  DebugThread *GetThread(ThreadId_t id);
 
  int GetThreadCount() const;
  DebugThread *GetThreadFirst();
  DebugThread *GetThreadNext();

  bool AddThread(DebugThread *dtp);
  bool RemoveThread(DebugThread *dtp);
  
  bool StopThreads();
  bool ResumeThreads();

  // Breakpoint Access functions
  bool AddBreakpoint(uint64_t offs);
  bool RemoveBreakpoint(uint64_t offs);
  bool RemoveAllBreakpoints();
  bool EnableBreakpoint(uint64_t offs);
  bool DisableBreakpoint(uint64_t offs);

  // Data Access functions
  DSResult GetDataBlock(uint64_t virt, void *dst, int len);
  DSResult SetDataBlock(uint64_t virt, void *src, int len);

public:
  // Stub thread access fuctions
  DSResult PacketPump();
  bool PrepSocketServer(const char *addr);
  bool GetPendingClient();
  bool FreeClient();


protected:
  // Packet processing members
  DSResult ProcessPacket(DebugPacket *in, DebugPacket *out);
  DSResult GetPacket(DebugPacket *pkt);
  DSResult SendPacket(DebugPacket *pkt);


 private:
  // Instance is created on first access
  explicit DebugInst();
  ~DebugInst();

  // Communication information
 private:
  DebugSocket* socketServer_;
  DebugPipe* pipe_;

private:
  uint32_t flags_;

  // NEXE Debugging information
 private:
  std::string exe_;
  std::string nexe_;
  uint64_t offset_; 
  uint64_t start_;
  void (*launcher_)(void *);

  BreakMap_t breaks_; 
  ThreadMap_t  threads_;
  ThreadMapCItr_t threadItr_;

  // Various caches, which are updated through "DIRTY" flags
 private:
  ThreadId_t currThread_;
  ThreadId_t selectedThread_;

  uint8_t currSignal_;

  mutable NaClMutex mtx_;
  static DebugInst *s_Instance;
};

} // namespace nacl_debug_stub
#endif