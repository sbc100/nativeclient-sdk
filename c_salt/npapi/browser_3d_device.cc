// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/npapi/browser_3d_device.h"

#include <nacl/npapi_extensions.h>

#include "c_salt/instance.h"
#include "c_salt/opengl_context.h"

NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device);

namespace {
const int32_t kCommandBufferSize = 1024 * 1024;
}  // namespace

namespace c_salt {

Browser3DDevice* CreateBrowser3DDevice(const Instance& instance) {
  return new npapi::Browser3DDeviceNPAPI(instance);
}

namespace npapi {

Browser3DDeviceNPAPI::Browser3DDeviceNPAPI(const Instance& instance)
    : instance_(instance), device3d_(NULL) {
  std::memset(&context3d_, 0, sizeof(context3d_));
}

Browser3DDeviceNPAPI::~Browser3DDeviceNPAPI() {
  DeleteBrowser3DContext();
  device3d_ = NULL;
}

bool Browser3DDeviceNPAPI::AcquireBrowser3DDevice() {
  if (is_valid())
    return true;
  device3d_ = NPN_AcquireDevice(instance_.npp_instance(), NPPepper3DDevice);
  assert(device3d_);
  return is_valid();
}

PGLContext Browser3DDeviceNPAPI::CreateBrowser3DContext(
    OpenGLContext* context) {
  NPDeviceContext3DConfig config;
  config.commandBufferSize = kCommandBufferSize;
  assert(device3d_);
  context3d_.repaintCallback = RepaintCallback;
  context3d_.user_data_ = context;
  device3d_->initializeContext(instance_.npp_instance(),
                               &config,
                               &context3d_);
  return pglCreateContext(instance_.npp_instance(), device3d_, &context3d_);
}

void Browser3DDeviceNPAPI::DeleteBrowser3DContext() {
  PGLBoolean success = device3d_->destroyContext(instance_.npp_instance(),
                                                 &context3d_);
  assert(PGL_TRUE == success);
  std::memset(&context3d_, 0, sizeof(context3d_));
}

void Browser3DDeviceNPAPI::RepaintCallback(NPP /* npp NOTUSED */,
                                           NPDeviceContext3D* native_context) {
  assert(native_context);
  DeviceContext3DExt* context =
      static_cast<DeviceContext3DExt*>(native_context);
  OpenGLContext* opengl_context = context->user_data_;
  assert(opengl_context);
  if (opengl_context)
    opengl_context->RenderContext();
}
}  // namespace npapi
}  // namespace c_salt
