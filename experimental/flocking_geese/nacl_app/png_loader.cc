// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nacl_app/png_loader.h"

using url_io::WebResourceLoader;

namespace {
static const size_t kPngSignatureLength = 8;

const uint32_t kRedBlueMask = 0x00FF00FF;
const uint32_t kGreenMask = 0x0000FF00;
const uint32_t kAlphaMask = 0xFF000000;
const uint32_t kPixelOne = 0xFF;
const uint32_t kAlphaShift = 24;
}  // namespace

namespace flocking_geese {

// Wrapper function handed to libpng that reads the PNG data.
void ReadDataFromInputStream(png_structp png_ptr,
                             png_bytep out_bytes,
                             png_size_t byte_count) {
  if(png_ptr->io_ptr == NULL)
    return;
  flocking_geese::PngLoader* png_loader =
      static_cast<flocking_geese::PngLoader*>(png_ptr->io_ptr);
  const size_t bytes_read = png_loader->ReadPngData(
      static_cast<uint8_t*>(out_bytes),
      static_cast<size_t>(byte_count));

  if (static_cast<png_size_t>(bytes_read) != byte_count)
    return;
}

PngLoader::~PngLoader() {
  ReleaseAndInvalidate();
}

void PngLoader::ReleaseAndInvalidate() {
  is_valid_ = false;
  if (png_ptr_ && png_info_ptr_) {
    png_destroy_read_struct(&png_ptr_, &png_info_ptr_, NULL);
  } else if (png_ptr_) {
    png_destroy_read_struct(&png_ptr_, NULL, NULL);
  }
  png_ptr_ = NULL;
  png_info_ptr_ = NULL;
  png_image_size_.SetSize(0, 0);
}

void PngLoader::OnLoaderReceivedResponseInfo(WebResourceLoader* loader) {
  ReleaseAndInvalidate();  // Start with clean data.
  // The content length should be a value greater than 0 or -1 for a
  // continuous stream.
  content_length_ = loader->GetContentLength();
  assert(content_length_ == -1 or content_length_ > 0);
  if (content_length_ == 0 || content_length_ < -1) {
    return;
  }
  // Allocate the internal buffer that will hold all the PNG data.  Start
  // reading data into the beginning of this buffer.
  png_data_.reset(new uint8_t[content_length_]);
  url_bytes_read_ = 0;
  loader->set_content_buffer(png_data_.get(), content_length_);
  loader->ReadMoreData();
}

void PngLoader::OnLoaderReceivedData(WebResourceLoader* loader) {
  // Advance the internal bufer pointer to the end of the current data and
  // issue another read.
  url_bytes_read_ += loader->data_size();
  if (url_bytes_read_ < content_length_) {
    uint8_t* buffer_ptr = &(png_data_[url_bytes_read_]);
    loader->set_content_buffer(buffer_ptr, content_length_);
  } else {
    // This will cause the loader to switch to an internal buffer and protect
    // |png_data_| from possibn=le overruns.
    loader->set_content_buffer(NULL, 0);
  }
  loader->ReadMoreData();
}

void PngLoader::OnLoaderCompletedDownload(WebResourceLoader* loader) {
  url_bytes_read_ = 0;
  is_valid_ = true;
  png_data_pos_ = 0;
  // Validate the data and initialize the PNG structs.
  uint8_t png_signature[kPngSignatureLength];
  if (ReadPngData(png_signature, kPngSignatureLength) != kPngSignatureLength) {
    ReleaseAndInvalidate();
    return;
  }
  if (!png_check_sig(png_signature, kPngSignatureLength)) {
    ReleaseAndInvalidate();
    return;
  }
  png_ptr_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr_ == NULL) {
    ReleaseAndInvalidate();
    return;
  }
  png_info_ptr_ = png_create_info_struct(png_ptr_);
  if (png_info_ptr_ == NULL) {
    ReleaseAndInvalidate();
    return;
  }
  png_set_read_fn(png_ptr_, this, ReadDataFromInputStream);
  // Tell libpng that the signature bytes have been read.
  png_set_sig_bytes(png_ptr_, kPngSignatureLength);

