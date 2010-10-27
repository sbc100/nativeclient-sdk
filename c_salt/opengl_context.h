// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_OPENGL_CONTEXT_H_
#define C_SALT_OPENGL_CONTEXT_H_

#include <pthread.h>
#include <pgl/pgl.h>

// TODO(c_salt_authors): Remove this when the API-specific layer supports
// creation of 3D contexts.
#include <nacl/npapi_extensions.h>

#include <algorithm>

#include "boost/noncopyable.hpp"
#include "c_salt/opengl_context_ptrs.h"
#include "c_salt/opengl_view.h"
#include "c_salt/opengl_view_ptrs.h"

namespace c_salt {

class Instance;

// Manage an OpenGL context associated with an OpenGLView.  This class
// encapsulates the 3D OpenGL rendering context that the bound OpenGLView
// uses to do the actual drawing.  All OpenGLContexts must be associated with
// an Instance object in the browser, they cannot operate stand-alone.
class OpenGLContext : public boost::noncopyable {
 public:
  // Ctor might not immediately create the underlying PGL context.  The context
  // is not actually created until it's needed, which is usually the first time
  // MakeContextCurrent() is called.
  OpenGLContext(const Instance& instance);
  virtual ~OpenGLContext();

  // Get the current context on the calling thread.
  static SharedOpenGLContext CurrentContext();

  // Called when the owning Instance is fully connected to the browser; at this
  // point, implementations can safely make calls into the browser.  This
  // method must be called before the context can be made current.
  virtual bool InitializeOpenGL();

  // Make this context the current rendering context.  If the underlying PGL
  // context is not valid, this routine changes the current context to
  // PGL_NO_CONTEXT.
  virtual bool MakeContextCurrent() const;

  // Flush the contents of this context to the real device.
  virtual void FlushContext() const;

  // Arranges to make |this| the current context on the calling thread, and
  // then calls the RenderOpenGL()_ method on |opengl_view_| (if valid).
  virtual void RenderOpenGL();

  const Instance& instance() const {
    return instance_;
  }

  SharedOpenGLView opengl_view() const {
    return opengl_view_;
  }

  // Resetting the view has the side-effect of calling InitializeOpenGL()
  // on the view.
  void set_opengl_view(SharedOpenGLView view) {
    opengl_view_ = view;
    first_make_current_ = true;
  }

  PGLContext pgl_context() const {
    return pgl_context_;
  }

  bool is_valid() const {
    return pgl_context_ != PGL_NO_CONTEXT;
  }

 private:
  friend class OpenGLView;

  // Create and destroy the PGL context.
  void CreatePGLContext();
  void DestroyPGLContext();

  const Instance& instance_;  // Weak reference.
  PGLContext pgl_context_;
  SharedOpenGLView opengl_view_;
  // |first_make_current_| is mutable so that it can be changed within the
  // const MakeContextCurrent() method.
  mutable bool first_make_current_;

  OpenGLContext();  // Not implemented, do not use.

  // TODO(c_salt_authors): This API-specific stuff needs to go into
  // the API-specific support layer impl.
  static void RepaintCallback(NPP npp, NPDeviceContext3D* context);
  // Augment the Pepper V1 context struct so that it can carry user data in
  // its payload.
  struct DeviceContext3DExt : public NPDeviceContext3D {
    // A raw pointer seems kind of scary here, but this is all very temporary
    // so I'm not inclined to think about it too much...
    OpenGLContext* user_data_;
  };
  NPDevice* device3d_;  // Weak reference.
  DeviceContext3DExt context3d_;
};

}  // namespace c_salt

#endif  // C_SALT_OPENGL_CONTEXT_H_
