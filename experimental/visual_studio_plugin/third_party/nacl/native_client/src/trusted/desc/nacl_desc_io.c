/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Service Runtime.  I/O Descriptor / Handle abstraction.  Memory
 * mapping using descriptors.
 */

#include "native_client/src/include/portability.h"

#if NACL_WINDOWS
# include "io.h"
# include "fcntl.h"
#endif

#include "native_client/src/shared/imc/nacl_imc_c.h"
#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nacl_desc_effector.h"
#include "native_client/src/trusted/desc/nacl_desc_io.h"

#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/shared/platform/nacl_log.h"

#include "native_client/src/trusted/service_runtime/internal_errno.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_common.h"

#include "native_client/src/trusted/service_runtime/include/sys/errno.h"
#include "native_client/src/trusted/service_runtime/include/sys/fcntl.h"
#include "native_client/src/trusted/service_runtime/include/sys/mman.h"


/*
 * This file contains the implementation for the NaClIoDesc subclass
 * of NaClDesc.
 *
 * NaClDescIoDesc is the subclass that wraps host-OS descriptors
 * provided by NaClHostDesc (which gives an OS-independent abstraction
 * for host-OS descriptors).
 */

/*
 * Takes ownership of hd, will close in Dtor.
 */
int NaClDescIoDescCtor(struct NaClDescIoDesc  *self,
                       struct NaClHostDesc    *hd) {
  struct NaClDesc *basep = (struct NaClDesc *) self;

  basep->vtbl = (struct NaClDescVtbl *) NULL;
  if (!NaClDescCtor(basep)) {
    return 0;
  }
  self->hd = hd;
  basep->vtbl = &kNaClDescIoDescVtbl;
  return 1;
}

void NaClDescIoDescDtor(struct NaClDesc *vself) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;

  NaClLog(4, "NaClDescIoDescDtor(0x%08"NACL_PRIxPTR").\n",
          (uintptr_t) vself);
  NaClHostDescClose(self->hd);
  free(self->hd);
  self->hd = NULL;
  vself->vtbl = (struct NaClDescVtbl *) NULL;
  NaClDescDtor(&self->base);
}

struct NaClDescIoDesc *NaClDescIoDescMake(struct NaClHostDesc *nhdp) {
  struct NaClDescIoDesc *ndp;

  ndp = malloc(sizeof *ndp);
  if (NULL == ndp) {
    NaClLog(LOG_FATAL,
            "NaClDescIoDescMake: no memory for 0x%08"NACL_PRIxPTR"\n",
            (uintptr_t) nhdp);
  }
  if (!NaClDescIoDescCtor(ndp, nhdp)) {
    NaClLog(LOG_FATAL,
            ("NaClDescIoDescMake:"
             " NaClDescIoDescCtor(0x%08"NACL_PRIxPTR",0x%08"NACL_PRIxPTR
             ") failed\n"),
            (uintptr_t) ndp,
            (uintptr_t) nhdp);
  }
  return ndp;
}

struct NaClDescIoDesc *NaClDescIoDescOpen(char  *path,
                                          int   mode,
                                          int   perms) {
  struct NaClHostDesc *nhdp;

  nhdp = malloc(sizeof *nhdp);
  if (NULL == nhdp) {
    NaClLog(LOG_FATAL, "NaClDescIoDescOpen: no memory for %s\n", path);
  }
  if (!NaClHostDescOpen(nhdp, path, mode, perms)) {
    NaClLog(LOG_FATAL, "NaClDescIoDescOpen: NaClHostDescOpen failed for %s\n",
            path);
  }
  return NaClDescIoDescMake(nhdp);
}

uintptr_t NaClDescIoDescMap(struct NaClDesc         *vself,
                            struct NaClDescEffector *effp,
                            void                    *start_addr,
                            size_t                  len,
                            int                     prot,
                            int                     flags,
                            nacl_off64_t            offset) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;
  int                   rv;
  uintptr_t             status;

  /*
   * prot must be PROT_NONE or a combination of PROT_{READ|WRITE}
   */
  if (0 != (~(NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE) & prot)) {
    NaClLog(LOG_INFO,
            ("NaClDescIoDescMap: prot has other bits"
             " than PROT_{READ|WRITE}\n"));
    return -NACL_ABI_EINVAL;
  }

  if (0 == (NACL_ABI_MAP_FIXED & flags) && NULL == start_addr) {
    NaClLog(LOG_INFO,
            ("NaClDescIoDescMap: Mapping not NACL_ABI_MAP_FIXED"
             " but start_addr is NULL\n"));
  }

  if (0 != (rv = (*effp->vtbl->UnmapMemory)(effp,
                                            (uintptr_t) start_addr,
                                            len))) {
    NaClLog(LOG_FATAL,
            ("NaClDescIoDescMap: error %d --"
             " could not unmap 0x%08"NACL_PRIxPTR", length 0x%"NACL_PRIxS"\n"),
            rv,
            (uintptr_t) start_addr,
            len);
  }

  status = NaClHostDescMap((NULL == self) ? NULL : self->hd,
                           start_addr,
                           len,
                           prot,
                           flags,
                           offset);
  return status;
}