  // Try to read and validate the header info.
  png_read_info(png_ptr_, png_info_ptr_);
  png_uint_32 width = 0;
  png_uint_32 height = 0;
  int bits_per_sample;
  int pixel_format;
  png_uint_32 png_error = png_get_IHDR(
      png_ptr_,
      png_info_ptr_,
      &width,
      &height,
      &bits_per_sample,
      &pixel_format,
      NULL, NULL, NULL);
  if (png_error != 1) {
    ReleaseAndInvalidate();
    return;
  }
  png_image_size_.SetSize(width, height);
  // Tell libpng to strip 16 bit/color files down to 8 bits/color.
  png_set_strip_16(png_ptr_);
  switch (pixel_format) {
  case PNG_COLOR_TYPE_PALETTE:
    // Expand paletted colors into true RGB triplets.
    png_set_palette_to_rgb(png_ptr_);
    break;
  case PNG_COLOR_TYPE_GRAY:
    // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel.
    if (bits_per_sample < 8)
      png_set_expand_gray_1_2_4_to_8(png_ptr_);
    break;
  case PNG_COLOR_TYPE_RGB:
    png_set_expand(png_ptr_);
    break;
  }
  // Expand paletted or RGB images with transparency to full alpha channels
  // so the data will be available as RGBA quartets.
  if (png_get_valid(png_ptr_, png_info_ptr_, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr_);
  // Change RGBA -> ARGB.
  png_set_swap_alpha(png_ptr_);
}

void PngLoader::OnLoaderError(int32_t error, WebResourceLoader* loader) {
  ReleaseAndInvalidate();
}

void PngLoader::OnLoaderDone(WebResourceLoader* loader) {
  loader->CloseAndDeleteSelf();
}

void PngLoader::FillPixelBuffer(uint32_t* pixel_buffer) {
  if (!is_valid())
    return;
  png_bytep row_pointers[png_image_size_.height()];
  size_t row_bytes = png_get_rowbytes(png_ptr_, png_info_ptr_);
  for (int32_t row = 0; row < png_image_size_.height(); ++row) {
    row_pointers[row] = static_cast<png_bytep>(png_malloc(png_ptr_, row_bytes));
  }
  png_read_image(png_ptr_, row_pointers);
  png_read_end(png_ptr_, png_info_ptr_);
  // Copy the PNG data into the given buffer.
  for (int32_t row = 0; row < png_image_size_.height(); ++row) {
    memcpy(pixel_buffer, row_pointers[row], row_bytes);
    PreMultiplyAlpha(pixel_buffer, png_image_size_.width());
    pixel_buffer += png_image_size_.width();
    png_free(png_ptr_, row_pointers[row]);
  }
}

void PngLoader::PreMultiplyAlpha(uint32_t* scanline, int32_t line_width) {
  for (int32_t x = 0; x < line_width; ++x) {
    uint32_t src = *scanline;
    uint32_t rb = src & kRedBlueMask;
    uint32_t g = src & kGreenMask;
    uint32_t alpha = (src >> kAlphaShift) & kPixelOne;
    rb *= alpha;
    g *= alpha;
    rb = (rb >> 8) & kRedBlueMask;
    g = (g >> 8) & kGreenMask;
    *scanline++ = (rb | g | (alpha << kAlphaShift));
  }
}

size_t PngLoader::ReadPngData(uint8_t* buffer, const size_t byte_count) {
  size_t copy_byte_count = byte_count;
  if (png_data_pos_ + byte_count >= static_cast<size_t>(content_length_))
    copy_byte_count = content_length_ - png_data_pos_;  // "end-of-file".
  memcpy(buffer, &(png_data_[png_data_pos_]), copy_byte_count);
  png_data_pos_ += copy_byte_count;
  return copy_byte_count;
}

}  // namespace flocking_geese

