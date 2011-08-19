// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SCOPED_PIXEL_LOCK_H_
#define SCOPED_PIXEL_LOCK_H_

#include <tr1/memory>
#include "experimental/conways_life/locking_image_data.h"

namespace life {

// A small helper RAII class used to acquire and release the pixel lock based
// on C++ scoping rules.
class ScopedPixelLock {
 public:
  explicit ScopedPixelLock(
      const std::tr1::shared_ptr<LockingImageData>& pixel_owner)
      : pixel_owner_(pixel_owner) {
    pixels_ = pixel_owner_ == NULL ? NULL : pixel_owner->LockPixels();
  }

  ~ScopedPixelLock() {
    pixels_ = NULL;
    if (pixel_owner_ != NULL)
      pixel_owner_->UnlockPixels();
  }

  bool is_valid() const {
    return pixels_ != NULL;
  }

  uint32_t* pixels() const {
    return pixels_;
  }

 private:
  std::tr1::shared_ptr<LockingImageData> pixel_owner_;
  uint32_t* pixels_;  // Weak reference.
};
}  // namespace life

#endif  // SCOPED_PIXEL_LOCK_H_

