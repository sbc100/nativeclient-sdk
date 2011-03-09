/*
 * Copyright 2010 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
#include <string>
#include <vector>
using std::string;
using std::vector;

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/debug_stub/debug_socket_impl.h"
#include "native_client/src/trusted/debug_stub/debug_stream.h"


#define CHECK_ERROR()   LogError(__FILE__, __LINE__, 1)
#define PROCESS_ERROR() LogError(__FILE__, __LINE__, 0)

#ifdef WIN32
#include <winsock2.h>

// Include these windows libraries during link
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "wininet.lib")

static int GetSocketError(int block_ok) {
  int err = GetLastError();
  
  if (block_ok && (err == WSAEWOULDBLOCK))
    return 0;

  return err;
}
#else

#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#ifndef SOCKET
#define SOCKET int
#endif 

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

static int closesocket(SOCKET s) {
  return close(s);
}

static int GetSocketError(int block_ok) {
  if (block_ok && (errno == EWOULDBLOCK))
    return 0;

  return errno;
}

#endif 

// Maximum string length of a Network Address
// For IPv4 addresses this is really only xxx.xxx.xxx.xxx:yyyyy
// But we allow larger to deal with name resolution in the future
#define MAX_ADDR_LEN 256


static DSResult LogError(const char *file, int line, int block_ok) {
  int err = GetSocketError(block_ok);

  // If this is a REAL error, then log it
  if (err) {
    NaClLog(LOG_WARNING, "%s(%d) : Socket error %d.\n",
      file, line, err);
    return DS_ERROR;
  }

  // Otherwise return what is effectively "WOULDBLOCK"
  return DS_NONE;
}


DSResult DebugSocketCreate(DebugHandle *h) {
  SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  *h = (DebugHandle) s;

  if (s == INVALID_SOCKET)
    return PROCESS_ERROR();

  return DS_OK;
}


DSResult DebugSocketClose(DebugHandle handle) {
  SOCKET s = (SOCKET) handle;

  // Check this isn't already invalid
  if (s == INVALID_SOCKET)
    return DS_OK;

  // If not then close it
  if (closesocket(s) != 0)
    return CHECK_ERROR();

  return DS_OK;
}

DSResult DebugSocketAccept(DebugHandle in, DebugHandle *out, string *addr) {
  sockaddr_in saddr;	
  int addrlen = sizeof(saddr);
  SOCKET s;
	
  s = ::accept((SOCKET) in, (sockaddr *) &saddr, &addrlen);	

  // Check if we WOULDBLOCK
  if (s == INVALID_SOCKET)
    return CHECK_ERROR();

  *out = (DebugHandle) s;
  
  DebugSocketAddrToStr(&saddr, addrlen, addr);
  return DS_OK;
}

DSResult DebugSocketBind(DebugHandle handle, const string &addr) {
  sockaddr_in saddr;
  int addrlen = sizeof(saddr);

  if (DebugSocketStrToAddr(addr, &saddr, addrlen) != DS_OK)
    return DS_ERROR;

  if (::bind((SOCKET) handle, (sockaddr *) &saddr, addrlen))
	  return PROCESS_ERROR();

  return DS_OK;
}

DSResult DebugSocketConnect(DebugHandle handle, const string& addr) {
  sockaddr_in saddr;
  int addrlen = sizeof(saddr);

  if (DebugSocketStrToAddr(addr, &saddr, addrlen) != DS_OK)
    return DS_ERROR;

  DebugSocketStrToAddr(addr, &saddr, addrlen);

  // Check if we WOULDBLOCK
  if (::connect((SOCKET) handle, (sockaddr *) &saddr, sizeof(saddr)))
	  return CHECK_ERROR();

  return DS_OK;
}

DSResult DebugSocketListen(DebugHandle handle, int cnt) {
  if (::listen((SOCKET) handle, cnt) ==  SOCKET_ERROR)
	  return PROCESS_ERROR();

  return DS_OK;
}

DSResult DebugSocketSend(DebugHandle handle, void *data, int max, int *len) {
  int res = ::send((SOCKET) handle, (char *) data, max, 0);

  // If we have nothing, then the socket must have closed
  if (res == 0)
    return DS_ERROR;

  if (res == SOCKET_ERROR)
  {
    // If error (or would block), transfer size must be zero
    *len = 0;	
    return CHECK_ERROR();
  }

  *len = res;
  return DS_OK;    
}

DSResult DebugSocketRecv(DebugHandle handle, void *data, int max, int *len) {
  int res = ::recv((SOCKET) handle, (char *) data, max, 0);

  // If we have nothing, then the socket must have closed
  if (res == 0)
    return DS_ERROR;

  if (res == SOCKET_ERROR)
  {
    // If error (or would block), transfer size must be zero
    *len = 0;	
    return CHECK_ERROR();
  }

  *len = res;
  return DS_OK;
}

DSResult DebugSocketCanRecv(DebugHandle handle, int ms_usec) {
  fd_set fds;

  FD_ZERO(&fds);
  FD_SET((SOCKET) handle, &fds);

  // We want a "non-blocking" check
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec= ms_usec * 1000;

  // Check if this file handle can select on read
  int cnt = select(0, &fds, 0, 0, &timeout);
  if (cnt == SOCKET_ERROR)
	    return PROCESS_ERROR();
  
  if (cnt > 0)
    return DS_OK;

  return DS_NONE;
}

DSResult DebugSocketCanSend(DebugHandle handle, int ms_usec) {
  fd_set fds;

  FD_ZERO(&fds);
  FD_SET((SOCKET) handle, &fds);

  // We want a "non-blocking" check
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec= ms_usec * 1000;

  // Check if this file handle can select on write
  int cnt = select(0, 0, &fds, 0, &timeout);
  if (cnt == SOCKET_ERROR)
	    return PROCESS_ERROR();
  
  if (cnt > 0)
    return DS_OK;

  return DS_NONE;
}

vector<string> Tokenizer(const string &in, char delim) {
  char *str   =strdup(in.data());
  char *start= str;
  char *word = str;
  
  vector<string> tokens;
  for (;*str; str++) {
    if (*str == delim) {      
      // Make this null, so we can copy it
      *str = 0;

      // Add it to the array;
      tokens.push_back(word);
      
      // Start scanning after the delim
      str++;
      word = str;
    }
  }

  if (*word)
    tokens.push_back(word);

  free(start);
  return tokens;
}

DSResult DebugSocketStrToAddr(const string &saddr, void *daddr, int len) {
  if (len != sizeof(sockaddr_in))
    return DS_ERROR;

  unsigned host = 0;
  unsigned port = 0;

  vector<string> ipport = Tokenizer(saddr, ':');
  if (ipport.size() > 0) {
    vector<string> bytes = Tokenizer(ipport[0], '.');
    if (bytes.size() == 4) {
      for (int a=0; a < 4; a++) {
         host <<= 8;
         host |= (atoi(bytes[a].data()) & 0xFF);
      }
    }
  }
  if (ipport.size() > 1) {
    port = atoi(ipport[1].data());
  }

  sockaddr_in *saddr_in = (sockaddr_in *) daddr;
  saddr_in->sin_family = AF_INET;
  saddr_in->sin_addr.s_addr = htonl(host);
  saddr_in->sin_port = htons(port);

  return DS_OK;
}


DSResult DebugSocketAddrToStr(void *saddr, int len, string *daddr) {
  char tmp[MAX_ADDR_LEN];

  if (len != sizeof(sockaddr_in))
    return DS_ERROR;

  sockaddr_in *saddr_in = (sockaddr_in *) saddr;
  unsigned int host = htonl(saddr_in->sin_addr.s_addr);
  unsigned int port = htons(saddr_in->sin_port);
 
  sprintf(tmp, 
          "%d.%d.%d.%d:%d",
          (host >> 24) & 0xFF,
          (host >> 16) & 0xFF,
          (host >>  8) & 0xFF,
          (host >>  0) & 0xFF,
          port);
     
  *daddr = tmp;
  return DS_OK;
}

