// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "experimental/conways_life/life_application.h"
#include "ppapi/cpp/module.h"

namespace life {
// The Module class.  The browser calls the CreateInstance() method to create
// an instance of you NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-nacl".
class LifeModule : public pp::Module {
 public:
  LifeModule() : pp::Module() {}
  virtual ~LifeModule() {}

  // Create and return a Life instance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new LifeApplication(instance);
  }
};
}  // namespace life

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  return new life::LifeModule();
}
}  // namespace pp

