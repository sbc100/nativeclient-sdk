// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_BROWSER_3D_DEVICE_H_
#define C_SALT_BROWSER_3D_DEVICE_H_

///
/// @file
/// Browser3DContext provides the browser-specific support for managing an
/// 3D context in the browser.  Currently, all 3D browser contexts support
/// OpenGL ES 2.0.
/// @see c_salt/opengl_context.h
///

#include <pgl/pgl.h>

namespace c_salt {
class Instance;
class OpenGLContext;

///
/// @class Browser3DContext
/// @a Browser3DContext manages access to the 3D device in the browser and a 3D
/// rendering context on that device.  A newly constructed @a Browser3DContext
/// is invalid until @a AcquireBrowser3DDevice() is called.  After acquiring
/// the 3D device in the browser, you then have to create a context by calling
/// @a CreateBrowser3DContext().
/// @see AcquireBrowser3DDevice()
/// @see CreateBrowser3DContext()
///
class Browser3DDevice {
 public:
  /// Deletes all active in-browser 3D contexts and releases the 3D device.
  virtual ~Browser3DDevice() {}

  /// Acquire the 3D device in the browser that is associated with the
  /// @a Instance instance passed into the ctor.
  /// @return success.
  virtual bool AcquireBrowser3DDevice() = 0;

  /// Create a 3D rendering context in the browser, and bind to @a context.
  /// This can be called more than once to create many 3D rendering contexts
  /// associated with the browser's 3D device.
  virtual PGLContext CreateBrowser3DContext(OpenGLContext* context) = 0;

  /// Delete the in-browser 3D context and release the 3D device.  After calling
  /// this method, the context is no longer valid.
  virtual void DeleteBrowser3DContext() = 0;

  /// The browser context is considered valid when the 3D device has been
  /// succesfully acquired, and a 3D context has been initialized.
  /// @return true if the 3D device is aquired.
  virtual bool is_valid() const = 0;
};

/// Browser-specific subclasses must implement this factory method.  The caller
/// takes ownership of the returned object.
/// @param instance The Instance instance associated with the in-browser
/// 3D device and context.
/// @return A newly-created Browser3DDevice instance.
Browser3DDevice* CreateBrowser3DDevice(const Instance& instance);

}  // namespace c_salt
#endif  // C_SALT_BROWSER_3D_DEVICE_H_
