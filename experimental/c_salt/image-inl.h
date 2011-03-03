// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_IMAGE_INL_H_
#define C_SALT_IMAGE_INL_H_

#include <stdint.h>

namespace c_salt {
#define INLINE_NO_INSTRUMENT \
    __attribute__((no_instrument_function, always_inline))

// build a packed color
INLINE_NO_INSTRUMENT
    uint32_t MakeARGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
inline uint32_t MakeARGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (((a) << 24) | ((r) << 16) | ((g) << 8) | (b));
}

// extract R, G, B, A from packed color
INLINE_NO_INSTRUMENT int ExtractR(uint32_t c);
inline int ExtractR(uint32_t c) {
  return (c >> 16) & 0xFF;
}

INLINE_NO_INSTRUMENT int ExtractG(uint32_t c);
inline int ExtractG(uint32_t c) {
  return (c >> 8) & 0xFF;
}

INLINE_NO_INSTRUMENT int ExtractB(uint32_t c);
inline int ExtractB(uint32_t c) {
  return c & 0xFF;
}

INLINE_NO_INSTRUMENT int ExtractA(uint32_t c);
inline int ExtractA(uint32_t c) {
  return (c >> 24) & 0xFF;
}
#undef INLINE_NO_INSTRUMENT
}  // namespace c_salt

#endif  // C_SALT_IMAGE_INL_H_
