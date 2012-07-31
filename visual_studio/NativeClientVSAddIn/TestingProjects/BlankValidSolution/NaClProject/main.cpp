// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// This project is for use with the testing framework
/// of the Visual Studio Add-in.
/// Note that the PPAPI and NaCl configurations do not exist. These are added
/// during testing time so that the most up-to-date settings are used.

#include <string>

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

class NaClProjectInstance : public pp::Instance {
 public:
  /// The constructor creates the plugin-side instance.
  /// @param[in] instance the handle to the browser-side plugin instance.
  explicit NaClProjectInstance(PP_Instance instance)
      : pp::Instance(instance) {
  }

  virtual ~NaClProjectInstance() {
  }

  virtual bool Init(uint32_t /*argc*/, const char* /*argn*/[],
      const char* /*argv*/[]) {
    // Start chain of message relaying.
    PostMessage(pp::Var("relay1"));
    return true;
  }

 private:
  virtual void HandleMessage(const pp::Var& var_message) {
    // Simply relay back to javascript the message we just received.
    if (!var_message.is_string())
      return;
    std::string msg = var_message.AsString();
    PostMessage(pp::Var(msg));
  }
};

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class NaClProjectModule : public pp::Module {
 public:
  NaClProjectModule() : pp::Module() {}
  virtual ~NaClProjectModule() {}

  /// Create and return a FileIoInstance object.
  /// @param[in] instance The browser-side instance.
  /// @return the plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new NaClProjectInstance(instance);
  }
};

namespace pp {
Module* CreateModule() {
  return new NaClProjectModule();
}
}  // namespace pp

