// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/opengl_context.h"

#include <pthread.h>

#include <cassert>
#include <cstring>
#include <map>
#include <sstream>

#include "c_salt/browser_3d_device.h"
#include "c_salt/instance.h"
#include "c_salt/module.h"
#include "c_salt/notification_center.h"

namespace c_salt {

// Map the platform-specific context handle to an instance of OpenGLContext.
// This map is lazily allocated on the heap.
typedef std::map<PGLContext, SharedOpenGLContext> ContextDictionary;
static ContextDictionary* g_context_dictionary = NULL;
static pthread_mutex_t g_context_dict_lock = PTHREAD_MUTEX_INITIALIZER;

const char* const OpenGLContext::kInitializeOpenGLContextNotification =
    "kInitializeOpenGLContext.OpenGLContext.c_salt";
const char* const OpenGLContext::kDeleteOpenGLContextNotification =
    "kDeleteOpenGLContext.OpenGLContext.c_salt";
const char* const OpenGLContext::kRenderOpenGLContextNotification =
    "kRenderOpenGLContext.OpenGLContext.c_salt";

OpenGLContext::OpenGLContext(const Instance& instance)
    : instance_(instance),
      pgl_context_(PGL_NO_CONTEXT),
      browser_device_(CreateBrowser3DDevice(instance)) {
  std::stringstream instance_name;
  instance_name << this << ".OpenGLContext.c_salt";
  instance_name_ = instance_name.str();
}

OpenGLContext::~OpenGLContext() {
  DeleteContext();
}

void OpenGLContext::DeleteContext() {
  if (!is_valid())
    return;
  // Make this context current before posting the "will destroy context"
  // notification.  This means that any context tear-down code in observers
  // will operate on this context.
  MakeContextCurrent();
  PostNotification(kDeleteOpenGLContextNotification);
  pglMakeCurrent(PGL_NO_CONTEXT);
  DestroyPGLContext();
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

bool OpenGLContext::MakeContextCurrent() {
  bool new_context_created = false;
  if (!is_valid()) {
    // If this is the first attempt at making the context current, perform all
    // the 3D device initialization.
    browser_device_->AcquireBrowser3DDevice();
    // Create an OpenGL device in the instance.
    c_salt::Module& module = c_salt::Module::GetModuleSingleton();
    if (module.InitializeOpenGL()) {
      new_context_created = CreatePGLContext();
    }
    if (!is_valid()) {
      return false;
    }
  }
  bool success = pglMakeCurrent(pgl_context_) == PGL_TRUE;
  if (!success && pglGetError() == PGL_CONTEXT_LOST) {
    // If the browser context was lost, attempt to create a new one.  Note that
    // CreatePGLContext() will set |new_context_created| to |true|, so that
    // observers can have a chance to re-initialize OpenGL resources if needed.
    DestroyPGLContext();
    new_context_created = CreatePGLContext();
    success = pglMakeCurrent(pgl_context_) == PGL_TRUE;
  }
  if (new_context_created && success) {
    PostNotification(kInitializeOpenGLContextNotification);
  }
  return success;
}

void OpenGLContext::FlushContext() const {
  pglSwapBuffers();
}

void OpenGLContext::RenderContext() {
  if (!MakeContextCurrent())
    return;
  PostNotification(kRenderOpenGLContextNotification);
  FlushContext();
}

bool OpenGLContext::CreatePGLContext() {
  if (is_valid())
    return true;  // Nothing to do here.
  // Create a PGL context.
  pgl_context_ = browser_device_->CreateBrowser3DContext(this);
  if (pgl_context_ == PGL_NO_CONTEXT)
    return false;
  pthread_mutex_lock(&g_context_dict_lock);
  if (!g_context_dictionary) {
    g_context_dictionary = new ContextDictionary();
  }
  SharedOpenGLContext shared_context(this);
  g_context_dictionary->insert(
      ContextDictionary::value_type(pgl_context_, shared_context));
  pthread_mutex_unlock(&g_context_dict_lock);
  return true;
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
  browser_device_->DeleteBrowser3DContext();
}

void OpenGLContext::PostNotification(const char* const notification_name)
    const {
  NotificationCenter* default_center =
      NotificationCenter::DefaultCenter(instance());
  if (default_center == NULL)
    return;
  default_center->PublishNotification(notification_name,
                                      Notification(notification_name),
                                      instance_name_);
}
}  // namespace c_salt
