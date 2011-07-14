// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STAMP_H_
#define STAMP_H_

#include <string>
#include <vector>
#include "ppapi/cpp/point.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"

namespace life {
// A stamp object.  This holds a bitmap for an initial-value stamp.  Applying
// the stamp puts values in the cell bitmap and a pixel buffer.  Stamps are
// initialized using the .LIF 1.05 format (see:
// http://psoup.math.wisc.edu/mcell/ca_files_formats.html). Stamps are always
// rectangular, and have minimum dimensions of (3 x 3).
class Stamp {
 public:
  // Create the default stamp: size of minimum dimensions and all cells alive.
  Stamp();
  virtual ~Stamp();

  // Create a stamp using the supplied stamp description. |stamp_desription| is
  // expressed as a string where each character represents a cell: '*' is a
  // live cell and '.' is a dead one.  A new-line represents the end of arow of
  // cells.  See the .LIF 1.05 format for more details:
  //   http://psoup.math.wisc.edu/mcell/ca_files_formats.html
  // Returns success.
  bool InitFromDescription(const std::string& stamp_description);

  // Apply the stamp to the color and cell buffers.  The stamp is cropped to
  // the buffer rectangles.  Both the pixel and cell buffer must have the same
  // dimensions, and are assumed to have matching coordinate systems (that is,
  // (x, y) in the pixel buffer corresponds to the same location in the cell
  // buffer).
  void StampAtPointInBuffers(const pp::Point& point,
                             uint32_t* dest_pixel_buffer,
                             uint8_t* dest_cell_buffer,
                             const pp::Size& buffer_size) const;

  pp::Size size() const {
    return size_;
  }

  // The stamp offset represents the origin of the stamp in the stamp's
  // coordinate system, where (0, 0) is the upper-left corner.
  pp::Point stamp_offset() const {
    return stamp_offset_;
  }
  void set_stamp_offset(const pp::Point& offset) {
    stamp_offset_ = offset;
  }

 private:
  pp::Size size_;
  pp::Point stamp_offset_;
  std::vector<uint32_t> pixel_buffer_;
  std::vector<uint8_t> cell_buffer_;
};

}  // namespace life

#endif  // LIFE_H_

