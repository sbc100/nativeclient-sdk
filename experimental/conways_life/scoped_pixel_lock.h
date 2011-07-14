// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SCOPED_PIXEL_LOCK_H_
#define SCOPED_PIXEL_LOCK_H_

namespace life {

class LockingImageData;

// A small helper RAII class used to acquire and release the pixel lock based
// on C++ scoping rules.
class ScopedPixelLock {
 public:
  explicit ScopedPixelLock(LockingImageData* pixel_owner)
      : pixel_owner_(pixel_owner) {
    pixels_ = pixel_owner->LockPixels();
  }

  ~ScopedPixelLock() {
    pixels_ = NULL;
    pixel_owner_->UnlockPixels();
  }

  bool is_valid() const {
    return pixels_ != NULL;
  }

  uint32_t* pixels() const {
    return pixels_;
  }

 private:
  LockingImageData* pixel_owner_;  // Weak reference.
  uint32_t* pixels_;  // Weak reference.
};
}  // namespace life

#endif  // SCOPED_PIXEL_LOCK_H_

