// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains some typedefs for basic types


#ifndef _COMMON_TYPES_H__
#define _COMMON_TYPES_H__

#ifdef _WIN32
#pragma warning(disable:4100)

typedef signed char         int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef signed char         int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef long long           int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#define __PTRDIFF_TYPE__ long long
#ifdef __PTRDIFF_TYPE__
typedef          __PTRDIFF_TYPE__ intptr;
typedef unsigned __PTRDIFF_TYPE__ uintptr;
#else
#error "Can't find pointer-sized integral types."
#endif

#else
#include <stdint.h>
#endif

#endif  // _COMMON_TYPES_H__
