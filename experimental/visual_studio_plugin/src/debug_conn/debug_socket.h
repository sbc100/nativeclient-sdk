/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_SOCKET_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_SOCKET_H_ 1

#include "debug_conn/debug_socket_impl.h"
#include "debug_conn/debug_stream.h"
#include "native_client/src/include/portability.h"

namespace nacl_debug_conn {

class DebugSocket : public DebugStream {
 public:
  enum DSState {
    DSS_INVALID   = 0,  // No Socket
    DSS_UNBOUND   = 1,  // Has Socket, but is unbound
    DSS_BOUND     = 2,  // Socket has been bound
    DSS_LISTEN    = 3,  // Socket is listening for connections
    DSS_CONNECTED = 4   // Socket is connected
  };

 public:
  virtual ~DebugSocket();

 public:
  static DebugSocket *CreateServer(const char *addr, int outstanding);
  static DebugSocket *CreateClient(const char *addr);
  DebugSocket *Accept();

 public:
  int32_t Read(void *ptr, int32_t len);
  int32_t Write(void *ptr, int32_t len);
  bool IsConnected() const;
  bool DataAvail() const;

 protected:
  DSState GetState() const;
  void SetState(DSState state);

  uint32_t GetTimeout() const;
  void SetTimeout(uint32_t msec);

 private:
  DebugSocket();
  bool Construct();
  void Destruct();

  void* GetHandle() const;
  void SetHandle(void *h);

 private:
  DSState state_;
  DSHandle handle_;
  uint32_t msec_timeout_;
};

} // namespace nacl_debug_conn
#endif