int NaClDescIoDescUnmapCommon(struct NaClDesc         *vself,
                              struct NaClDescEffector *effp,
                              void                    *start_addr,
                              size_t                  len,
                              int                     safe_mode) {
  int status;

  UNREFERENCED_PARAMETER(vself);
  UNREFERENCED_PARAMETER(effp);

  if (safe_mode) {
    status = NaClHostDescUnmap(start_addr, len);
  } else {
    status = NaClHostDescUnmapUnsafe(start_addr, len);
  }

  return status;
}

/*
 * NB: User code should never be able to invoke the Unsafe method.
 */
int NaClDescIoDescUnmapUnsafe(struct NaClDesc         *vself,
                              struct NaClDescEffector *effp,
                              void                    *start_addr,
                              size_t                  len) {
  return NaClDescIoDescUnmapCommon(vself, effp, start_addr, len, 0);
}

int NaClDescIoDescUnmap(struct NaClDesc         *vself,
                        struct NaClDescEffector *effp,
                        void                    *start_addr,
                        size_t                  len) {
  return NaClDescIoDescUnmapCommon(vself, effp, start_addr, len, 1);
}

ssize_t NaClDescIoDescRead(struct NaClDesc          *vself,
                           struct NaClDescEffector  *effp,
                           void                     *buf,
                           size_t                   len) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;

  UNREFERENCED_PARAMETER(effp);

  return NaClHostDescRead(self->hd, buf, len);
}

ssize_t NaClDescIoDescWrite(struct NaClDesc         *vself,
                            struct NaClDescEffector *effp,
                            void const              *buf,
                            size_t                  len) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;

  UNREFERENCED_PARAMETER(effp);

  return NaClHostDescWrite(self->hd, buf, len);
}

nacl_off64_t NaClDescIoDescSeek(struct NaClDesc          *vself,
                                struct NaClDescEffector  *effp,
                                nacl_off64_t             offset,
                                int                      whence) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;

  UNREFERENCED_PARAMETER(effp);

  return NaClHostDescSeek(self->hd, offset, whence);
}

int NaClDescIoDescIoctl(struct NaClDesc         *vself,
                        struct NaClDescEffector *effp,
                        int                     request,
                        void                    *arg) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;

  UNREFERENCED_PARAMETER(effp);

  return NaClHostDescIoctl(self->hd, request, arg);
}

int NaClDescIoDescFstat(struct NaClDesc         *vself,
                        struct NaClDescEffector *effp,
                        struct nacl_abi_stat    *statbuf) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;
  int                   rv;
  nacl_host_stat_t      hstatbuf;

  UNREFERENCED_PARAMETER(effp);

  rv = NaClHostDescFstat(self->hd, &hstatbuf);
  if (0 != rv) {
    return rv;
  }
  return NaClAbiStatHostDescStatXlateCtor(statbuf, &hstatbuf);
}

int NaClDescIoDescClose(struct NaClDesc         *vself,
                        struct NaClDescEffector *effp) {
  UNREFERENCED_PARAMETER(effp);

  NaClDescUnref(vself);
  return 0;
}

int NaClDescIoDescExternalizeSize(struct NaClDesc *vself,
                                  size_t          *nbytes,
                                  size_t          *nhandles) {
  UNREFERENCED_PARAMETER(vself);

  *nbytes = 0;
  *nhandles = 1;
  return 0;
}

/*
 * TODO(bsy) -- this depends on implementation of IMC being able to xfer
 * Unix descriptors or Windows Handles as NaclHandles.  Should this be
 * part of IMC requirements?  Also, Windows POSIX layer library may have
 * strange side effects due to extra buffering.  TEST AND DECIDE.
 */
