// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/rect.h"

#include <algorithm>

namespace c_salt {

Rect::Rect(int left, int top, int right, int bottom)
    : left_(left), top_(top), right_(right), bottom_(bottom) {
}

Rect::Rect(int width, int height)
    : left_(0), top_(0), right_(width), bottom_(height) {
}

Rect::Rect(Size size)
    : left_(0), top_(0), right_(size.width()), bottom_(size.height()) {
}

bool Rect::Empty() const {
  return (left_ == bottom_) && (top_ == bottom_);
}

void Rect::Deflate(int sz) {
  right_ = std::max(0, right_ - sz);
  left_ = std::min(right_, left_ + sz);
  bottom_ = std::max(0, bottom_ - sz);
  top_ = std::min(bottom_, top_ + sz);
}

void Rect::ShrinkToFit(const Rect& dest) {
  double x_scale = 1;
  double y_scale = 1;
  bool need_to_shrink = false;
  if (dest.width() == 0) {
    right_ = left_;
  } else if (dest.width() < width()) {
    x_scale = static_cast<double>(dest.width()) / width();
    need_to_shrink = true;
  }
  if (dest.height() == 0) {
    bottom_ = top_;
  } else if (dest.height() < height()) {
    y_scale = static_cast<double>(dest.height()) / height();
    need_to_shrink = true;
  }

  if (need_to_shrink) {
    double scale = std::min(x_scale, y_scale);
    right_ = left_ + (width() * scale);
    bottom_ = top_ + (height() * scale);
  }
}

void Rect::CenterIn(const Rect& dest) {
  int center_x = (left_ + right_) / 2;
  int center_y = (top_ + bottom_) / 2;
  int dest_center_x = (dest.left_ + dest.right_) / 2;
  int dest_center_y = (dest.top_ + dest.bottom_) / 2;
  MoveBy(dest_center_x - center_x, dest_center_y - center_y);
}

void Rect::MoveBy(int dx, int dy) {
  left_ += dx;
  right_ += dx;
  top_ += dy;
  bottom_ += dy;
}
}  // namespace c_salt

