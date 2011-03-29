/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_STUB_VSX_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_STUB_VSX_H_ 1

#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"

EXTERN_C_BEGIN

// Handle for any OS dependant handle, or 'C' object reference
typedef void *DebugHandle;

// The result of any DebugStub Operation
typedef enum {
  DS_ERROR = -1,    // Error on the object
  DS_NONE  = 0,     // Nothing to do
  DS_OK    = 1      // Success
} DSResult;


enum {
  // Create a server socket to wait for connections.
  DAPI_SET_ENV,       // Set the running environment
  DAPI_SOCKER_SERVER, // Create a socket server
  DAPI_INSERT_BREAK,  // Insert a breakpoint at 
  DAPI_CREATE_THREAD, // Create a thread object
  DAPI_ADD_THREAD,    // Add a thread object to the instance

  DAPI_GET_INFO,      // Get general information
};

//
// Interface for using a debug interface
//
DSResult NaClDebugStubDispatch(int cmd, void *src, int len);
void WINAPI NaClDebugStubThreadStart(void *cookie);


typedef struct {
  const char *exe;
  const char *nexe;
  uint64_t offset;
  uint64_t start; 
  void (*launcher)(void *state);
  uint8_t startBroken;
} DAPISetEnvironment_t;

typedef struct {
  void *cookie;
  void (WINAPI *start_fn)(void *);
  void *handle;
} DAPICreateThread_t;

//
// 'C' Interfaces for debugging
//

uint32_t NaClDebugStubReprotect(void *ptr, uint32_t len, uint32_t flags);
void NaClGetFilePath(const char *file, char *out, uint32_t max);
const char *NaClDebugSigToStr(int sig);

/* NMM */
extern void *g_Dbg;

EXTERN_C_END

#endif