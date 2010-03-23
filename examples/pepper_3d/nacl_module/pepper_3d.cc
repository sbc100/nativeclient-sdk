// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "examples/pepper_3d/nacl_module/pepper_3d.h"

#include <assert.h>

#include "examples/pepper_3d/nacl_module/cube_view.h"
#include "examples/pepper_3d/nacl_module/scripting_bridge.h"

extern NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device);

namespace {

// Callback given to the browser for things like context repaint and draw
// notifications.

void DrawPepper3D(void* data) {
    static_cast<pepper_3d::Pepper3D*>(data)->DrawSelf();
}

}  // namespace


using pepper_3d::ScriptingBridge;

namespace pepper_3d {

static const int32 kCommandBufferSize = 1024 * 1024;

Pepper3D::Pepper3D(NPP npp)
    : npp_(npp),
      scriptable_object_(NULL),
      device3d_(NULL),
      cube_view_(NULL) {
  memset(&context3d_, 0, sizeof(context3d_));
  ScriptingBridge::InitializeIdentifiers();
}

Pepper3D::~Pepper3D() {
  if (scriptable_object_) {
    NPN_ReleaseObject(scriptable_object_);
  }
  // Destroy the cube view while GL context is current.
  pglMakeCurrent(pgl_context_);
  delete cube_view_;
  pglMakeCurrent(PGL_NO_CONTEXT);

  DestroyContext();
}

NPObject* Pepper3D::GetScriptableObject() {
  if (scriptable_object_ == NULL) {
    scriptable_object_ =
      NPN_CreateObject(npp_, &ScriptingBridge::np_class);
  }
  if (scriptable_object_) {
    NPN_RetainObject(scriptable_object_);
  }
  return scriptable_object_;
}

NPError Pepper3D::SetWindow(const NPWindow& window) {
  if (!pgl_context_) {
    CreateContext();
  }
  if (!pglMakeCurrent(pgl_context_))
    return NPERR_INVALID_INSTANCE_ERROR;
  if (cube_view_ == NULL) {
    cube_view_ = new CubeView();
    cube_view_->PrepareOpenGL();
  }
  cube_view_->Resize(window.width, window.height);
  PostRedrawNotification();
  return NPERR_NO_ERROR;
}

void Pepper3D::PostRedrawNotification() {
  NPN_PluginThreadAsyncCall(npp_, DrawPepper3D, this);
}

bool Pepper3D::DrawSelf() {
  if (cube_view_ == NULL)
    return false;

  if (!pglMakeCurrent(pgl_context_) && pglGetError() == PGL_CONTEXT_LOST) {
    DestroyContext();
    CreateContext();
    pglMakeCurrent(pgl_context_);
    delete cube_view_;
    cube_view_ = new CubeView();
    cube_view_->PrepareOpenGL();
  }

  cube_view_->Draw();
  pglSwapBuffers();
  pglMakeCurrent(PGL_NO_CONTEXT);
  return true;
}

bool Pepper3D::GetCameraOrientation(float* orientation) const {
  if (cube_view_ != NULL) {
    cube_view_->GetOrientation(orientation);
    return true;
  }
  return false;
}

bool Pepper3D::SetCameraOrientation(const float* orientation) {
  if (cube_view_ != NULL) {
    cube_view_->SetOrientation(orientation);
    PostRedrawNotification();
    return true;
  }
  return false;
}

void Pepper3D::CreateContext() {
  if (pgl_context_ != NULL)
    return;
  // Create and initialize a 3D context.
  device3d_ = NPN_AcquireDevice(npp_, NPPepper3DDevice);
  assert(NULL != device3d_);
  NPDeviceContext3DConfig config;
  config.commandBufferSize = kCommandBufferSize;
  device3d_->initializeContext(npp_, &config, &context3d_);

  // Create a PGL context.
  pgl_context_ = pglCreateContext(npp_, device3d_, &context3d_);
}

void Pepper3D::DestroyContext() {
  if (pgl_context_ == NULL)
    return;
  pglDestroyContext(pgl_context_);
  pgl_context_ = NULL;

  device3d_->destroyContext(npp_, &context3d_);
}

}  // namespace pinpapi_bridge

