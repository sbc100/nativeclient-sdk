/*
 * Copyright (c) 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl simple RPC over IMC mechanism.
 * This module is used to build SRPC connections in both trusted (e.g., the
 * browser plugin, and untrusted (e.g., a NaCl module) settings.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/shared/srpc/nacl_srpc_internal.h"

#ifdef __native_client__
/**
 * Note: nacl/nacl_inttypes.h must be included last...
 * after all other types headers.
 */
#include <inttypes.h>
#include <nacl/nacl_inttypes.h>
#else
#include "native_client/src/include/portability.h"
#endif

#ifndef SIZE_T_MAX
# define SIZE_T_MAX (~((size_t) 0))
#endif

/*
 * Buffers differ between trusted code and untrusted primarily by
 * the use of NaClImcTypedMsgHdr (trusted) or NaClImcMsgHdr (untrusted).
 * These two structs differ by whether they contain vectors of NaClDesc*
 * (ndescv, for trusted) or int (descv, untrusted), and correspondingly,
 * their counts, ndesc_length (trusted) and desc_length (untrusted).  To
 * avoid replicating unnecessarily, we define these two macros.
 */
#ifdef __native_client__
#define NACL_SRPC_IMC_HEADER_DESCV(buf)        (buf).header.descv
#define NACL_SRPC_IMC_HEADER_DESC_LENGTH(buf)  (buf).header.desc_length
#define NACL_SRPC_IMC_INVALID_DESC             -1
#else  /* trusted code */
#define NACL_SRPC_IMC_HEADER_DESCV(buf)        (buf).header.ndescv
#define NACL_SRPC_IMC_HEADER_DESC_LENGTH(buf)  (buf).header.ndesc_length
#define NACL_SRPC_IMC_INVALID_DESC             NULL
#endif  /* __native_client__ */

/*
 * IMC wrapper functions.
 */

void __NaClSrpcImcBufferCtor(NaClSrpcImcBuffer* buffer, int is_write_buf) {
  buffer->iovec[0].base = buffer->bytes;
  buffer->iovec[0].length = sizeof(buffer->bytes);
  buffer->header.iov = buffer->iovec;
  buffer->header.iov_length = sizeof(buffer->iovec) / sizeof(buffer->iovec[0]);
  NACL_SRPC_IMC_HEADER_DESCV(*buffer) = buffer->descs;
  if (is_write_buf) {
    NACL_SRPC_IMC_HEADER_DESC_LENGTH(*buffer) = 0;
  } else {
    NACL_SRPC_IMC_HEADER_DESC_LENGTH(*buffer) =
        NACL_ARRAY_SIZE(buffer->descs);
  }
  buffer->header.flags = 0;
  /* Buffers start out empty */
  buffer->next_byte = 0;
  buffer->last_byte = 0;
}

/*
 * __NaClSrpcImcRead attempts to read n_elt entities of size elt_size into
 * target from buffer.  It returns the number of elements read if successful,
 * and -1 otherwise.
 */
nacl_abi_size_t __NaClSrpcImcRead(NaClSrpcImcBuffer* buffer,
                                  nacl_abi_size_t elt_size,
                                  nacl_abi_size_t n_elt,
                                  void* target) {
  nacl_abi_size_t request_bytes;
  nacl_abi_size_t avail_bytes = buffer->last_byte - buffer->next_byte;

  if (n_elt >= NACL_ABI_SIZE_T_MAX / elt_size) {
    return -1;
  }
  request_bytes = n_elt * elt_size;
  if (avail_bytes >= request_bytes) {
    /* Buffer contains enough data to fully satisfy request */
    memcpy(target, (void*)(buffer->bytes + buffer->next_byte), request_bytes);
    buffer->next_byte += request_bytes;
    return n_elt;
  } else {
    dprintf((SIDE "READ: insufficient bytes read to satisfy request.\n"));
    return -1;
  }
}

/*
 * __NaClSrpcImcReadDesc reads a NaCl resource descriptor from the specified
 * buffer.  It returns a valid descriptor if successful and -1 (untrusted)
 * or NULL (trusted) otherwise.
 */
NaClSrpcImcDescType __NaClSrpcImcReadDesc(NaClSrpcImcBuffer* buffer) {
  uint32_t desc_index = buffer->next_desc;

  if (desc_index >= NACL_SRPC_IMC_HEADER_DESC_LENGTH(*buffer)) {
    return NACL_SRPC_IMC_INVALID_DESC;
  } else {
    NaClSrpcImcDescType desc = buffer->descs[desc_index];
    buffer->next_desc++;
    return desc;
  }
}

/*
 * __NaClSrpcImcRefill moves the buffer pointers for a read buffer back to
 * the beginning of the buffer.  (It effectively undoes all the reads from the
 * current buffer.)
 */
void __NaClSrpcImcRefill(NaClSrpcImcBuffer* buffer) {
  buffer->next_byte = 0;
  buffer->next_desc = 0;
}

/*
 * __NaClSrpcImcWrite attempts to write n_elt elements of size elt_size from
 * source to the specified buffer.  It returns the number of elements it
 * wrote if successful, and -1 otherwise.
 */
