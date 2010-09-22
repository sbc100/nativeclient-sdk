// Copyright 2008 The Native Client Authors. All rights reserved.
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
#include <ppapi/cpp/scriptable_object.h>
#include <ppapi/cpp/var.h>

// These are the method names as JavaScript sees them.
namespace {
const char* kHelloWorldMethodId = "helloWorld";
const char* kFortyTwoMethodId = "fortyTwo";

// This is the module's function that does the work to compute the value 42.
// The ScriptableObject that called this function then returns the result back
// to the browser as a JavaScript value.
int32_t FortyTwo() {
  return 42;
}

// This function returns a pointer to some constant memory holding the string
// "hello, world.".  The ScriptableObject that called this function then returns
// the result back to the browser as a JavaScript value
std::string HelloWorld() {
  const char *msg = "hello, world.";
  return msg;
}
}  // namespace

// This class exposes the scripting interface for this NaCl module.  The
// HasMethod method is called by the browser when executing a method call on
// the |helloWorld| object (see, e.g. the helloWorld() function in
// hello_world.html).  The name of the JavaScript function (e.g. "fortyTwo") is
// passed in the |method| paramter as a string pp::Var.  If HasMethod()
// returns |true|, then the browser will call the Call() method to actually
// invoke the method.
class HelloWorldScriptableObject : public pp::ScriptableObject {
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

bool HelloWorldScriptableObject::HasMethod(const pp::Var& method,
                                           pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  std::string method_name = method.AsString();
  bool has_method = method_name == kHelloWorldMethodId ||
      method_name == kFortyTwoMethodId;
  return has_method;
}

pp::Var HelloWorldScriptableObject::Call(const pp::Var& method,
                                         const std::vector<pp::Var>& args,
                                         pp::Var* exception) {
  if (!method.is_string()) {
    return pp::Var();
  }
  std::string method_name = method.AsString();
  if (method_name == kHelloWorldMethodId)
    return pp::Var(HelloWorld());
  else if (method_name == kFortyTwoMethodId)
    return pp::Var(FortyTwo());
  return pp::Var();
}

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurence of the <embed> tag that has these
// attributes:
//     type="application/x-ppapi-nacl-srpc"
//     nexes="ARM: hello_world_arm.nexe
//            ..."
// The Instance can return a ScriptableObject representing itself.  When the
// browser encounters JavaScript that wants to access the Instance, it calls
// the GetInstanceObject() method.  All the scripting work is done though
// the returned ScriptableObject.
class HelloWorldInstance : public pp::Instance {
 public:
  HelloWorldInstance(PP_Instance instance) : pp::Instance(instance) {}
  virtual ~HelloWorldInstance() {}

  // The pp::Var takes over ownership of the HelloWorldScriptableObject.
  virtual pp::Var GetInstanceObject() {
    HelloWorldScriptableObject* hw_object = new HelloWorldScriptableObject();
    return pp::Var(hw_object);
  }
};

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of you NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-ppapi-nacl-srpc".
class HelloWorldModule : public pp::Module {
 public:
  HelloWorldModule() : pp::Module() {}
  virtual ~HelloWorldModule() {}

  // Create and return a HelloWorldInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new HelloWorldInstance(instance);
  }
};

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  return new HelloWorldModule();
}
}  // namespace pp
