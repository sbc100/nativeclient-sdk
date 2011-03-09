/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


/*
 * Nacl inter-module communication primitives.
 * Primitive NaCl socket and shared memory functions which provide a portable
 * inter-module communication mechanism between processes on Windows, Mac OS X,
 * and Unix variants. On Unix variants, these functions are simple wrapper
 * functions for the AF_UNIX domain socket API.
 */

#ifndef NATIVE_CLIENT_SRC_SHARED_IMC_NACL_IMC_C_H_
#define NATIVE_CLIENT_SRC_SHARED_IMC_NACL_IMC_C_H_

#include "native_client/src/include/portability.h"
#ifndef __native_client__
#include "native_client/src/trusted/service_runtime/include/machine/_types.h"
#endif  /* __native_client__ */


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* Get the last error message string. */
int NaClGetLastErrorString(char* buffer, size_t length);


/*
 * NaCl resource descriptor type NaCl resource descriptors can be
 * directly used with epoll on Linux, or with WaitForMultipleObject on
 * Windows.  TODO(sehr): work out one place for this definition.
 */

#ifndef __nacl_handle_defined
#define __nacl_handle_defined
#if NACL_WINDOWS
typedef HANDLE NaClHandle;
#else
typedef int NaClHandle;
#endif
#endif

#define NACL_INVALID_HANDLE ((NaClHandle) -1)

/* The maximum length of the zero-terminated pathname for SocketAddress */
#define NACL_PATH_MAX 28            /* TBD */

/*
 * A NaCl socket address is defined as a pathname. The pathname must
 * be a zero- terminated character string starting from a character
 * within the ranges A - Z or a - z. The pathname is not case
 * sensitive.
*/

typedef struct NaClSocketAddress {
  char path[NACL_PATH_MAX];
} NaClSocketAddress;

/*
 * I/O vector for the scatter/gather operation used by
 * NaClSendDatagram() and NaClReceiveDatagram()
*/

typedef struct NaClIOVec {
  void*  base;
  size_t length;
} NaClIOVec;

/* The maximum number of handles to be passed by NaClSendDatagram() */
#define NACL_HANDLE_COUNT_MAX 8     /* TBD */

/*
 * NaClMessageHeader flags set by NaClReceiveDatagram()
 */
/* The trailing portion of a message was discarded. */
#define NACL_MESSAGE_TRUNCATED 0x1
/* Not all the handles were received. */
#define NACL_HANDLES_TRUNCATED 0x2

/* Message header used by NaClSendDatagram() and NaClReceiveDatagram() */
typedef struct NaClMessageHeader {
  NaClIOVec*  iov;            /* scatter/gather array */
  size_t      iov_length;     /* number of elements in iov */
  NaClHandle* handles;        /* array of handles to be transferred */
  size_t      handle_count;   /* number of handles in handles */
  int         flags;
} NaClMessageHeader;

/*
 * Creates a NaCl socket associated with the local address.
 *
 * NaClBoundSocket() returns a handle of the newly created
 * bound socket on success, and NACL_INVALID_HANDLE on failure.
 */
NaClHandle NaClBoundSocket(const NaClSocketAddress* address);

/*
 * Creates an unnamed pair of connected sockets.  NaClSocketPair()
 * return 0 on success, and -1 on failure.
 */
int NaClSocketPair(NaClHandle pair[2]);

/*
 * Closes a NaCl descriptor created by NaCl primitives.
 *
 * NaClClose() returns 0 on success, and -1 on failure. Note NaCl
 * descriptors must be explicitly closed by NaClClose(). Otherwise,
 * the resources of the underlining operating system will not be
 * released correctly.
 */
int NaClClose(NaClHandle handle);

/*
 * NaClSendDatagram()/NaClReceiveDatagram() flags
 */

#define NACL_DONT_WAIT 0x1  /* Enables non-blocking operation */

/*
 * Checks the last non-blocking operation was failed because no
 * message is available in the queue.  WouldBlock() returns non-zero
 * value if the previous non-blocking NaClSendDatagram() or
 * NaClReceiveDatagram() operation would block if NACL_DONT_WAIT was
 * not specified.
 */
