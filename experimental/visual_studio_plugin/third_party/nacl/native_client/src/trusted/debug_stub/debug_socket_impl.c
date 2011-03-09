/*
 * Copyright 2010 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#ifdef WIN32
# include <winsock2.h>
# include <stdio.h>
# define snprintf _snprintf
typedef int socklen_t;
#else
# include <arpa/inet.h>
# include <sys/socket.h>
# include <errno.h>
# include <unistd.h>
#endif


#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/debug_stub/debug_socket_impl.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

#define CHECK_ERROR()   DebugSocketLogError(__FILE__, __LINE__, 1)
#define PROCESS_ERROR() DebugSocketLogError(__FILE__, __LINE__, 0)


DSError DebugSocketLogError(const char *file, int line, int block_ok) {
  int err = DebugSocketGetError(block_ok);

  // If this is a REAL error, then log it
  if (err) {
    debug_log_error("%s(%d) : Socket error %d.\n",
      file, line, err);
    return DSE_ERROR;
  }

  // Otherwise return what is effectively "WOULDBLOCK"
  return DSE_TIMEOUT;
}


DSError DebugSocketAccept(DSHandle in,
                           DSHandle *out,
                           char *addr,
                           uint32_t max) {
  struct sockaddr_in saddr;
  socklen_t addrlen = (socklen_t)(sizeof(saddr));

  SOCKET s;
  s = accept((SOCKET) in, (struct sockaddr *) &saddr, &addrlen);

  // Check if we WOULDBLOCK
  if (-1 == s)
    return CHECK_ERROR();

  *out = (DSHandle) s;

  DebugSocketAddrToStr(&saddr, addrlen, addr, max);
  return DSE_OK;
}

DSError DebugSocketBind(DSHandle handle, const char *addr) {
  struct sockaddr_in saddr;
  socklen_t addrlen = (socklen_t) sizeof(saddr);

  if (DebugSocketStrToAddr(addr, &saddr, addrlen) != DSE_OK)
    return DSE_ERROR;

  if (bind((SOCKET) handle, (struct sockaddr *) &saddr, addrlen))
    return PROCESS_ERROR();

  return DSE_OK;
}

DSError DebugSocketConnect(DSHandle handle, const char *addr) {
  struct sockaddr_in saddr;
  socklen_t addrlen = (socklen_t) sizeof(saddr);

  if (DebugSocketStrToAddr(addr, &saddr, addrlen) != DSE_OK)
    return DSE_ERROR;

  DebugSocketStrToAddr(addr, &saddr, addrlen);

  // Check if we WOULDBLOCK
  if (connect((SOCKET) handle,
              (struct sockaddr *) &saddr,
              sizeof(saddr)))
    return CHECK_ERROR();

  return DSE_OK;
}

DSError DebugSocketListen(DSHandle handle, uint32_t cnt) {
  if (listen((SOCKET) handle, cnt) ==  -1)
    return PROCESS_ERROR();

  return DSE_OK;
}

DSError DebugSocketSend(DSHandle handle, void *data, int32_t max, int32_t *len) {
  int res = (int) send((SOCKET) handle, (char *)(data), max, 0);

  // If we have nothing, then the socket must have closed
  if (0 == res) {
    *len  = 0;
    return DSE_TIMEOUT;
  }

  if (-1 == res) {
    // If error (or would block), transfer size must be zero

    *len = 0;
    return CHECK_ERROR();
  }

  *len = res;
  return DSE_OK;
}

DSError DebugSocketRecv(DSHandle handle, void *data, int32_t max, int32_t *len) {
  int res = (int) recv((SOCKET) handle, (char *)(data), max, 0);

  // If we have nothing, then the socket must have closed
  if (0 == res) {
    *len  = 0;
    return DSE_TIMEOUT;
  }

  if (-1 == res) {
    // If error (or would block), transfer size must be zero
    *len = 0;
    return CHECK_ERROR();
  }

  *len = res;
  return DSE_OK;
}

DSError DebugSocketRecvAvail(DSHandle handle, uint32_t ms_usec) {
  struct timeval timeout;
  fd_set fds;
  int cnt;

  FD_ZERO(&fds);
  FD_SET((SOCKET) handle, &fds);

  // We want a "non-blocking" check
  timeout.tv_sec = 0;
  timeout.tv_usec= ms_usec * 1000;

  // Check if this file handle can select on read
  cnt = select(0, &fds, 0, &fds, &timeout);
  if (-1 == cnt)
      return PROCESS_ERROR();

  if (cnt > 0)
    return DSE_OK;

  return DSE_TIMEOUT;
}

DSError DebugSocketSendAvail(DSHandle handle, uint32_t ms_usec) {
  fd_set fds;
  struct timeval timeout;
  int cnt;

  FD_ZERO(&fds);
  FD_SET((SOCKET) handle, &fds);

  // We want a "non-blocking" check
  timeout.tv_sec = 0;
  timeout.tv_usec= ms_usec * 1000;

  // Check if this file handle can select on write
  cnt = select(0, 0, &fds, 0, &timeout);
  if (-1 == cnt)
      return PROCESS_ERROR();

  if (cnt > 0)
    return DSE_OK;

  return DSE_TIMEOUT;
}


DSError DebugSocketStrToAddr(const char *saddr, void *daddr, uint32_t len) {
  struct sockaddr_in *saddr_in = (struct sockaddr_in *) daddr;
  char *ip_port[2];
  int ip_port_cnt = 0;

  char *octets[4];
  int octets_cnt = 0;

  unsigned host = 0;
  unsigned port = 0;

  if (len != sizeof(struct sockaddr_in))
    return DSE_ERROR;

  ip_port_cnt = debug_get_tokens(saddr, ':', ip_port, 2);
  if (ip_port_cnt > 0) {
    octets_cnt = debug_get_tokens(ip_port[0], '.', octets, 4);
    if (4 == octets_cnt) {
      int a;
      for (a = 0; a < 4; a++) {
         host <<= 8;
         host |= (atoi(octets[a]) & 0xFF);
      }
    }
    debug_free_tokens(octets, octets_cnt);
  }
  if (ip_port_cnt > 1) {
    port = atoi(ip_port[1]);
  }
  debug_free_tokens(ip_port, ip_port_cnt);

  saddr_in->sin_family = AF_INET;
  saddr_in->sin_addr.s_addr = htonl(host);
  saddr_in->sin_port = htons(port);

  return DSE_OK;
}


DSError DebugSocketAddrToStr(void *saddr, uint32_t len, char *daddr, uint32_t max) {
  char tmp[MAX_ADDR_LEN];
  struct sockaddr_in *saddr_in = (struct sockaddr_in *)(saddr);

  unsigned int host;
  unsigned int port;

  if (len != sizeof(struct sockaddr_in))
    return DSE_ERROR;

  host = htonl(saddr_in->sin_addr.s_addr);
  port = htons(saddr_in->sin_port);

  if (snprintf(tmp,
          sizeof(tmp),
          "%d.%d.%d.%d:%d",
          (host >> 24) & 0xFF,
          (host >> 16) & 0xFF,
          (host >>  8) & 0xFF,
          (host >>  0) & 0xFF,
          port) > (int) max)
    return DSE_ERROR;

  strcpy(daddr, tmp);
  return DSE_OK;
}

DSError DebugSocketAddrSize(uint32_t *len) {
  if (NULL == len)
    return DSE_ERROR;

  *len = (uint32_t) sizeof(struct sockaddr_in);
  return DSE_OK;
}

