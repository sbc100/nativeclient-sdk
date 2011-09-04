// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nacl_app/sprite.h"

#include <algorithm>

namespace {
const uint32_t kRedBlueMask = 0x00FF00FF;
const uint32_t kGreenMask = 0x0000FF00;
const uint32_t kAlphaMask = 0xFF000000;
const uint32_t k8BitMultiplyMask = 0xFF00FF00;
const uint32_t kPixelOne = 0xFF;
const uint32_t kAlphaShift = 24;
}  // namespace

namespace flocking_geese {

Sprite::Sprite(uint32_t* pixel_buffer,
               const pp::Size& size,
               int32_t row_bytes) {
  SetPixelBuffer(pixel_buffer, size, row_bytes);
}

void Sprite::SetPixelBuffer(uint32_t* pixel_buffer,
                            const pp::Size& size,
                            int32_t row_bytes) {
  pixel_buffer_.reset(pixel_buffer);
  pixel_buffer_size_ = size;
  row_bytes_ = row_bytes ? row_bytes : size.width() * sizeof(uint32_t);
}

void Sprite::CompositeFromRectToPoint(const pp::Rect& src_rect,
                                      uint32_t* dest_pixel_buffer,
                                      const pp::Size& dest_size,
                                      int32_t dest_row_bytes,
                                      const pp::Point& dest_point) const {
  // Clip the source rect to the source image bounds.
  pp::Rect src_bounds(pp::Point(), size());
  pp::Rect src_rect_clipped(src_rect.Intersect(src_bounds));
  if (src_rect_clipped.IsEmpty())
    return;

  // Validate the destination point: it has to be inside the destination
  // image bounds.
  if (dest_point.x() < 0 || dest_point.x() > dest_size.width() ||
      dest_point.y() < 0 || dest_point.y() > dest_size.height())
    return;

  // Clip the source rectangle's size to the destination size.
  pp::Size dest_size_clipped(dest_size.width() - dest_point.x(),
                             dest_size.height() - dest_point.y());
  src_rect_clipped.set_width(std::min(src_rect_clipped.width(),
                                      dest_size_clipped.width()));
  src_rect_clipped.set_height(std::min(src_rect_clipped.height(),
                                       dest_size_clipped.height()));
  if (src_rect_clipped.IsEmpty())
    return;

  size_t src_byte_offset = src_rect_clipped.x() * sizeof(uint32_t) +
                           src_rect_clipped.y() * row_bytes_;
  const uint8_t* src_pixels =
      reinterpret_cast<const uint8_t*>(pixel_buffer_.get()) + src_byte_offset;

  if (dest_row_bytes == 0)
    dest_row_bytes = dest_size.width() * sizeof(uint32_t);
  size_t dest_byte_offset = dest_point.x() * sizeof(uint32_t) +
                            dest_point.y() * dest_row_bytes;
  uint8_t* dest_pixels = reinterpret_cast<uint8_t*>(dest_pixel_buffer) +
                         dest_byte_offset;

  // All the pointers are set up, now do the SOver.  Note that the only pre-
  // multiplied alpha formats are supported.
  for (int32_t y = 0; y < src_rect_clipped.height(); ++y) {
    const uint32_t* src_scanline =
        reinterpret_cast<const uint32_t*>(src_pixels);
    uint32_t* dest_scanline = reinterpret_cast<uint32_t*>(dest_pixels);
    for (int32_t x = 0; x < src_rect_clipped.width(); ++x) {
      uint32_t src = *src_scanline++;
      uint32_t dst = *dest_scanline;
      uint32_t alpha = (src >> kAlphaShift) & kPixelOne;
      uint32_t one_minus_alpha = kPixelOne - alpha;
      // Compute RB and G separately: this allows for SIMD-like behaviour
      // when multiplying the channels by alpha.  Note that over-saturated
      // pixels will wrap to 0 and not clamp.
      uint32_t src_rb = src & kRedBlueMask;
      uint32_t src_g = src & kGreenMask;
      uint32_t dst_rb = (((dst & kRedBlueMask) * one_minus_alpha) >> 8) &
                        kRedBlueMask;
      uint32_t dst_g = (((dst & kGreenMask) * one_minus_alpha) >> 8) &
                       kGreenMask;
      uint32_t rb = src_rb | dst_rb;
      uint32_t g = src_g | dst_g;

      *dest_scanline++ = (dst & kAlphaMask) | (rb | g);
    }
    src_pixels += row_bytes_;
    dest_pixels += dest_row_bytes;
  }
}

}  // namespace flocking_geese

