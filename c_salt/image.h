// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_IMAGE_H_
#define C_SALT_IMAGE_H_

#include <stdint.h>
#include <boost/scoped_array.hpp>

namespace c_salt {
// Image class is a container for ABGR-8888 pixels. Image files with other
// formats are converted when reading
class Image {
 public:
  // Ctor creates an empty, invalid surface.  Use Resize()
  // to fully initialize the Image.
  Image();
  // Create deep copy of |other|.
  Image(const Image& other);

  virtual ~Image();

  // Create deep copy of |other|.
  Image& operator=(const Image& other);

  // Change the size of the surface to (width, height).  The minimum dimension
  // is clamped to (1, 1).
  void Resize(int width, int height);

  // Set all image pixels to the current background color.
  void Erase();

  // Load pixels from raw data. Only JPEG is supported for now.
  bool InitWithData(const void* data, size_t data_length);

  int width() const {return width_;}
  int height() const {return height_;}

  // A Image is valid if it has a dimension of at least (1, 1) and there is
  // a pixel store allocated.
  bool is_valid() const {return (size_ > 0) && (pixels_.get() != NULL);}

  void set_background_color(uint32_t color) {background_color_ = color;}
  uint32_t background_color() const {return background_color_;}

  // Used to get the address of a pixel at location (|x|, |y|).
  // Does not check bounds.
  inline uint32_t* PixelAddress(int x, int y) {
    return &pixels_[x + y * width_];
  }

  inline const uint32_t* PixelAddress(int x, int y) const {
    return &pixels_[x + y * width_];
  }

  // Sets a pixel at a linear offset in memory.  Does not check bounds or if
  // the Image is valid.
  void SetPixelAt(int i, uint32_t color) {pixels_[i] = color;}

  // Get the pixel value at a linear offset in memory.  Does not check bounds
  // or if the Image is valid.
  uint32_t GetPixelAt(int i) const {return pixels_[i];}

  // Set the pixel at 2D address (|x|, |y|) to |color| without any bounds
  // checking on the values of |x| and |y|.
  void SetPixelNoClip(int x, int y, uint32_t color) {
    SetPixelAt(y * width_ + x, color);
  }

  // Retrieve the pixel value at 2D address (|x|, |y|) without any bounds
  // checking on the values of |x| and |y|.
  uint32_t GetPixelNoClip(int x, int y) const {
    return GetPixelAt(y * width_ + x);
  }

  // Set the pixel at 2D address (|x|, |y|) to |color| with bounds checking
  // on the values of |x| and |y|.  If either |x| or |y| are out of bounds,
  // this method does nothing.
  void SetPixel(int x, int y, uint32_t color) {
    if (is_valid() && (x >= 0) && (x < width_) && (y >= 0) && (y < height_)) {
      SetPixelAt(y * width_ + x, color);
    }
  }

  // Retrieve the pixel value at 2D address (|x|, |y|) with bounds checking
  // on the values of |x| and |y|.  If either |x| or |y| are out of bounds,
  // return the background color.
  uint32_t GetPixel(int x, int y) const {
    if (!is_valid() || (x < 0) || (x >= width_) || (y < 0) || (y >= height_)) {
      return background_color_;
    }
    return GetPixelAt(y * width_ + x);
  }

 private:
  int width_;
  int height_;
  size_t size_;
  uint32_t background_color_;
  boost::scoped_array<uint32_t> pixels_;
};
}  // namespace c_salt

#endif  // C_SALT_IMAGE_H_

