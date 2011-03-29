// Copyright 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_TUMBLER_OPENGL_CONTEXT_H_
#define EXAMPLES_TUMBLER_OPENGL_CONTEXT_H_

///
/// @file
/// OpenGLContext manages the OpenGL context in the browser that is associated
/// with a @a c_salt::Instance instance.
/// @see c_salt/embedded_app.h
///

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <ppapi/cpp/dev/context_3d_dev.h>
#include <ppapi/cpp/dev/surface_3d_dev.h>
#include <ppapi/cpp/instance.h>
#include <pthread.h>

#include <algorithm>
#include <string>

#include "examples/tumbler/opengl_context_ptrs.h"

namespace tumbler {

/// OpenGLContext manages an OpenGL rendering context in the browser.
///
class OpenGLContext : public boost::noncopyable {
 public:
  OpenGLContext() : flush_pending_(false) {}

  /// Release all the in-browser resources used by this context, and make this
  /// context invalid.
  virtual ~OpenGLContext();

  /// Make @a this the current 3D context in @a instance.
  /// @param instance The instance of the NaCl module that will receive the
  ///                 the current 3D context.
  /// @return success.
  bool MakeContextCurrent(pp::Instance* instance);

  /// Flush the contents of this context to the browser's 3D device.
  void FlushContext();

  /// Make the underlying 3D device invalid, so that any subsequent rendering
  /// commands will have no effect.  The next call to MakeContextCurrent() will
  /// cause the underlying 3D device to get rebound and start receiving
  /// receiving rendering commands again.  Use InvalidateContext(), for
  /// example, when resizing the context's viewing area.
  void InvalidateContext(pp::Instance* instance);

  /// Indicate whether a flush is pending.  This can only be called from the
  /// main thread; it is not thread safe.
  bool flush_pending() const {
    return flush_pending_;
  }
  void set_flush_pending(bool flag) {
    flush_pending_ = flag;
  }

 private:
  pp::Context3D_Dev context_;
  pp::Surface3D_Dev surface_;
  bool flush_pending_;
};

}  // namespace tumbler

#endif  // EXAMPLES_TUMBLER_OPENGL_CONTEXT_H_

