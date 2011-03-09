/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "native_client/src/trusted/debug_stub/debug_socket.h"
#include "native_client/src/trusted/debug_stub/debug_socket_impl.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

using namespace nacl_debug_conn;

static const char *StateToString(DebugSocket::DSState state_) {
  switch (state_) {
    case DebugSocket::DSS_INVALID: return "INVALID";
    case DebugSocket::DSS_UNBOUND: return "UNBOUND";
    case DebugSocket::DSS_BOUND: return "BOUND";
    case DebugSocket::DSS_LISTEN: return "LISTEN";
    case DebugSocket::DSS_CONNECTED: return "CONNECTED";
    default:
      break;
  }

  return "<UNKNOWN>";
}


DebugSocket::DebugSocket() 
 : state_(DSS_INVALID),
   handle_(DEBUG_SOCKET_BAD),
   msec_timeout_(1000) {
   DebugSocketInit();
}

DebugSocket::~DebugSocket() {
  if (GetHandle() != DEBUG_SOCKET_BAD)
    Destruct();
  DebugSocketExit();
}

bool DebugSocket::Construct() {
  if (state_ != DSS_INVALID) {
      debug_log_error("Socket can not construct from state_ %s.\n",
        StateToString( GetState() ));
        return false;
  }

  DSHandle handle;
  if (DebugSocketCreate(&handle) == DSE_OK) {
    SetState(DSS_UNBOUND);
    SetHandle(handle);
    return true;
  }

  return false;
}

void DebugSocket::Destruct() {
  if (state_ != DSS_INVALID) {
    DebugSocketClose( GetHandle() );
  }

  SetHandle(DEBUG_SOCKET_BAD);
  SetState(DSS_INVALID);
}


DebugSocket::DSState DebugSocket::GetState() const {
  return state_;
}

void DebugSocket::SetState(DSState s) {
  state_ = s;
}

void* DebugSocket::GetHandle() const {
  return handle_;
}

void DebugSocket::SetHandle(void *h) {
  handle_ = h;
}

uint32_t DebugSocket::GetTimeout() const {
  return msec_timeout_;
}

void DebugSocket::SetTimeout(uint32_t msec) {
  msec_timeout_ = msec;
}



DebugSocket *DebugSocket::CreateServer(const char *addr, 
                                              int outstanding) {
  DebugSocket *serv = new DebugSocket();
  serv->Construct();
  
  if (DebugSocketBind(serv->GetHandle(), addr) == DSE_OK)
  {
      serv->SetState(DSS_BOUND);
      if (DebugSocketListen(serv->GetHandle(), outstanding) == DSE_OK) {
        serv->SetState(DSS_LISTEN);
        return serv;
      }
      else
        debug_log_error("Failed to listen on '%s'.\n", addr);
  }
  else 
    debug_log_error("Failed to bind server on '%s'.\n", addr);
      
  delete serv;
  return 0;
}

DebugSocket *DebugSocket::CreateClient(const char *addr) {
  DebugSocket *client = new DebugSocket();
  client->Construct();

  if (DebugSocketConnect(client->GetHandle(), addr) == DSE_OK) {
    client->SetState(DSS_CONNECTED);
    return client;
  }

  delete client;
  return 0;
}

DebugSocket *DebugSocket::Accept() {
  DSHandle newHandle;
  DebugSocket *newSocket;

  if (state_ != DSS_LISTEN) {
      debug_log_error("Socket can not accept from state_ %s.\n",
        StateToString(state_));
      return 0;
  }

  if (DebugSocketAccept(GetHandle(), &newHandle, 0, 0) == DSE_OK) {
    newSocket = new DebugSocket();
    newSocket->SetHandle(newHandle);
    newSocket->SetState(DSS_CONNECTED);
    return newSocket;
  }

  return 0;
}

int32_t DebugSocket::Read(void *ptr, int32_t len) {
  if (len == 0) 
    return 0;

  if (DebugSocketRecv(GetHandle(), ptr, len, &len) == DSE_ERROR) {
    Destruct();  
    return -1;
  }

  if (len == 0) {
    Destruct();
    return -1;
  }

  return len;
}

int DebugSocket::Write(void *ptr, int32_t len) {
  if (len == 0) 
    return 0;

  if (DebugSocketSend(handle_, ptr, len, &len) == DSE_ERROR) {
    Destruct();  
    return -1;
  }

  if (len == 0) {
    Destruct();
    return -1;
  }

  return len;
}

bool DebugSocket::IsConnected() const {
  return GetState() == DSS_CONNECTED;
}

bool DebugSocket::DataAvail() const {
  return DebugSocketRecvAvail(GetHandle(), GetTimeout()) !=  DSE_TIMEOUT;
}