int NaClWouldBlock();

/*
 * Sends a message on a socket.
 *
 * NaClSendDatagram() and NaClSend() send the message to the remote
 * peer of the connection created by SocketPair().
 *
 * NaClSendDatagramTo() and NaClSendTo() send the message to the
 * socket specified by the name. The socket parameter should be a
 * socket created by NaClBoundSocket().
 *
 * The send functions return the number of bytes sent, or -1 upon
 * failure.  If NACL_DONT_WAIT flag is specified with the call and the
 * other peer of the socket is unable to receive more data, the
 * function returns -1 without waiting, and the subsequent
 * NaClWouldBlock() will return non-zero value.
 *
 * Note it is not safe to send messages from the same socket handle by
 * multiple threads simultaneously unless the destination address is
 * explicitly specified by NaClSendDatagramTo() or NaClSendTo().
 */
int NaClSendDatagram(NaClHandle socket, const NaClMessageHeader* message,
                     int flags);
int NaClSendDatagramTo(NaClHandle socket, const NaClMessageHeader* message,
                       int flags, const NaClSocketAddress* name);
int NaClSend(NaClHandle socket, const void* buffer, size_t length, int flags);
int NaClSendTo(NaClHandle socket, const void* buffer, size_t length, int flags,
               const NaClSocketAddress* name);

/*
 * Receives a message from a socket.
 *
 * The receive functions return the number of bytes received,
 * or -1 upon failure.
 *
 * If NACL_DONT_WAIT flag is specified with the call and no messages are
 * available in the queue, the function returns -1 and the subsequent
 * NaClWouldBlock() will return non-zero value. Internally, in this case
 * ERROR_PIPE_LISTENING is set to the last error code on Windows and EAGAIN is
 * set to errno on Linux.
 *
 * Note it is not safe to receive messages from the same socket handle
 * by multiple threads simultaneously unless the socket handle is created
 * by NaClBoundSocket().
 */

int NaClReceiveDatagram(NaClHandle socket, NaClMessageHeader* message,
                        int flags);
int NaClReceive(NaClHandle socket, void* buffer, size_t length, int flags);

/*
 * Creates a memory object of length bytes.
 *
 * NaClCreateMemoryObject() returns a handle of the newly created
 * memory object on success, and NACL_INVALID_HANDLE on failure.
 * length must be a multiple of allocation granularity given by
 * NACL_MAP_PAGESIZE in nacl_config.h.
 */

NaClHandle NaClCreateMemoryObject(size_t length);

/* NaClMap() prot bits */
#define NACL_PROT_READ    0x1   /* Mapped area can be read */
#define NACL_PROT_WRITE   0x2   /* Mapped area can be written */
#define NACL_PROT_EXEC    0x4   /* Mapped area can be executed */

/* NaClMap() flags */
#define NACL_MAP_SHARED   0x1   /* Create a sharable mapping with other */
                                /* processes */
#define NACL_MAP_PRIVATE  0x2   /* Create a private copy-on-write mapping */
#define NACL_MAP_FIXED    0x4   /* Try to create a mapping at the specified */
                                /* address */

#define NACL_MAP_FAILED   ((void*) -1)

/*
 * Maps the specified memory object in the process address space.
 *
 * NaClMap() returns a pointer to the mapped area, or NACL_MAP_FAILED upon
 * error.
 * For prot, the bitwise OR of the NACL_PROT_* bits must be specified.
 * For flags, either NACL_MAP_SHARED or NACL_MAP_PRIVATE must be specified.
 * If NACL_MAP_FIXED is also set, NaClMap() tries to map the memory object at
 * the address specified by start.
 */
void* NaClMap(void* start, size_t length, int prot, int flags,
              NaClHandle memory, off_t offset);

/*
 * Unmaps the memory objects mapped within the specified process
 * address space range.
 *
 * NaClUnmap() returns 0 on success, and -1 on failure.
 */
int NaClUnmap(void* start, size_t length);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* NATIVE_CLIENT_SRC_SHARED_IMC_NACL_IMC_C_H_ */
