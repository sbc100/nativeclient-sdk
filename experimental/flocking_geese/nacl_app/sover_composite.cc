// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nacl_app/sover_composite.h"

#include "ppapi/cpp/size.h"

static const uint32_t kRedBlueMask = 0x00FF00FF;
static const uint32_t kAlphaGreenMask = 0xFF00FF00;
static const uint32_t kPixelOne = 0xFF;
static const uint32_t kAlphaShift = 24;

// Composite one ImageData into another, performing clipping as needed.
// Assumes that both source and destination Images have the same pixel
// formats.  If the pixel formats differ, do nothing.  The only compositing
// function supported is source-over.  Only the pixels in |src_rect| are
// composited into the destination Image.  The source image is copied to
// |dst_point|.
void SOverComposite(const pp::ImageData& source,
                    const pp::Rect& src_rect,
                    pp::ImageData* destination,
                    const pp::Point& dst_point) {
  if (source.format() != destination->format())
    return;

  // Clip the source rect to the source image bounds.
  pp::Rect src_bounds(pp::Point(), source.size());
  pp::Rect src_rect_clipped(src_rect.Intersect(src_bounds));
  if (src_rect_clipped.IsEmpty())
    return;

  // Validate the destination point: it has to be inside the destination
  // image bounds.
  pp::Size dst_size(destination->size());
  if (dst_point.x() < 0 || dst_point.x() > dst_size.width() ||
      dst_point.y() < 0 || dst_point.y() > dst_size.height())
    return;

  // Clip the source rectangle to the destination bounds.
  pp::Rect dst_bounds(dst_point,
                      pp::Size(dst_size.width() - dst_point.x(),
                               dst_size.height() - dst_point.y()));
  src_rect_clipped = src_rect_clipped.Intersect(dst_bounds);
  if (src_rect_clipped.IsEmpty())
    return;

  const uint32_t* src_pixels = static_cast<const uint32_t*>(source.data()) +
                               src_rect_clipped.x() +
                               src_rect_clipped.y() * source.stride();
  uint32_t* dst_pixels = static_cast<uint32_t*>(destination->data()) +
                         dst_point.x() +
                         dst_point.y() * destination->stride();

  // All the pointers are set up, now do the SOver.  Note that the only pre-
  // multiplied alpha formats are supported.
  for (int32_t y = 0; y < src_rect_clipped.height(); ++y) {
    for (int32_t x = 0; x < src_rect_clipped.width(); ++x) {
      uint32_t src = *src_pixels++;
      uint32_t dst = *dst_pixels;
      uint32_t one_minus_alpha = kPixelOne - ((src >> kAlphaShift) & kPixelOne);
      // Compute RB and AG separately: this allows for SIMD-like behaviour
      // when multiplying the channels by alpha.  Note that over-saturated
      // pixels will wrap to 0 and not clamp.
      uint32_t src_rb = src & kRedBlueMask;
      // Shift the AG channels right by 8 to accomodate the 8-bit SIMD-like
      // multiply.
      uint32_t src_ag = (src >> 8) & kRedBlueMask;
      uint32_t dst_rb = (dst & kRedBlueMask) * one_minus_alpha;
      uint32_t dst_ag = ((dst >> 8) & kRedBlueMask) * one_minus_alpha;
      uint32_t rb = (src_rb + dst_rb) & kRedBlueMask;
      uint32_t ag = ((src_ag + dst_ag) << 8) & kAlphaGreenMask;

      *dst_pixels++ = rb | ag;
    }
  }
}
