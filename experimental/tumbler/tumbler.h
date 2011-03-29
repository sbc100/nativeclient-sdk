// Copyright 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_TUMBLER_TUMBLER_H_
#define EXAMPLES_TUMBLER_TUMBLER_H_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <ppapi/cpp/instance.h>
#include <pthread.h>
#include <map>
#include <vector>

#include "examples/tumbler/cube.h"
#include "examples/tumbler/opengl_context.h"
#include "examples/tumbler/opengl_context_ptrs.h"
#include "examples/tumbler/scripting_bridge.h"

namespace tumbler {

class Tumbler : public pp::Instance, boost::noncopyable {
 public:
  explicit Tumbler(PP_Instance instance) : pp::Instance(instance) {}

  // The dtor makes the 3D context current before deleting the cube view, then
  // destroys the 3D context both in the module and in the browser.
  virtual ~Tumbler();

  // The browser calls this function to get a handle, in form of a pp::Var,
  // to the Native Client-side scriptable object.  The pp::Var takes over
  // ownership of the returned object, meaning the browser can call its
  // destructor.
  // Returns the browser's handle to the plugin side instance.
  virtual pp::Var GetInstanceObject();

  // Called whenever the in-browser window changes size.
  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

  // Bind and publish the module's methods to JavaScript.
  void InitializeMethods(ScriptingBridge* bridge);

  // Fill in the passed-in quaternion with the quaternion that represents the
  // current camera orientation.  Returns success.
  // This method is bound to the JavaScript "getCameraOrientation" method and
  // is called like this:
  //     var orientation = [0.0, 0.0, 1.0, 0.0];
  //     module.getCameraOrientation(orientation);
  pp::Var GetCameraOrientation(const ScriptingBridge& bridge,
                               const std::vector<pp::Var>& args);

  // Set the camera orientation to the quaternion in |args[0]|.  |args| must
  // have length at least 1; the first element is expeted to be an Array
  // object containing 4 floating point number elements (the quaternion).
  // This method is bound to the JavaScript "setCameraOrientation" method and
  // is called like this:
  //     module.setCameraOrientation([0.0, 1.0, 0.0, 0.0]);
  pp::Var SetCameraOrientation(const ScriptingBridge& bridge,
                               const std::vector<pp::Var>& args);

  // Called to draw the contents of the module's browser area.
  void DrawSelf();

 private:
  SharedOpenGLContext opengl_context_;
  boost::scoped_ptr<Cube> cube_;
};

}  // namespace tumbler

#endif  // EXAMPLES_TUMBLER_TUMBLER_H_
