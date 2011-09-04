// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PNG_LOADER_H_
#define PNG_LOADER_H_

#include <png.h>
#include <string.h>

#include "boost/scoped_array.hpp"
#include "ppapi/cpp/size.h"
#include "threading/thread_condition.h"
#include "url_io/web_resource_loader.h"

namespace flocking_geese {

// Class that knows how to load and unpack a .png file into a buffer.  The
// caller is expected to provide the memory for the final decompressed ARGB
// pixel buffer.
class PngLoader : public url_io::WebResourceLoader::Delegate {
 public:
  PngLoader()
      : content_length_(0),
        url_bytes_read_(0),
        is_valid_(false),
        png_data_pos_(0),
        png_ptr_(NULL),
        png_info_ptr_(NULL) {}
  ~PngLoader();

  // WebResourceLoader::Delegate interface.
  virtual void OnLoaderReceivedResponseInfo(url_io::WebResourceLoader* loader);
  virtual void OnLoaderReceivedData(url_io::WebResourceLoader* loader);
  virtual void OnLoaderCompletedDownload(url_io::WebResourceLoader* loader);
  virtual void OnLoaderError(int32_t error, url_io::WebResourceLoader* loader);
  virtual void OnLoaderDone(url_io::WebResourceLoader* loader);

  // Fill in the given buffer with the decompressed image.  |pixel_buffer| is
  // assumed to point to enough memory to hold all the pixel data.  If there
  // isn't enough memory, there will be unpredictable results.
  void FillPixelBuffer(uint32_t* pixel_buffer);

  // Return |true| if the PNG data was sucessfully downloaded.
  bool is_valid() const {
    return is_valid_;
  }

  const uint8_t* png_data() const {
    return png_data_.get();
  }

  // The size in pixels of the final, decompressed pixel buffer.
  const pp::Size& png_image_size() const {
    return png_image_size_;
  }

 private:
  // Support routines for using libpng.

  // Wrapper function given to libpng.
  friend void ReadDataFromInputStream(png_structp png_ptr,
                                      png_bytep out_bytes,
                                      png_size_t byte_count);

  // Copy the next |byte_count| bytes from the PNG buffer into the supplied
  // buffer.  This advances the internal stream pointer by |byte_count|.
  // Returns the actual number of bytes read.  This method is passed into the
  // libpng API.
  size_t ReadPngData(uint8_t* buffer, const size_t byte_count);

  // Premultiply all pixels in a scanline by their alpha.
  void PreMultiplyAlpha(uint32_t* scanline, int32_t line_width) ;

  // Delete all internal PNG sturctures and invlaidate this object.
  void ReleaseAndInvalidate();

  int32_t content_length_;  // The total content length.
  int32_t url_bytes_read_;  // The number of bytes read so far.
  bool is_valid_;
  boost::scoped_array<uint8_t> png_data_;
  size_t png_data_pos_;
  png_structp png_ptr_;  // The main PNG structure maintained by libpng.
  png_infop png_info_ptr_;
  pp::Size png_image_size_;
};

}  // namespace flocking_geese

#endif  // PNG_LOADER_H_
