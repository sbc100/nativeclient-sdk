// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_OPENGL_VIEW_H_
#define C_SALT_OPENGL_VIEW_H_

#include "boost/noncopyable.hpp"
#include "c_salt/opengl_context_ptrs.h"

namespace c_salt {

class Instance;
class OpenGLContext;

// Provide the drawing support for an OpenGLContext.  OpenGLContext calls the
// InitializeOpenGL() and RenderOpenGL() methods at the appropriate times.
class OpenGLView : public boost::noncopyable {
 public:
  OpenGLView();
  virtual ~OpenGLView();

  // This method is called by the c_salt rendering pipeline exactly once when
  // |context| first made the current context.  You do not call this method
  // directly. Subclasses can specialize this to perform OpenGL set-up code.
  // This can include compiling and loading shaders, etc.  Before this call is
  // made, the |context| is guaranteed to be the current context and is ready
  // to process OpenGL drawing commands.
  virtual void InitializeOpenGL(const OpenGLContext* context) = 0;

  // This method is called exactly once by the associated context when it is
  // about to be deallocated.  You do not call this method directly.  Subclasses
  // can specialize this method to do graphics shut-down procedures.
  virtual void ReleaseOpenGL(const OpenGLContext* context) = 0;

  // Subclasses specialize this code to draw OpenGL.  The associated context
  // calls this method; you do not call it directly.  |context| is guaranteed
  // to be current before this call is made.
  virtual void RenderOpenGL(const OpenGLContext* context) = 0;

  // Called by the c_salt rendering pipeline whenever the size of the view
  // changes.  Subclasses can specialize this method to recompute viewport-
  // dependent things like the perspective projection transform.  Then this
  // method is called, width() and height() will return the new viewport size
  // values.  You do not need to call this method directly, it is called by
  // the c_salt rendering pipeline.
  virtual void ResizeViewport() = 0;

  // Call this to indicate that the view needs to be refreshed.  This in turn
  // tells the associated context to issue a repaint request.  If |flag| is
  // |false|, then this view will be skipped during rendering.
  void SetNeedsRedraw(bool flag);

  // Attach |this| to |instance|.  If no OpenGLContext has been associated
  // with this view, then one is created in |instance|, and that context is
  // used for rendering.  This call inserts |this| into the rendering chain
  // for |instance|.  |instance| must be fully initialized and ready to make
  // browser calls.  Typically, you would call this method from within an
  // Instance's InstanceDidLoad() method.
  void AddToInstance(const Instance& instance);

  // Clamp the size to 1x1 pixels.  Call ResizeViewport() once the new size
  // information has ben set.
  void SetSize(int32_t width, int32_t height);

  // Accessor and mutator for the OpenGL context.  Setting the context clears
  // and releases the old one, then attaches this view to the new one.  The
  // first time a newly attached context is made current, the InitializeOpenGL()
  // method will be called once.
  SharedOpenGLContext GetOpenGLContext() const;
  void SetOpenGLContext(SharedOpenGLContext context);

  int32_t width() const {
    return width_;
  }
  int32_t height() const {
    return height_;
  }

 private:
  int32_t width_;
  int32_t height_;
  bool needs_redraw_;
  SharedOpenGLContext context_;
};

}  // namespace c_salt

#endif  // C_SALT_OPENGL_VIEW_H_
