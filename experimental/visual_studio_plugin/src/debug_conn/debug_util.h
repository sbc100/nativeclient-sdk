/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_UTIL_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_UTIL_H_ 1

#include "native_client/src/include/portability.h"

EXTERN_C_BEGIN


// Free or Parse string and dup into provided array
int debug_get_tokens(const char *in, char delim, char *out[], int max);
void debug_free_tokens(char *strings[], int max);

// Convert between 4bit value and ASCII 0-F hex.
int debug_nibble_to_int(char ch);
char debug_int_to_nibble(int nibble);

// Debug Logging
void debug_log_info(const char *fmt, ...);
void debug_log_warning(const char *fmt, ...);
void debug_log_error(const char *fmt, ...);

EXTERN_C_END

#endif
