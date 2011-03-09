/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_SOCKET_IMPL_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_SOCKET_IMPL_H_ 1


#include "native_client/src/include/portability.h"

EXTERN_C_BEGIN

// Maximum string length of a Network Address
// For IPv4 addresses this is really only xxx.xxx.xxx.xxx:yyyyy
// But we allow larger to deal with name resolution in the future
#define MAX_ADDR_LEN 256

typedef enum {
  DSE_ERROR = -1,
  DSE_TIMEOUT = 0,
  DSE_OK = 1
} DSError;

#define DEBUG_SOCKET_BAD ((void *) -1)
typedef void *DSHandle;

// Platform dependant calls
DSError DebugSocketInit();
DSError DebugSocketExit();
DSError DebugSocketCreate(DSHandle *handle);
DSError DebugSocketClose(DSHandle handle);
int DebugSocketGetError(int can_block);

// Platform independant calls
DSError DebugSocketAccept(DSHandle srv, DSHandle *sock, char *addr, uint32_t max);
DSError DebugSocketBind(DSHandle handle, const char *addr);
DSError DebugSocketConnect(DSHandle handle, const char *addr);
DSError DebugSocketListen(DSHandle handle, uint32_t cnt);
DSError DebugSocketRecv(DSHandle handle, void *data, int32_t max, int32_t *len);
DSError DebugSocketSend(DSHandle handle, void *data, int32_t max, int32_t *len);
DSError DebugSocketRecvAvail(DSHandle handle, uint32_t ms_usec);
DSError DebugSocketSendAvail(DSHandle handle, uint32_t ms_usec);

DSError DebugSocketStrToAddr(const char *saddr, void *daddr, uint32_t len);
DSError DebugSocketAddrToStr(void *saddr, uint32_t len, char *daddr, uint32_t max);
DSError DebugSocketAddrSize(uint32_t *len);
DSError DebugSocketLogError(const char *file, int line, int block_ok);

EXTERN_C_END

#endif

