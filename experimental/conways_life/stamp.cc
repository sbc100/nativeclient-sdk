// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "experimental/conways_life/stamp.h"

#include <algorithm>
#include <cstring>

namespace {
const int32_t kMinimumStampDimension = 3;
const char kStampLineSeparator = '\n';
const char kStampAliveCharacter = '*';
const char kStampDeadCharacter = '.';
const uint32_t kWhiteColor = 0xFFFFFFFF;
const uint32_t kBlackColor = 0xFF000000;

// Do a simple copy composite from src_buffer into dst_buffer.  Copy the pixels
// contained in |src_rect| to |dst_loc|.  This assumes that all the necessary
// cropping and clipping has been done, and that all the buffer pointers are
// valid.
template <class PixelT> void CopyCompositeToPoint(
    const PixelT* src_buffer,
    int src_row_width,
    const pp::Rect& src_rect,
    PixelT* dst_buffer,
    int dst_row_width,
    const pp::Point& dst_loc) {
  const PixelT* src = src_buffer + src_rect.x() + src_rect.y() * src_row_width;
  PixelT* dst = dst_buffer + dst_loc.x() + dst_loc.y() * dst_row_width;
  for (int y = 0; y < src_rect.height(); ++y) {
    memcpy(dst, src, src_rect.width() * sizeof(PixelT));
    src += src_row_width;
    dst += dst_row_width;
  }
}
}  // namespace

namespace life {
Stamp::Stamp()
    : size_(kMinimumStampDimension, kMinimumStampDimension),
      stamp_offset_(0, 0) {
  // Build the default stamp.
  int buffer_size = size_.GetArea();
  pixel_buffer_.resize(buffer_size);
  cell_buffer_.resize(buffer_size);
  std::fill(&pixel_buffer_[0],
            &pixel_buffer_[0] + buffer_size, kBlackColor);
  std::fill(&cell_buffer_[0],
            &cell_buffer_[0] + buffer_size, 1);
}

bool Stamp::InitFromDescription(const std::string& stamp_description) {
  // Compute the width and height of the stamp description.
  size_t eol_pos = stamp_description.find(kStampLineSeparator);
  pp::Size new_size;
  if (eol_pos == std::string::npos) {
    new_size.set_width(stamp_description.size());
    new_size.set_height(1);
  } else {
    new_size.set_width(eol_pos);
    // Count up the number of lines.
    int count = 0;
    do {
      ++count;
      eol_pos = stamp_description.find(kStampLineSeparator, eol_pos + 1);
    } while (eol_pos != std::string::npos);
    new_size.set_height(count);
  }
  int buffer_size = new_size.GetArea();
  if (buffer_size <= 0)
    return false;
  size_ = new_size;
  pixel_buffer_.resize(buffer_size);
  cell_buffer_.resize(buffer_size);
  int buffer_index = 0;
  for (size_t i = 0; i < stamp_description.size(); ++i) {
    switch (stamp_description[i]) {
    case kStampAliveCharacter:
      pixel_buffer_[buffer_index] = kBlackColor;
      cell_buffer_[buffer_index] = 1;
      ++buffer_index;
      break;
    case kStampDeadCharacter:
      pixel_buffer_[buffer_index] = kWhiteColor;
      cell_buffer_[buffer_index] = 0;
      ++buffer_index;
      break;
    case kStampLineSeparator:
      // Ignore these.
      break;
    default:
      // Invalid character - error?
      ++buffer_index;
      break;
    }
  }
  return true;
}

Stamp::~Stamp() {
}

void Stamp::StampAtPointInBuffers(const pp::Point& point,
                                  uint32_t* dest_pixel_buffer,
                                  uint8_t* dest_cell_buffer,
                                  const pp::Size& buffer_size) const {
  pp::Rect src_rect_clipped(pp::Point(), size());
  if (src_rect_clipped.IsEmpty())
    return;

  pp::Point dest_point(point);
  dest_point -= stamp_offset();
  // Create a clipped rect in the destination coordinate space that contains the
  // final image.
  pp::Rect dest_bounds(pp::Point(), buffer_size);
  pp::Rect draw_rect(dest_point, src_rect_clipped.size());
  pp::Rect draw_rect_clipped(dest_bounds.Intersect(draw_rect));
  if (draw_rect_clipped.IsEmpty())
    return;
  // Transform the destination rectangle back to the source image coordinate
  // system: Translate(-dest_point) . Translate(src_rect_clipped.origin).
  pp::Point src_offset(draw_rect_clipped.point());
  src_offset -= dest_point;
  src_rect_clipped.Offset(src_offset);
  src_rect_clipped.set_size(draw_rect_clipped.size());
  if (dest_pixel_buffer) {
    CopyCompositeToPoint(&pixel_buffer_[0],
                         size().width(),
                         src_rect_clipped,
                         dest_pixel_buffer,
                         buffer_size.width(),
                         dest_point);
  }
  if (dest_cell_buffer) {
    CopyCompositeToPoint(&cell_buffer_[0],
                         size().width(),
                         src_rect_clipped,
                         dest_cell_buffer,
                         buffer_size.width(),
                         dest_point);
  }
}
}  // namespace life
