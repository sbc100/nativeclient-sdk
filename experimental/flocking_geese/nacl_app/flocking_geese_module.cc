// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nacl_app/flocking_geese_app.h"
#include "ppapi/cpp/module.h"

namespace flocking_geese {
// The Module class.  The browser calls the CreateInstance() method to create
// an instance of your NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-nacl".
class FlockingGeeseModule : public pp::Module {
 public:
  FlockingGeeseModule() : pp::Module() {}
  virtual ~FlockingGeeseModule() {}

  /// Create and return a FlockingGeeseInstance object.
  /// @param[in] instance The browser-side instance.
  /// @return the plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new FlockingGeeseApp(instance);
  }
};
}  // namespace flocking_geese

namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
Module* CreateModule() {
  return new flocking_geese::FlockingGeeseModule();
}
}  // namespace pp
