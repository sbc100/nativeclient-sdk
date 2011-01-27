// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// This example demonstrates loading, running and scripting a very simple NaCl
// module.  To load the NaCl module, the browser first looks for the
// CreateModule() factory method (ad the end of this file).  It calls
// CreateModule() once to load the module code from your .nexe.  After the
// .nexe code is loaded, CreateModule() is not called again.
//
// Once the .nexe code is loaded, the browser than calls the CreateInstance()
// method on the object returned by CreateModule().  It calls CreateInstance()
// each time it encounters an <embed> tag that references your NaCl module.
//
// When the browser encounters JavaScript that references your NaCl module, it
// calls the GetInstanceObject() method on the object returned from
// CreateInstance().  In this example, the returned object is a subclass of
// ScriptableObject, which handles the scripting support.

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/var.h>
#include <cstdio>
#include <string>

// These are the method names as JavaScript sees them.  Add any methods for
// your class here.
namespace {
const char* const kStubMethodId = "StubMethod";

// This is the module's function.
// The ScriptableObject that called this function then returns the result back
// to the browser as a JavaScript value.
void StubMethod() {
  std::printf("Stub method called.");
}
}  // namespace

// This class exposes the scripting interface for this NaCl module.  The
// HasMethod method is called by the browser when executing a method call on
// the object.  The name of the JavaScript function (e.g. "fortyTwo") is
// passed in the |method| paramter as a string pp::Var.  If HasMethod()
// returns |true|, then the browser will call the Call() method to actually
// invoke the method.
class StubScriptableObject : public pp::deprecated::ScriptableObject {
 public:
  // Return |true| if |method| is one of the exposed method names.
  virtual bool HasMethod(const pp::Var& method, pp::Var* exception);

  // Invoke the function associated with |method|.  The argument list passed in
  // via JavaScript is marshaled into a vector of pp::Vars.  None of the
  // functions in this example take arguments, so this vector is always empty.
  virtual pp::Var Call(const pp::Var& method,
                       const std::vector<pp::Var>& args,
                       pp::Var* exception);
};

bool StubScriptableObject::HasMethod(const pp::Var& method,
                                     pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  std::string method_name = method.AsString();
  bool has_method = (method_name == kStubMethodId);
  return has_method;
}

pp::Var StubScriptableObject::Call(const pp::Var& method,
                                   const std::vector<pp::Var>& args,
                                   pp::Var* exception) {
  if (!method.is_string()) {
    return pp::Var();
  }
  std::string method_name = method.AsString();
  if (method_name == kStubMethodId) {
    StubMethod();
  }
  return pp::Var();
}

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurence of the <embed> tag that has these
// attributes:
//     type="application/x-ppapi-nacl-srpc"
//     nexes="ARM: stub_arm.nexe
//            ..."
// The Instance can return a ScriptableObject representing itself.  When the
// browser encounters JavaScript that wants to access the Instance, it calls
// the GetInstanceObject() method.  All the scripting work is done though
// the returned ScriptableObject.
class StubInstance : public pp::Instance {
 public:
  explicit StubInstance(PP_Instance instance) : pp::Instance(instance) {}
  virtual ~StubInstance() {}

  // The pp::Var takes over ownership of the StubScriptableObject.
  virtual pp::Var GetInstanceObject() {
    StubScriptableObject* hw_object = new StubScriptableObject();
    return pp::Var(this, hw_object);
  }
};

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of you NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-ppapi-nacl-srpc".
class StubModule : public pp::Module {
 public:
  StubModule() : pp::Module() {}
  virtual ~StubModule() {}

  // Create and return a StubInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new StubInstance(instance);
  }
};

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  return new StubModule();
}
}  // namespace pp
