// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_NPAPI_BROWSER_3D_DEVICE_H_
#define C_SALT_NPAPI_BROWSER_3D_DEVICE_H_

///
/// @file
/// Browser3DContext provides the browser-specific support for managing a
/// 3D context in the browser.  Currently, all 3D browser contexts support
/// OpenGL ES 2.0.
/// @see c_salt/opengl_context.h
///

#include <pgl/pgl.h>

#include "boost/noncopyable.hpp"
#include "c_salt/browser_3d_device.h"

namespace c_salt {
class Instance;
class OpenGLContext;

namespace npapi {

///
/// @class Browser3DContext
/// @a Browser3DContext manages access to the 3D device in the browser and a 3D
/// rendering context on that device.
///
class Browser3DDeviceNPAPI : public Browser3DDevice,
                             public boost::noncopyable {
 public:
  /// A newly constructed @a Browser3DContext is invalid until
  /// @a AcquireBrowser3DDevice() is called.  After acquiring the 3D device in
  /// the browser, you then have to create a context by calling
  /// @a CreateBrowser3DContext().
  /// @param instance The Instance instance associated with the in-browser
  /// 3D device and context.
  /// @see AcquireBrowser3DDevice()
  /// @see CreateBrowser3DContext()
  explicit Browser3DDeviceNPAPI(const Instance& instance);

  /// Deletes all active in-browser 3D contexts and releases the 3D device.
  virtual ~Browser3DDeviceNPAPI();

  /// Acquire the 3D device in the browser that is associated with the
  /// @a Instance instance passed into the ctor.  If the 3D device has
  /// already been acquired by a previous call, then this is a no-op.
  /// @return success.
  virtual bool AcquireBrowser3DDevice();

  /// Create a 3D rendering context in the browser, and bind to @a context.
  /// This can be called more than once to create many 3D rendering contexts
  /// associated with the browser's 3D device.
  virtual PGLContext CreateBrowser3DContext(OpenGLContext* context);

  /// Delete the in-browser 3D context and release the 3D device.  After calling
  /// this method, the context is no longer valid.
  virtual void DeleteBrowser3DContext();

  /// The browser context is considered valid when the 3D device has been
  /// succesfully acquired, and a 3D context has been initialized.
  /// @return true if the 3D device is aquired.
  virtual bool is_valid() const {
    return device3d_ != NULL;
  }

 private:
  // Called by the browser whenever the 3D context needs to be repainted.
  // This callback is a simple wrapper that calls the @a RenderContext()
  // method on the associated @a OpenGLContext.
  static void RepaintCallback(NPP npp, NPDeviceContext3D* context);
  // Augment the Pepper V1 context struct so that it can carry user data in
  // its payload.
  struct DeviceContext3DExt : public NPDeviceContext3D {
    // A raw pointer seems kind of scary here, but this is all very temporary
    // so I'm not inclined to think about it too much...
    OpenGLContext* user_data_;
  };
  const Instance& instance_;  // Weak reference.
  // |device3d_| is managed by the browser.  This class maintains a weak
  // reference to it.
  NPDevice* device3d_;
  DeviceContext3DExt context3d_;

  // Not implemented - do not use.
  Browser3DDeviceNPAPI();
};

}  // namespace npapi
}  // namespace c_salt
#endif  // C_SALT_NPAPI_BROWSER_3D_DEVICE_H_
