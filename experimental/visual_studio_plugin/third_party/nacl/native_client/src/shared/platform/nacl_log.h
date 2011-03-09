/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Server Runtime logging code.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_NACL_LOG_H__
#define NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_NACL_LOG_H__

#include <stdarg.h>

/*
 * We cannot include this header file from an installation that does not
 * have the native_client source tree.
 * TODO(sehr): use export_header.py to copy these files out.
 */
#include "native_client/src/include/nacl_base.h"
#include "native_client/src/shared/gio/gio.h"

EXTERN_C_BEGIN

enum NaClLogOptions {
  NACL_LOG_OPTIONS_NONE,
  NACL_LOG_OPTIONS_DEFAULT_FROM_ENVIRONMENT
};

/*
 * TODO: per-module logging, adding a module-name parameter, probably
 * an atom that is statically allocated.
 */
void NaClLogModuleInit(void);

void NaClLogModuleInitExtended(enum NaClLogOptions);

/*
 * Sets the log file to the named file.  Aborts program if the open
 * fails.
 *
 * The GioFile object is dynamically allocated, so caller is responsible
 * for obtaining it via NaClLogGetGio and freeing it as appropriate.
 */
void NaClLogSetFile(char const *log_file);

void NaClLogModuleFini(void);

void NaClLogSetVerbosity(int verb);

int NaClLogGetVerbosity(void);

void NaClLogIncrVerbosity(void);

void NaClLogSetGio(struct Gio *out_stream);

struct Gio *NaClLogGetGio(void);

/*
 * Timestamps on log entries may be disabled, e.g., to make it easier to
 * write test that compare logging output.
 */

void  NaClLogEnableTimestamp(void);

void  NaClLogDisableTimestamp(void);

/*
 * Users of NaClLogV should add ATTRIBUTE_FORMAT_PRINTF(m,n) to their
 * function prototype, where m is the argument position of the format
 * string and n is the position of the first argument to be consumed
 * by the format specifier.
 */
void NaClLogV(int         detail_level,
              char const  *fmt,
              va_list     ap);

void  NaClLog(int         detail_level,
              char const  *fmt,
              ...) ATTRIBUTE_FORMAT_PRINTF(2, 3);

#define LOG_INFO    (-1)
#define LOG_WARNING (-2)
#define LOG_ERROR   (-3)
#define LOG_FATAL   (-4)

/*
 * Low-level logging code that requires manual lock manipulation.
 * Should be used carefully!
 */
void NaClLogLock(void);
void NaClLogUnlock(void);

/*
 * Caller is responsible for grabbing the NaClLog mutex lock (via
 * NaClLogLock) before calling NaClLogV_mu or NaClLog_mu and for
 * releasing it (via NaClLogUnlock) afterward.
 *
 * Users of NaClLogV_mu should also use ATTRIBUTE_FORMAT_PRINTF as
 * above.
 *
 * Only the first call to NaClLog_mu or NaClLogV_mu after the
 * NaClLogLock will get a timestamp tag prepended.  No newline
 * detection is done, so a multi-line output must be (1) split into
 * multiple calls to NaClLog_mu or NaClLogV_mu so that newlines are
 * the last character in the format string to the NaClLog_mu or
 * NaClLogV_mu calls, and (2) are followed by NaClLogTagNext_mu() so
 * that the next output will know to generate a tag.
 */
void NaClLogV_mu(int         detail_level,
                 char const  *fmt,
                 va_list     ap);

void NaClLog_mu(int         detail_level,
                char const  *fmt,
                ...) ATTRIBUTE_FORMAT_PRINTF(2, 3);

void NaClLogTagNext_mu(void);


EXTERN_C_END

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_NACL_LOG_H__ */
