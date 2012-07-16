// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// This project is for use with the testing framework
/// of the Visual Studio Add-in
#include <string>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

class DummyInstance : public pp::Instance {
 public:
  /// The constructor creates the plugin-side instance.
  /// @param[in] instance the handle to the browser-side plugin instance.
  explicit DummyInstance(PP_Instance instance)
      : pp::Instance(instance) {
  }

  virtual ~DummyInstance() {
  }

  virtual bool Init(uint32_t /*argc*/, const char* /*argn*/[],
      const char* /*argv*/[]) {
    // Start chain of message relaying
    PostMessage(pp::Var("relay1"));
    return true;
  }

 private:
  virtual void HandleMessage(const pp::Var& var_message) {
    // Simply relay back to javascript the message we just received
    if (!var_message.is_string())
      return;
    std::string msg = var_message.AsString();
    PostMessage(pp::Var(msg));
  }
};

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class DummyModule : public pp::Module {
 public:
  DummyModule() : pp::Module() {}
  virtual ~DummyModule() {}

  /// Create and return a FileIoInstance object.
  /// @param[in] instance The browser-side instance.
  /// @return the plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new DummyInstance(instance);
  }
};

namespace pp {
Module* CreateModule() {
  return new DummyModule();
}
}  // namespace pp

