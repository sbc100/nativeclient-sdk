/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "native_client/src/include/portability_string.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

int debug_get_tokens(const char *in, char delim, char *out[], int max) {
  char *str   =STRDUP(in);
  char *start= str;
  char *word = str;
  int  cnt = 0;

  for (;*str; str++) {
    if (*str == delim) {

      // Make this null, so we can copy it
      *str = 0;

      // Add it to the array;
      if (cnt < max)
        out[cnt++] = STRDUP(word);

      // Start scanning after the delim
      str++;
      word = str;
    }
  }

  if (*word)
    if (cnt < max)
      out[cnt++] = STRDUP(word);

  free(start);
  return cnt;
}

void debug_free_tokens(char *strings[], int max) {
  int cnt = 0;
  for (cnt = 0; cnt < max; cnt++) {
    if (strings[cnt]) {
      free(strings[cnt]);
      strings[cnt] = 0;
    }
  }
}

int debug_nibble_to_int(char ch) {
  if ((ch >= 'a') && (ch <= 'f'))
    return (ch - 'a' + 10);
  if ((ch >= '0') && (ch <= '9'))
    return (ch - '0');
  if ((ch >= 'A') && (ch <= 'F'))
    return (ch - 'A' + 10);

  return (-1);
}

char debug_int_to_nibble(int nibble) {
  nibble &= 0xF;

  if (nibble < 10)
    return '0' + nibble;

  return 'a' + (nibble - 10);
}

enum {
  DPL_INFO = 0,
  DPL_WARN = 1,
  DPL_ERROR= 2,
  DPL_COUNT
};

static const char *s_ErrStrs[DPL_COUNT] = {
  "INFO", "WARN", "!ERR"
};

void debug_printf(int level, const char *format, va_list args) {
  char buffer[4096];
  vsnprintf(buffer, sizeof(buffer), format, args);
  printf("[%s] %s", s_ErrStrs[level], buffer);
}


void debug_log_info(const char *format, ...) {
  va_list args;
  va_start( args, format );
  
  debug_printf(DPL_INFO, format, args);
}

void debug_log_warning(const char *format, ...) {
  va_list args;
  va_start( args, format );
  
  debug_printf(DPL_WARN, format, args);
}

void debug_log_error(const char *format, ...) {
  va_list args;
  va_start( args, format );
  
  debug_printf(DPL_ERROR, format, args);
}