int NaClDescIoDescExternalize(struct NaClDesc           *vself,
                              struct NaClDescXferState  *xfer) {
  struct NaClDescIoDesc *self = (struct NaClDescIoDesc *) vself;

#if NACL_WINDOWS
  HANDLE  h = (HANDLE) _get_osfhandle(self->hd->d);

  NaClLog(LOG_WARNING, "NaClDescIoDescExternalize is EXPERIMENTAL\n");
  NaClLog(LOG_WARNING, "handle 0x%x\n", (uintptr_t) h);

  *xfer->next_handle++ = (NaClHandle) h;
#else
  *xfer->next_handle++ = self->hd->d;
#endif
  return 0;
}

struct NaClDescVtbl const kNaClDescIoDescVtbl = {
  NaClDescIoDescDtor,
  NaClDescIoDescMap,
  NaClDescIoDescUnmapUnsafe,
  NaClDescIoDescUnmap,
  NaClDescIoDescRead,
  NaClDescIoDescWrite,
  NaClDescIoDescSeek,
  NaClDescIoDescIoctl,
  NaClDescIoDescFstat,
  NaClDescIoDescClose,
  NaClDescGetdentsNotImplemented,
  NACL_DESC_HOST_IO,
  NaClDescIoDescExternalizeSize,
  NaClDescIoDescExternalize,
  NaClDescLockNotImplemented,
  NaClDescTryLockNotImplemented,
  NaClDescUnlockNotImplemented,
  NaClDescWaitNotImplemented,
  NaClDescTimedWaitAbsNotImplemented,
  NaClDescSignalNotImplemented,
  NaClDescBroadcastNotImplemented,
  NaClDescSendMsgNotImplemented,
  NaClDescRecvMsgNotImplemented,
  NaClDescConnectAddrNotImplemented,
  NaClDescAcceptConnNotImplemented,
  NaClDescPostNotImplemented,
  NaClDescSemWaitNotImplemented,
  NaClDescGetValueNotImplemented,
};

/* set *baseptr to struct NaClDescIo * output */
int NaClDescIoInternalize(struct NaClDesc           **baseptr,
                          struct NaClDescXferState  *xfer) {
  int                   rv;
  NaClHandle            h;
  int                   d;
  struct NaClHostDesc   *nhdp;
  struct NaClDescIoDesc *ndidp;

  rv = -NACL_ABI_EIO;  /* catch-all */
  h = NACL_INVALID_HANDLE;
  nhdp = NULL;
  ndidp = NULL;

  if (xfer->next_handle == xfer->handle_buffer_end) {
    rv = -NACL_ABI_EIO;
    goto cleanup;
  }
  nhdp = malloc(sizeof *nhdp);
  if (NULL == nhdp) {
    rv = -NACL_ABI_ENOMEM;
    goto cleanup;
  }
  ndidp = malloc(sizeof *ndidp);
  if (!ndidp) {
    rv = -NACL_ABI_ENOMEM;
    goto cleanup;
  }
  h = *xfer->next_handle;
  *xfer->next_handle++ = NACL_INVALID_HANDLE;
#if NACL_WINDOWS
  if (-1 == (d = _open_osfhandle((intptr_t) h, _O_RDWR | _O_BINARY))) {
    rv = -NACL_ABI_EIO;
    goto cleanup;
  }
#else
  d = h;
#endif
  /*
   * We mark it as read/write, but don't really know for sure until we
   * try to make those syscalls (in which case we'd get EBADF).
   */
  if ((rv = NaClHostDescPosixTake(nhdp, d, NACL_ABI_O_RDWR)) < 0) {
    goto cleanup;
  }
  h = NACL_INVALID_HANDLE;  /* nhdp took ownership of h */

  if (!NaClDescIoDescCtor(ndidp, nhdp)) {
    rv = -NACL_ABI_ENOMEM;
    goto cleanup_hd_dtor;
  }
  /*
   * ndidp took ownership of nhdp, now give ownership of ndidp to caller.
   */
  *baseptr = (struct NaClDesc *) ndidp;
  rv = 0;
cleanup_hd_dtor:
  if (rv < 0) {
    (void) NaClHostDescClose(nhdp);
  }
cleanup:
  if (rv < 0) {
    free(nhdp);
    free(ndidp);
    if (NACL_INVALID_HANDLE != h) {
      NaClClose(h);
    }
  }
  return rv;
}
