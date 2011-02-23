// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// This example demonstrates how to load content of the page into NaCl module.

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <cstdio>
#include <string>
#include "examples/geturl/geturl_handler.h"

// These are the method names as JavaScript sees them.
namespace {
const char* const kLoadUrlMethodId = "getUrl";

// Helper function to set the scripting exception.  Both |exception| and
// |except_string| can be NULL.  If |exception| is NULL, this function does
// nothing.
void SetExceptionString(pp::Var* exception, const std::string& except_string) {
  if (exception) {
    *exception = except_string;
  }
}

// Exception strings.  These are passed back to the browser when errors
// happen during property accesses or method calls.
const char* const kExceptionMethodNotAString = "Method name is not a string";
const char* const kExceptionNoMethodName = "No method named ";
const char* const kExceptionStartFailed = "GetURLHandler::Start() failed";
const char* const kExceptionURLNotAString = "URL is not a string";
}  // namespace

// This class exposes the scripting interface for this NaCl module.
class GetURLScriptableObject : public pp::deprecated::ScriptableObject {
 public:
  explicit GetURLScriptableObject(pp::Instance* instance)
      : instance_(instance) {}

  // Return |true| if |method| is one of the exposed method names.
  virtual bool HasMethod(const pp::Var& method, pp::Var* exception);

  // Invoke the function associated with |method|.  The argument list passed in
  // via JavaScript is marshaled into a vector of pp::Vars.
  virtual pp::Var Call(const pp::Var& method,
                       const std::vector<pp::Var>& args,
                       pp::Var* exception);
 private:
  pp::Instance* instance_;
};

bool GetURLScriptableObject::HasMethod(const pp::Var& method,
                                       pp::Var* exception) {
  if (!method.is_string()) {
    SetExceptionString(exception, kExceptionMethodNotAString);
    return false;
  }
  std::string method_name = method.AsString();
  return method_name == kLoadUrlMethodId;
}

pp::Var GetURLScriptableObject::Call(const pp::Var& method,
                                     const std::vector<pp::Var>& args,
                                     pp::Var* exception) {
  if (!method.is_string()) {
    SetExceptionString(exception, kExceptionMethodNotAString);
    return pp::Var();
  }
  std::string method_name = method.AsString();
  if (method_name == kLoadUrlMethodId) {
    if ((args.size() >= 1) && args[0].is_string()) {
      std::string url = args[0].AsString();
      printf("GetURLScriptableObject::Call('%s'', '%s'')\n",
             method_name.c_str(),
             url.c_str());
      fflush(stdout);
      GetURLHandler* handler = GetURLHandler::Create(instance_, url);
      if (handler != NULL) {
        // Starts asynchronous download. When download is finished or when an
        // error occurs, |handler| calls JavaScript function
        // reportResult(url, result, success) (defined in geturl.html) and
        // self-destroys.
        if (!handler->Start()) {
          SetExceptionString(exception, kExceptionStartFailed);
        }
      } else {
        const char* msg = "GetURLHandler::Create failed";
        printf("%s\n", msg);
        SetExceptionString(exception, msg);
      }
    } else {
      SetExceptionString(exception, kExceptionURLNotAString);
    }
  } else {
    SetExceptionString(exception,
                       std::string(kExceptionNoMethodName) + method_name);
  }
  return pp::Var();
}

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurrence of the <embed> tag that has these
// attributes:
//     type="application/x-nacl"
//     nacl="geturl.nmf"
//
// The Instance can return a ScriptableObject representing itself.  When the
// browser encounters JavaScript that wants to access the Instance, it calls
// the GetInstanceObject() method.  All the scripting work is done though
// the returned ScriptableObject.
class GetURLInstance : public pp::Instance {
 public:
  explicit GetURLInstance(PP_Instance instance) : pp::Instance(instance) {}
  virtual ~GetURLInstance() {}

  // The pp::Var takes over ownership of the GetURLScriptableObject.
  virtual pp::Var GetInstanceObject() {
    return pp::Var(this, new GetURLScriptableObject(this));
  }
};

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of you NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-ppapi-nacl-srpc".
class GetURLModule : public pp::Module {
 public:
  GetURLModule() : pp::Module() {}
  virtual ~GetURLModule() {}

  // Create and return a GetURLInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new GetURLInstance(instance);
  }
};

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  return new GetURLModule();
}
}  // namespace pp

