// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/opengl_view.h"

#include <algorithm>

#include "c_salt/instance.h"
#include "c_salt/opengl_context.h"

namespace c_salt {

OpenGLView::OpenGLView() : width_(1), height_(1), needs_redraw_(true) {
}

OpenGLView::~OpenGLView() {
}

SharedOpenGLContext OpenGLView::GetOpenGLContext() const {
  return context_;
}

void OpenGLView::SetOpenGLContext(SharedOpenGLContext context) {
  context_ = context;  // Might cause ReleaseOpenGL() to be called.
  SharedOpenGLView shared_view(this);
  context_->set_opengl_view(shared_view);
}

void OpenGLView::SetNeedsRedraw(bool flag) {
  needs_redraw_ = flag;
  if (needs_redraw_) {
    // Signal the associated context to issue a repaint request.
    context_->RenderOpenGL();
  }
}

void OpenGLView::AddToInstance(const Instance& instance) {
  if (context_.get() && context_->is_valid()) {
    // This view already belongs to a context, do nothing.
    return;
  }
  SharedOpenGLContext shared_context(new OpenGLContext(instance));
  shared_context->InitializeOpenGL();
  SetOpenGLContext(shared_context);
}

void OpenGLView::SetSize(int32_t width, int32_t height) {
  width_ = std::max(static_cast<int32_t>(1), width);
  height_ = std::max(static_cast<int32_t>(1), height);
  assert(context_.get());
  if (context_.get() && context_->is_valid())
    context_->MakeContextCurrent();
  ResizeViewport();
}

}  // namespace c_salt
