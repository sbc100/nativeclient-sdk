/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
#include <winsock2.h>

#include "debug_conn/debug_socket_impl.h"
#include "debug_conn/debug_util.h"

#define CHECK_ERROR()   DebugSocketLogError(__FILE__, __LINE__, 1)
#define PROCESS_ERROR() DebugSocketLogError(__FILE__, __LINE__, 0)

static int s_SocketsAvailible = 0;
DSError DebugSocketInit()
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  // Make sure to request the use of sockets.
  // NOTE:  It is safe to call Startup multiple times
  wVersionRequested = MAKEWORD(2, 2);
  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
	  // We could not find a matching DLL
	  debug_log_error("WSAStartup failed with error: %d\n", err);
	  return DSE_ERROR;
  }

  if (HIBYTE(wsaData.wVersion) != 2) 
  {
	  // We couldn't get a matching version
	  debug_log_error("Could not find a usable version of Winsock.dll\n");
	  WSACleanup();
	  return DSE_ERROR;
  }

  s_SocketsAvailible = 1;
  return DSE_OK;
} 

DSError DebugSocketExit() {
  // Make sure to signal we are done with sockets
  // NOTE:  It must be called as many times as Startup.
  if (s_SocketsAvailible)
  {
	  s_SocketsAvailible = 0;
	  WSACleanup();
  }

  return DSE_OK;
}

DSError DebugSocketCreate(DSHandle *h) {
  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == s)
    return PROCESS_ERROR();

  *h = (DSHandle) s;
  return DSE_OK;
}


DSError DebugSocketClose(DSHandle handle) {
  SOCKET s = (SOCKET) handle;

  // Check this isn't already invalid
  if (-1 == s)
    return DSE_OK;

  if (shutdown(s, SD_BOTH))
    return CHECK_ERROR();

  // If not then close it
  if (closesocket(s) != 0)
    return CHECK_ERROR();

  return DSE_OK;
}


int DebugSocketGetError(int block_ok) {
  int err = GetLastError();

  if (block_ok && (WSAEWOULDBLOCK == err))
    return 0;

  return err;
}

