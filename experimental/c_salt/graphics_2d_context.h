// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_GRAPHICS_2D_CONTEXT_H_
#define C_SALT_GRAPHICS_2D_CONTEXT_H_

#include "c_salt/rect.h"
#include "boost/scoped_ptr.hpp"

namespace c_salt {
class Image;
class Instance;
class Graphics2DContextImpl;

// Manage a 2D graphics context associated with an instance.  This class
// encapsulates the 2D rendering context.
// drawing.
class Graphics2DContext {
 public:
  // Constructor might not immediately create the underlying 2D graphics
  // context. The context is not actually created until Init() is called.
  Graphics2DContext();
  virtual ~Graphics2DContext();

  // Called when the owning Instance is fully connected to the browser; at this
  // point, implementations can safely make calls into the browser.
  bool Init(const c_salt::Instance& instance);

  // Returns the size of the plugin image.
  const Size& GetSize() const {return size_;}

  // Copy |img| pixels into graphics context and flush the contents of this
  // context to the real device.
  void Update(const Image& img);

 protected:
  // Not implemented.
  Graphics2DContext(const Graphics2DContext&);
  Graphics2DContext& operator=(const Graphics2DContext&);

  boost::scoped_ptr<Graphics2DContextImpl> impl_;
  Size size_;  // size of the plugin image.
};
}  // namespace c_salt

#endif  // C_SALT_GRAPHICS_2D_CONTEXT_H_

