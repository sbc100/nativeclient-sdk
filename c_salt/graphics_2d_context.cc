// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/graphics_2d_context.h"

#include <nacl/nacl_npapi.h>
#include <nacl/npapi_extensions.h>

#include <algorithm>

#include "c_salt/image.h"
#include "c_salt/instance.h"

extern NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device);

namespace c_salt {
class Graphics2DContextImpl {
 public:
  Graphics2DContextImpl()
      : npp_instance_(0), device2d_(NULL) {
    memset(&context2d_, 0, sizeof(context2d_));
  }
  ~Graphics2DContextImpl() {
    if (NULL != device2d_) {
      device2d_->destroyContext(npp_instance_, &context2d_);
      device2d_ = NULL;
    }
  }
  NPP npp_instance_;   // Instance ID.
  NPDevice* device2d_;  // The Pepper1 2D device.
  NPDeviceContext2D context2d_;  // The Pepper1 2D drawing context.
};

Graphics2DContext::Graphics2DContext() : impl_(new Graphics2DContextImpl) {
}

Graphics2DContext::~Graphics2DContext() {
}

bool Graphics2DContext::Init(const c_salt::Instance& instance) {
  impl_->npp_instance_ = instance.npp_instance();
  impl_->device2d_ = NPN_AcquireDevice(impl_->npp_instance_, NPPepper2DDevice);

  NPDeviceContext2DConfig config;
  NPError init_err =
      impl_->device2d_->initializeContext(impl_->npp_instance_,
                                          &config,
                                          &impl_->context2d_);
  if (NPERR_NO_ERROR != init_err)
    return false;

  size_ = Size(impl_->context2d_.dirty.right, impl_->context2d_.dirty.bottom);
  return true;
}

void Graphics2DContext::Update(const Image& img) {
  if (NULL == impl_->device2d_)
    return;

  int w = std::min(size_.width(), img.width());
  int h = std::min(size_.height(), img.height());

  const uint32_t* img_pixels = img.PixelAddress(0, 0);
  uint32_t* window_pixels = static_cast<uint32_t*>(impl_->context2d_.region);
  int window_stride = impl_->context2d_.stride / sizeof(*window_pixels);

  for (int y = 0; y < h; ++y) {
    memcpy(window_pixels, img_pixels, w * sizeof(*window_pixels));
    window_pixels += window_stride;
    img_pixels += img.width();
  }
  impl_->device2d_->flushContext(impl_->npp_instance_,
                                 &impl_->context2d_,
                                 NULL,
                                 NULL);
}
}  // namespace c_salt

