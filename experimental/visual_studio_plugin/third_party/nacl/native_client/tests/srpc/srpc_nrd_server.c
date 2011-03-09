/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nacl/nacl_srpc.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/nacl_syscalls.h>
#include <sys/stat.h>

/*
 * NativeClient server side of a test for passing socket addresses.  The
 * server side steps are:
 * 1) The JavaScript bridge invokes the start_server method.
 * 2) StartServer creates a bound socket and socket address pair.
 * 3) StartServer creates a server pthread listening on the bound socket.
 * 4) StartServer returns the socket address to the JavaScript bridge.
 * 5) The JavaScript bridge starts a client, either in itself or another
 *    NativeClient module.
 * 6) The server accepts a connection, setting up an SRPC server on the
 *    resulting connected socket.
 * 7) The server handles an RPC to get a message.
 * 8) The JavaScript bridge invokes the report message to find out if there
 *    were errors.
 */

/* The count of errors seen during the test.  Returned by the report method. */
int errors_seen = 0;

/*
 * ServerThread awaits connection on the specified descriptor.  Once connected,
 * ServerThread runs an SRPC server that has one method.
 */
void* ServerThread(void* desc);

/* StartServer creates a new SRPC server thread each time it is invoked. */
NaClSrpcError StartServer(NaClSrpcChannel *channel,
                          NaClSrpcArg **in_args,
                          NaClSrpcArg **out_args) {
  int            pair[2];
  pthread_t      server;
  int            rv;

  /* Create a pair (bound socket, socket address). */
  printf("StartServer: creating bound socket\n");
  rv = imc_makeboundsock(pair);
  if (rv == 0) {
    printf("StartServer: bound socket %d, address %d\n", pair[0], pair[1]);
  } else {
    printf("StartServer: bound socket creation FAILED, errno %d\n", errno);
    ++errors_seen;
  }
  /* Pass the socket address back to the caller. */
  out_args[0]->u.hval = pair[1];
  /* Start the server, passing ownership of the bound socket. */
  printf("StartServer: creating server...\n");
  rv = pthread_create(&server, NULL, ServerThread, (void*) pair[0]);
  if (rv == 0) {
    printf("StartServer: server created\n");
  } else {
    printf("StartServer: server creation FAILED, errno %d\n", errno);
    ++errors_seen;
  }
  /* Return success. */
  printf("START RPC FINISHED\n");
  return NACL_SRPC_RESULT_OK;
}

NACL_SRPC_METHOD("start_server::h", StartServer);

/* GetMsg simply returns a string in a character array. */
static NaClSrpcError GetMsg(NaClSrpcChannel *channel,
                            NaClSrpcArg **in_args,
                            NaClSrpcArg **out_args) {
  static char message[] = "Quidquid id est, timeo Danaos et dona ferentes";

  printf("ServerThread: GetMsg\n");

  if (out_args[0]->u.caval.count >= strlen(message)) {
    strncpy(out_args[0]->u.caval.carr, message, strlen(message) + 1);
    return NACL_SRPC_RESULT_OK;
  } else {
    printf("GetMsg: u.caval.count %u is too small %u\n",
           (unsigned) out_args[0]->u.caval.count,
           (unsigned) strlen(message));
    ++errors_seen;
    return NACL_SRPC_RESULT_APP_ERROR;
  }
}

/* Shutdown stops the RPC service. */
static NaClSrpcError Shutdown(NaClSrpcChannel *channel,
                              NaClSrpcArg **in_args,
                              NaClSrpcArg **out_args) {
  printf("ServerThread: Shutdown\n");

  return NACL_SRPC_RESULT_BREAK;
}

void* ServerThread(void* desc) {
  int connected_desc;

  printf("ServerThread: waiting on %d to accept connections...\n",
         (int) desc);
  fflush(stdout);
  /* Wait for connections from the client. */
  connected_desc = imc_accept((int) desc);
  if (connected_desc >= 0) {
    static struct NaClSrpcHandlerDesc methods[] = {
      { "getmsg::C", GetMsg },
      { "shutdown::", Shutdown },
      { NULL, NULL }
    };

    /* Export the server on the connected socket descriptor. */
    if (!NaClSrpcServerLoop(connected_desc, methods, NULL)) {
      printf("SRPC server loop failed.\n");
      ++errors_seen;
    }
    printf("ServerThread: shutting down\n");
    /* Close the connected socket */
    if (0 != close(connected_desc)) {
      printf("ServerThread: connected socket close failed.\n");
      ++errors_seen;
    }
  } else {
    printf("ServerThread: connection FAILED, errno %d\n", errno);
    ++errors_seen;
  }
  /* Close the bound socket */
  if (0 != close((int) desc)) {
    printf("ServerThread: bound socket close failed.\n");
    ++errors_seen;
  }

  printf("THREAD EXIT\n");
  return 0;
}

/*
 * TestSharedMemory tests the passing of shared memory regions.
 * It expects to receive a handle and a string.
 */
NaClSrpcError TestSharedMemory(NaClSrpcChannel *channel,
                               NaClSrpcArg **in_args,
                               NaClSrpcArg **out_args) {
  int desc = in_args[0]->u.hval;
  char* compare_string = in_args[1]->u.sval;
  char* map_addr;
  struct stat st;

  printf("TestSharedMemory(%d, %s)\n", desc, compare_string);
  if (fstat(desc, &st)) {
    printf("TestSharedMemory: fstat failed\n");
    ++errors_seen;
  } else {
    size_t length = (size_t) st.st_size;
    printf("TestSharedMemory: fstat returned 0x%08x\n", length);
    /* Map the shared memory region into the NaCl module's address space. */
    map_addr = (char*) mmap(NULL,
                            length,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            desc,
                            0);
    if (MAP_FAILED == map_addr) {
      printf("TestSharedMemory: map failed\n");
      ++errors_seen;
    } else {
      printf("TestSharedMemory: mapped memory string: %12s\n", map_addr);
      if (strncmp(map_addr, compare_string, strlen(compare_string))) {
        printf("TestSharedMemory: strncmp failed\n");
        ++errors_seen;
      } else {
        const char* reply = "Quod erat demonstrandum";
        strncpy(map_addr + strlen(compare_string), reply, strlen(reply));
        /* Unmap the memory region. */
        if (munmap((void*) map_addr, length)) {
          printf("TestSharedMemory: munmap failed\n");
          ++errors_seen;
        } else {
          /* Close the passed-in descriptor. */
          if (close(desc)) {
            printf("TestSharedMemory: close failed\n");
            ++errors_seen;
          }
        }
      }
    }
  }
  /* Report the number of errors seen back as the result. */
  out_args[0]->u.ival = errors_seen;
  printf("TestSharedMemory: returning errors %d\n", errors_seen);
  return NACL_SRPC_RESULT_OK;
}

NACL_SRPC_METHOD("test_shared_memory:hs:i", TestSharedMemory);

/* Report reports the number of errors seen during the tests. */
NaClSrpcError Report(NaClSrpcChannel *channel,
                     NaClSrpcArg **in_args,
                     NaClSrpcArg **out_args) {
  out_args[0]->u.ival = errors_seen;
  return NACL_SRPC_RESULT_OK;
}

NACL_SRPC_METHOD("report::i", Report);