nacl_abi_size_t __NaClSrpcImcWrite(const void* source,
                                   nacl_abi_size_t elt_size,
                                   nacl_abi_size_t n_elt,
                                   NaClSrpcImcBuffer* buffer) {
  nacl_abi_size_t request_bytes;
  nacl_abi_size_t avail_bytes;
  /*
   * What follows works on the assumption that buffer->bytes is an array,
   * rather than a pointer to a buffer.  If it were the latter, the subtraction
   * would almost invariably be negative, producing a massive size_t value
   * and allowing heap corruptions.
   */
#ifndef __native_client__ /* these checks are unnecessary in an ILP32 module */
  CHECK(sizeof(buffer->bytes) <= NACL_ABI_SIZE_T_MAX);
  CHECK(sizeof(buffer->bytes) >= buffer->next_byte);
#endif /* __native_client__ */
  avail_bytes =
      nacl_abi_size_t_saturate(sizeof(buffer->bytes) - buffer->next_byte);

  /*
   * protect against future change of buffer->bytes to be dynamically
   * allocated, since otherwise avail_bytes becomes huge
   */
  NACL_ASSERT_IS_ARRAY(buffer->bytes);

  if (n_elt >= SIZE_T_MAX / elt_size) {
    return -1;
  }
  request_bytes = n_elt * elt_size;
  /*
   * Writes are not broken into multiple datagram sends for now either.
   */
  if (avail_bytes < request_bytes) {
    dprintf((SIDE "WRITE: insufficient space available to satisfy.\n"));
    return -1;
  } else {
    memcpy((void*)(buffer->bytes + buffer->next_byte), source, request_bytes);
    buffer->next_byte += request_bytes;
    return n_elt;
  }
}

/*
 * __NaClSrpcImcWriteDesc writes a NaCl resource descriptor to the specified
 * buffer.  It returns 1 if successful, 0 otherwise.
 */
int __NaClSrpcImcWriteDesc(NaClSrpcImcDescType desc,
                           NaClSrpcImcBuffer* buffer) {
  nacl_abi_size_t desc_index = NACL_SRPC_IMC_HEADER_DESC_LENGTH(*buffer);

  if (SRPC_DESC_MAX <= desc_index) {
    return 0;
  } else {
    buffer->descs[desc_index] = desc;
    NACL_SRPC_IMC_HEADER_DESC_LENGTH(*buffer)++;
    return 1;
  }
}

/*
 * __NaClSrpcImcFillBuf fills buffer from an IMC channel.  It returns 1 if
 * successful, and 0 otherwise.
 */
NaClSrpcImcBuffer* __NaClSrpcImcFillbuf(NaClSrpcChannel* channel) {
  NaClSrpcImcBuffer* buffer;
  ssize_t            retval;
  double             start_usec = 0.0;
  double             this_usec;

  buffer = &channel->receive_buf;
  buffer->iovec[0].base = buffer->bytes;
  buffer->iovec[0].length = sizeof(buffer->bytes);
  if (channel->timing_enabled) {
    start_usec = __NaClSrpcGetUsec();
  }
  NACL_SRPC_IMC_HEADER_DESC_LENGTH(*buffer) = SRPC_DESC_MAX;
#ifdef __native_client__
  retval = imc_recvmsg(channel->imc_handle, &buffer->header, 0);
#else
  retval = NaClImcRecvTypedMessage(channel->imc_handle,
                                   (struct NaClDescEffector*) &channel->eff,
                                   &buffer->header,
                                   0);
#endif
  buffer->next_desc = 0;
  if (channel->timing_enabled) {
    this_usec = __NaClSrpcGetUsec();
    channel->imc_read_usec += this_usec;
  }
  if (!NaClIsNegErrno(retval)) {
    channel->receive_buf.next_byte = 0;
    channel->receive_buf.last_byte = nacl_abi_size_t_saturate(retval);
  } else {
    dprintf((SIDE "READ: read failed.\n"));
    return NULL;
  }
  return buffer;
}

/*
 * __NaClSrpcImcFlush send the contents of a write buffer over the IMC
 * channel. It returns 1 if successful, or 0 otherwise.
 */
int __NaClSrpcImcFlush(NaClSrpcImcBuffer* buffer, NaClSrpcChannel* channel) {
  ssize_t        retval;
  double         start_usec = 0.0;
  double         this_usec;
  buffer->iovec[0].length = buffer->next_byte;
  if (channel->timing_enabled) {
    start_usec = __NaClSrpcGetUsec();
  }
#ifdef __native_client__
  retval = imc_sendmsg(channel->imc_handle, &buffer->header, 0);
#else
  retval = NaClImcSendTypedMessage(channel->imc_handle,
                                   (struct NaClDescEffector*) &channel->eff,
                                   &buffer->header,
                                   0);
#endif
  NACL_SRPC_IMC_HEADER_DESC_LENGTH(*buffer) = 0;
  buffer->next_desc = 0;
  if (channel->timing_enabled) {
    this_usec = __NaClSrpcGetUsec();
    channel->imc_write_usec += this_usec;
  }
  buffer->next_byte = 0;
  if ((size_t) retval != buffer->iovec[0].length) {
    dprintf((SIDE "FLUSH: send error.\n"));
    return 0;
  }
  dprintf((SIDE "FLUSH: complete send.\n"));
  return 1;
}
