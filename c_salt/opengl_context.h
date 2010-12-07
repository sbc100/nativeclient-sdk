// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_OPENGL_CONTEXT_H_
#define C_SALT_OPENGL_CONTEXT_H_

///
/// @file
/// OpenGLContext manages the OpenGL context in the browser that is associated
/// with a @a c_salt::Instance instance.
/// @see c_salt/embedded_app.h
///

#include <pgl/pgl.h>
#include <pthread.h>

#include <algorithm>
#include <string>

#include "boost/noncopyable.hpp"
#include "boost/scoped_ptr.hpp"
#include "c_salt/opengl_context_ptrs.h"

namespace c_salt {
class Browser3DDevice;
class Instance;

/// OpenGLContext manages an OpenGL rendering context in the browser.
/// Each OpenGLContext is associated with a @a c_salt::Instance; by default,
/// the OpenGLContext renders directly into the browser area owned by the
/// @a c_salt::Instance instance.  You can also create an OpenGLContext that
/// renders into an off-screen FBO.
///
/// Note that all OpenGLContext instance must be bound to a
/// @a c_salt::Instance instance.  They cannot operate stand-alone, this
/// includes contexts that only render to an FBO.
///
class OpenGLContext : public boost::noncopyable {
 public:
  /// Bind to @a instance.  The in-browser context is not actually created
  /// until it's needed, which is usually the first time MakeContextCurrent()
  /// is called.
  /// @param instance The Instance instance associated with this 3D device
  ///                 and context.
  /// @see MakeContextCurrent()
  explicit OpenGLContext(const Instance& instance);

  /// Dtor makes this the current context, then posts a |kDeleteOpenGLContext|
  /// notification.  Destroys the underlying OpenGL context once the
  /// notification handlers have all returned.
  virtual ~OpenGLContext();

  /// Get the current context on the calling thread.
  /// @return The current context.  If there is no OpenGLContext for the
  ///         calling thread, return NULL.
  static SharedOpenGLContext CurrentContext();

  /// Make this context the current rendering context.  If the underlying OpenGL
  /// context is not valid, this routine makes the current context invalid.
  bool MakeContextCurrent();

  /// Flush the contents of this context to the browser's 3D device.
  void FlushContext() const;

  /// Makes the OpenGL context owned by this instance the current context on
  /// the calling thread, and then posts a |kRenderOpenGLContext| notification.
  /// When the notification handlers have all returned, the context is flushed
  /// to the browser's 3D device.  This OpenGL context remains the current
  /// context.
  /// @see FlushContext()
  void RenderContext();

  /// Release all the in-browser resources used by this context, and make this
  /// context invalid.  First, make this context current and post the a
  /// |kDeleteOpenGLContext| notification; this allows observers to perform any
  /// necessary clean-up.
  void DeleteContext();

  /// @return The Instance instance associated with this context.
  const Instance& instance() const {
    return instance_;
  }

  /// Get the OpenGL context owned by this instance.  Modifications made to the
  /// raw context are not tracked by this class.  If you delete the raw context
  /// directly, then further use of this class will have unpredictable results.
  /// @return The raw OpenGL context owned by this instance.
  PGLContext pgl_context() const {
    return pgl_context_;
  }

  /// @return @a true if the context is valid.  The context is valid if the
  /// raw OpenGL context exists.
  bool is_valid() const {
    return pgl_context_ != PGL_NO_CONTEXT;
  }

  /// Get the instance name used when posting notifications.  Observers can use
  /// this name to receive notificaiton from this instance only.
  /// @return The unique name for this instance.
  const std::string& instance_name() const {
    return instance_name_;
  }

  // Notifications posted by this class.

  /// Posted once when the context is first made current.  Observers can perform
  /// any one-time context initialization in their notification handlers.  The
  /// OpenGL context owned by this instance is guaranteed to be the current
  /// context when this notification is posted.
  static const char* const kInitializeOpenGLContextNotification;

  /// Posted once when the context is about to be deleted. The OpenGL context
  /// owned by this instance is guaranteed to be the current context when this
  /// notification is published.
  static const char* const kDeleteOpenGLContextNotification;

  /// Posted whenever the context needs to be re-rendered. This can be in
  /// response to a repaint event from the browser.  The OpenGL context owned
  /// by this instance is guaranteed to be current when this notification is
  /// posted.  Observers do not need to flush the context while handling this
  /// notification, the context is flushed to the in-browser 3D device when all
  /// the observers have returned from their handlers.  An observer can flush
  /// the context if needed, however (although performance may suffer if the
  /// flush blocks).
  static const char* const kRenderOpenGLContextNotification;

 private:
  // Create the PGL context, return true on success.  On successful return,
  // |pgl_context_| will be valid and ready for rendering.
  bool CreatePGLContext();
  // Destroy the PGL context.  On return, |pgl_context_| will be invalid and
  // have a value of PGL_NO_CONTEXT.
  void DestroyPGLContext();

  // Post |notification_name| to the default notification center associated
  // with |instance_|.
  void PostNotification(const char* const notification_name) const;

  const Instance& instance_;  // Weak reference.
  PGLContext pgl_context_;  // The OpenGL context.
  // The unique name of this instance, used for publishing notifications.
  std::string instance_name_;
  boost::scoped_ptr<Browser3DDevice> browser_device_;

  OpenGLContext();  // Not implemented, do not use.
};

}  // namespace c_salt

#endif  // C_SALT_OPENGL_CONTEXT_H_
