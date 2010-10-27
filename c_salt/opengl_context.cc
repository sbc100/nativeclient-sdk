// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/opengl_context.h"

#include <pthread.h>

#include <cassert>
#include <cstring>
#include <map>

#include "c_salt/instance.h"
#include "c_salt/module.h"

// TODO(c_salt_authors): remove this when 3D devices are supported in
// the API-specific support layer.
NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device);

// TODO(c_salt_authors): this is an artifact of Pepper V1; remove when migrating
// to Pepper V2.
namespace {
const int32_t kCommandBufferSize = 1024 * 1024;
}  // namespace

namespace c_salt {

// Map the platform-specific context handle to an instance of OpenGLContext.
// This map is lazily allocated on the heap.
typedef std::map<PGLContext, SharedOpenGLContext> ContextDictionary;
static ContextDictionary* g_context_dictionary = NULL;
static pthread_mutex_t g_context_dict_lock = PTHREAD_MUTEX_INITIALIZER;

OpenGLContext::OpenGLContext(const Instance& instance)
    : instance_(instance),
      pgl_context_(PGL_NO_CONTEXT),
      first_make_current_(true),
      device3d_(NULL) {
  std::memset(&context3d_, 0, sizeof(context3d_));
}

OpenGLContext::~OpenGLContext() {
  pglMakeCurrent(pgl_context_);
  if (opengl_view_.get()) opengl_view_->ReleaseOpenGL(this);
  pglMakeCurrent(PGL_NO_CONTEXT);
  DestroyPGLContext();
}

bool OpenGLContext::InitializeOpenGL() {
  // TODO(c_salt_authors): NPAPI here!!  Move it to BrowserBinding.
  device3d_ = NPN_AcquireDevice(instance().npp_instance(), NPPepper3DDevice);
  assert(device3d_);
  if (pgl_context_ == PGL_NO_CONTEXT) {
    // Create an OpenGL device in the instance.
    c_salt::Module& module = c_salt::Module::GetModuleSingleton();
    if (module.InitializeOpenGL()) {
      CreatePGLContext();
    }
  }
  return pgl_context_ != PGL_NO_CONTEXT;
}

SharedOpenGLContext OpenGLContext::CurrentContext() {
  SharedOpenGLContext current_context;
  PGLContext current_pgl_context = pglGetCurrentContext();
  if (g_context_dictionary != NULL &&
      current_pgl_context != PGL_NO_CONTEXT) {
    pthread_mutex_lock(&g_context_dict_lock);
    ContextDictionary::const_iterator iter =
      g_context_dictionary->find(current_pgl_context);
    if (iter != g_context_dictionary->end())
      current_context = iter->second;
    pthread_mutex_unlock(&g_context_dict_lock);
  }
  return current_context;
}

bool OpenGLContext::MakeContextCurrent() const {
  if (pgl_context_ == PGL_NO_CONTEXT) {
    return false;
  }
  bool success = !!pglMakeCurrent(pgl_context_);
  if (first_make_current_ && success) {
    first_make_current_ = false;
    if (opengl_view_.get()) opengl_view_->InitializeOpenGL(this);
  }
  return success;
}

void OpenGLContext::FlushContext() const {
  pglSwapBuffers();
}

void OpenGLContext::RepaintCallback(NPP /* npp NOTUSED */,
                                    NPDeviceContext3D* native_context) {
  assert(native_context);
  DeviceContext3DExt* context =
      static_cast<DeviceContext3DExt*>(native_context);
  OpenGLContext* opengl_context = context->user_data_;
  assert(opengl_context);
  if (opengl_context)
    opengl_context->RenderOpenGL();
}

void OpenGLContext::RenderOpenGL() {
  if (!pglMakeCurrent(pgl_context_) && pglGetError() == PGL_CONTEXT_LOST) {
    DestroyPGLContext();
    CreatePGLContext();
    pglMakeCurrent(pgl_context_);
  }
  if (opengl_view_.get()) opengl_view_->RenderOpenGL(this);
  pglSwapBuffers();
  pglMakeCurrent(PGL_NO_CONTEXT);
}

void OpenGLContext::CreatePGLContext() {
  if (is_valid())
    return;  // Nothing to do here.
  // Initialize a 3D context.
  NPDeviceContext3DConfig config;
  config.commandBufferSize = kCommandBufferSize;
  device3d_->initializeContext(instance().npp_instance(), &config, &context3d_);
  context3d_.repaintCallback = RepaintCallback;
  context3d_.user_data_ = this;
  // Create a PGL context.
  pgl_context_ = pglCreateContext(instance().npp_instance(),
                                  device3d_,
                                  &context3d_);
  if (pgl_context_ == PGL_NO_CONTEXT)
    return;
  pthread_mutex_lock(&g_context_dict_lock);
  if (!g_context_dictionary) {
    g_context_dictionary = new ContextDictionary();
  }
  SharedOpenGLContext shared_context(this);
  g_context_dictionary->insert(
      ContextDictionary::value_type(pgl_context_, shared_context));
  pthread_mutex_unlock(&g_context_dict_lock);
  first_make_current_ = true;
}

void OpenGLContext::DestroyPGLContext() {
  assert(is_valid());
  assert(g_context_dictionary);
  if (g_context_dictionary) {
    pthread_mutex_lock(&g_context_dict_lock);
    ContextDictionary::iterator iter =
      g_context_dictionary->find(pgl_context_);
    if (iter != g_context_dictionary->end())
      g_context_dictionary->erase(iter);
    pthread_mutex_unlock(&g_context_dict_lock);
  }
  pglDestroyContext(pgl_context_);
  pgl_context_ = PGL_NO_CONTEXT;
  PGLBoolean success = device3d_->destroyContext(instance().npp_instance(),
                                                 &context3d_);
  assert(PGL_TRUE == success);
  device3d_ = NULL;
  std::memset(&context3d_, 0, sizeof(context3d_));
}


}  // namespace c_salt
