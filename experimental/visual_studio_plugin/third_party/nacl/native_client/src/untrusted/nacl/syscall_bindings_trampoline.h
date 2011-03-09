/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_UNTRUSTED_SYSCALL_BINDINGS_TRAMPOLINE_H
#define NATIVE_CLIENT_SRC_UNTRUSTED_SYSCALL_BINDINGS_TRAMPOLINE_H

#if __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/bits/nacl_syscalls.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"

struct NaClImcMsgHdr;
struct stat;
struct timeval;
struct timespec;
union NaClMultimediaEvent;

#define NACL_SYSCALL(s) ((TYPE_nacl_ ## s) NACL_SYSCALL_ADDR(NACL_sys_ ## s))
/* ============================================================ */
/* files */
/* ============================================================ */

typedef int (*TYPE_nacl_read) (int desc, void *buf, size_t count);

typedef int (*TYPE_nacl_close) (int desc);

typedef int (*TYPE_nacl_fstat) (int fd, struct stat *stbp);

typedef int (*TYPE_nacl_write) (int desc, void const *buf, size_t count);

typedef int (*TYPE_nacl_open) (char const *pathname, int flags, mode_t mode);

typedef off_t (*TYPE_nacl_lseek) (int desc, off_t offset, int whence);

typedef int (*TYPE_nacl_stat) (const char *file, struct stat *st);

/* ============================================================ */
/* imc */
/* ============================================================ */

typedef int (*TYPE_nacl_imc_recvmsg) (int desc,
                                      struct NaClImcMsgHdr *nmhp,
                                      int flags);
typedef int (*TYPE_nacl_imc_sendmsg) (int desc,
                                      struct NaClImcMsgHdr const *nmhp,
                                      int flags);
typedef int (*TYPE_nacl_imc_accept) (int d);

typedef int (*TYPE_nacl_imc_connect) (int d);

typedef int (*TYPE_nacl_imc_makeboundsock) (int *dp);

typedef int (*TYPE_nacl_imc_socketpair) (int *d2);

typedef int (*TYPE_nacl_imc_mem_obj_create) (size_t nbytes);

/* ============================================================ */
/* mmap */
/* ============================================================ */

typedef void *(*TYPE_nacl_mmap) (void *start,
                                  size_t length,
                                  int prot,
                                  int flags,
                                  int desc,
                                  off_t offset);

typedef int (*TYPE_nacl_munmap) (void *start, size_t length);

/* ============================================================ */
/* threads */
/* ============================================================ */

typedef void (*TYPE_nacl_thread_exit) (int32_t *stack_flag);
typedef int (*TYPE_nacl_thread_create) (void *start_user_address,
                                        void *stack,
                                        void *tdb,
                                        size_t tdb_size);
typedef int (*TYPE_nacl_thread_nice) (const int nice);

/* ============================================================ */
/* mutex */
/* ============================================================ */

typedef int (*TYPE_nacl_mutex_create) ();
typedef int (*TYPE_nacl_mutex_lock) (int mutex);
typedef int (*TYPE_nacl_mutex_unlock) (int mutex);
typedef int (*TYPE_nacl_mutex_trylock) (int mutex);

/* ============================================================ */
/* condvar */
/* ============================================================ */

typedef int (*TYPE_nacl_cond_create) ();
typedef int (*TYPE_nacl_cond_wait) (int cv, int mutex);
typedef int (*TYPE_nacl_cond_signal) (int cv);
typedef int (*TYPE_nacl_cond_broadcast) (int cv);
typedef int (*TYPE_nacl_cond_timed_wait_abs) (int condvar,
                                              int mutex,
                                              struct timespec *abstime);

/* ============================================================ */
/* semaphore */
/* ============================================================ */

typedef int (*TYPE_nacl_sem_create) (int32_t value);
typedef int (*TYPE_nacl_sem_wait) (int sem);
typedef int (*TYPE_nacl_sem_post) (int sem);

/* ============================================================ */
/* multimedia */
/* ============================================================ */

typedef int (*TYPE_nacl_multimedia_init)(int subsystems);
typedef int (*TYPE_nacl_multimedia_shutdown)();
typedef int (*TYPE_nacl_video_init)(int width, int height);
typedef int (*TYPE_nacl_video_shutdown)();
typedef int (*TYPE_nacl_video_update)(const void* data);
typedef int (*TYPE_nacl_video_poll_event)(union NaClMultimediaEvent *event);
/* NOTE: we cannot forward declare enums in C */
typedef int (*TYPE_nacl_audio_init)(/*enum NaClAudioFormat*/ int format,
                                    int desired_samples,
                                    int *obtained_samples);
typedef int (*TYPE_nacl_audio_shutdown)();
typedef int (*TYPE_nacl_audio_stream)(const void *data, size_t *size);

/* ============================================================ */
/* misc */
/* ============================================================ */

typedef int (*TYPE_nacl_getdents) (int desc, void *dirp, size_t count);

typedef int (*TYPE_nacl_gettimeofday) (struct timeval *tv, void *tz);

typedef int (*TYPE_nacl_sched_yield) ();

typedef int (*TYPE_nacl_sysconf) (int name, int *res);

typedef void *(*TYPE_nacl_sysbrk) (void *p);

typedef pid_t (*TYPE_nacl_getpid) (void);

typedef clock_t (*TYPE_nacl_clock) (void);

typedef int (*TYPE_nacl_nanosleep) (const struct timespec *req,
                                    struct timespec *rem);

#ifdef __GNUC__
typedef void (*TYPE_nacl_exit) (int status) __attribute__((noreturn));
#else
typedef void (*TYPE_nacl_exit) (int status);
#endif

typedef void (*TYPE_nacl_null) (void);

typedef int (*TYPE_nacl_tls_init) (void *tdb, int size);

typedef void *(*TYPE_nacl_tls_get) ();

typedef int (*TYPE_nacl_srpc_get_fd) (void);

typedef int (*TYPE_nacl_dyncode_copy) (void *dest, const void *src,
                                       size_t size);

#if __cplusplus
}
#endif

#endif  /*  NATIVE_CLIENT_SRC_UNTRUSTED_SYSCALL_BINDINGS_TRAMPOLINE_H */
