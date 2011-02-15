// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_RECT_H_
#define C_SALT_RECT_H_

namespace c_salt {
class Size {
 public:
  Size() : width_(0), height_(0) {}
  Size(int w, int h) : width_(w), height_(h) {}

  int width() const {return width_;}
  int height() const {return height_;}

  int width_;
  int height_;
};

class Rect {
 public:
  Rect() : left_(0), top_(0), right_(0), bottom_(0) {}
  Rect(int left, int top, int right, int bottom);
  Rect(int width, int height);
  explicit Rect(Size size);

  int width() const {return right_ - left_;}
  int height() const {return bottom_ - top_;}

  bool Empty() const;
  void Deflate(int sz);
  void ShrinkToFit(const Rect& dest);
  void CenterIn(const Rect& dest);
  void MoveBy(int dx, int dy);

  int left_;
  int top_;
  int right_;
  int bottom_;
};
}  // namespace c_salt
#endif  // C_SALT_RECT_H_

