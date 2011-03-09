/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * portability macros for file descriptors, read, write, open, etc.
 */

#ifndef NATIVE_CLIENT_SRC_INCLUDE_PORTABILITY_IO_H_
#define NATIVE_CLIENT_SRC_INCLUDE_PORTABILITY_IO_H_ 1

#if NACL_WINDOWS
/* disable warnings for deprecated _snprintf */
#pragma warning(disable : 4996)

#include "io.h"
#include "fcntl.h"

#define DUP2 _dup2
#define OPEN _open
#define FDOPEN _fdopen
#define PORTABLE_DEV_NULL "nul"
#define SNPRINTF _snprintf

/* Seek method constants */
#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

/* missing from win stdio.h and fcntl.h */

/* from bits/fcntl.h */
#define O_ACCMODE 0003

#else

#include <stdlib.h>
#include <fcntl.h>

#define OPEN open
#define DUP2 dup2
#define FDOPEN fdopen
#define PORTABLE_DEV_NULL  "/dev/null"
#define SNPRINTF snprintf
#define _O_BINARY 0
#endif

#endif  /* NATIVE_CLIENT_SRC_INCLUDE_PORTABILITY_IO_H_ */
