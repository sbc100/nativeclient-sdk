// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LOCKING_IMAGE_DATA_H_
#define LOCKING_IMAGE_DATA_H_

#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/size.h"
#include "threading/pthread_ext.h"

namespace flocking_geese {

// An extension of ImageData that can protect direct pixel access thorugh a
// mutex lock.
class LockingImageData : public pp::ImageData {
 public:
  LockingImageData(pp::Instance* instance,
                   PP_ImageDataFormat format,
                   const pp::Size& size,
                   bool init_to_zero)
      : ImageData(instance, format, size, init_to_zero) {
    pthread_mutex_init(&pixel_buffer_mutex_, NULL);
  }

  virtual ~LockingImageData() {
    UnlockPixels();
    pthread_mutex_destroy(&pixel_buffer_mutex_);
  }

  // Acquire the lock in |pixel_owner_| that governs the pixel buffer.  Return
  // a pointer to the locked pixel buffer if successful; return NULL otherwise.
  uint32_t* LockPixels() {
    uint32_t* pixels = NULL;
    if (pthread_mutex_lock(&pixel_buffer_mutex_) == PTHREAD_MUTEX_SUCCESS) {
      pixels = PixelBufferNoLock();
    }
    return pixels;
  }

  // Release the lock governing the pixel bugger in |pixel_owner_|.
  void UnlockPixels() {
    pthread_mutex_unlock(&pixel_buffer_mutex_);
  }

  uint32_t* PixelBufferNoLock() {
    return is_null() ? NULL : static_cast<uint32_t*>(data());
  }

 private:
  mutable pthread_mutex_t pixel_buffer_mutex_;
};
}  // namespace flocking_geese

#endif  // LOCKING_IMAGE_DATA_H_

