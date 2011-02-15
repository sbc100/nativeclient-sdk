// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_SCRIPTING_BRIDGE_H_
#define C_SALT_SCRIPTING_BRIDGE_H_

// TODO(dspringer, dmichael): Remove this:
#include <nacl/npruntime.h>

#include <map>
#include <string>
#include <vector>

#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "c_salt/callback.h"
#include "c_salt/property.h"
#include "c_salt/scriptable_native_object_ptrs.h"
#include "c_salt/variant.h"

namespace c_salt {

class Instance;
class Module;

namespace npapi {
class BrowserBinding;
}  // namespace npapi

// This class handles all the calls across the bridge to the browser via its
// browser binding object, |browser_binding_|.  Note that NPObjects cannot have
// a vtable, hence the PIMPL pattern used here.  Use AddMethodNamed() and
// AddProperty() to publish methods and static properties that can be accessed
// from the browser code.
//
// TODO(dspringer): |browser_binding_| gets replaced by pp::ScriptableObject
// when Pepper v2 becomes available.

class ScriptingBridge : public c_salt::ScriptingInterface,
                        public boost::noncopyable {
 public:
  virtual ~ScriptingBridge();

  // Causes |method_name| to be published as a method that can be called by
  // JavaScript.  Associated this method with |method|.
  //
  // Usage:
  //   AddMethodNamed("myFunc",  /* This is the JavaScript name for the method
  //                                you want to expose. */
  //                  &handler,  /* This is a pointer to an instance of the
  //                                handler object. */
  //                  &Handler::MyFunc  /* This is the class method that will
  //                                       get called when "myFunc" is invoked
  //                                       from JavaScript. */
  //                 );
  //
  // Example:
  //   class MyClass {
  //     void InitializeMethods(ScriptingBridge*);
  //     bool MyFunc(int32_t, double, std::string);
  //   };
  //
  //   MyClass::InitializeMethods(ScriptingBridge* bridge) {
  //      bridge->AddMethodNamed("myFunc", this, MyClass::MyFunc);
  //   }
  //
  // Caveats:
  //   - Currently, only up to 6 arguments are supported.
  //   - void return types are currently not supported.
  //   - Supported parameter types are currently limited to:
  //     std::string, int32_t, bool, double, and shared_ptr<ScriptingBridge>
  template <class T, class Signature>
  bool AddMethodNamed(const std::string& method_name,
                      T* handler,
                      Signature method) {
    if (method_name.empty() || method == NULL)
      return false;
    SharedMethodCallbackExecutor method_ptr(
      new MethodCallbackExecutorImpl<Signature>(handler, method));
    method_dictionary_.insert(MethodDictionary::value_type(method_name,
                                                           method_ptr));
    return true;
  }

  // Adds |property| to the property dictionary.  These properties should all
  // be declared as static, mutable properties (the default).  See
  // c_salt/property.h for details.  Example usage:
  //   SharedVariant value(new c_salt::Variant(42));
  //   PropertyAttributes prop_attrib("myProp", value);
  //   bridge->AddProperty(Property(prop_attribs));
  bool AddProperty(const Property& property);
  // Return a Property by ref counting so this can be made thread-safe.  Note
  // that this method bumps the ref count of the underlying instance to a Value,
  // the caller is responsible for freeing it.  Sets |value| to point to a NULL
  // Value and returns |false| if no such property exists.
  bool GetValueForPropertyNamed(const std::string& name,
                                SharedVariant* value) const;
  // Sets the value of the property associated with |name|.  Returns |false|
  // if no such property exists.
  bool SetValueForPropertyNamed(const std::string& name, const Variant& value);


  // Make a copy of the browser binding object by asking the browser to retain
  // it.  Use this for the return value of functions that expect the retain
  // count to increment, such as NPP_GetScriptableInstance().
  NPObject* CopyBrowserBinding();

  // Release the browser binding object.  Note that this *might* cause |this|
  // to get deleted, if the ref count of the browser binding object falls to 0.
  void ReleaseBrowserBinding();

  // Log a message to the browser's console window.  You can usually see this
  // message when using a JavaScript debugger, such as Chrome Developer Tools.
  bool LogToConsole(const std::string& msg) const;

  // Return the browser instance associated with this ScriptingBridge.
  const NPP& GetBrowserInstance() const;

  void set_native_object(SharedScriptableNativeObject native_object) {
    native_object_ = native_object;
  }

  // Accessors.
  const npapi::BrowserBinding* browser_binding() const {
    return browser_binding_;
  }
  SharedScriptableNativeObject native_object() {
    return native_object_;
  }

  // This does not return a const NPObject* because none of the NPAPI that uses
  // this value accepts a const NPObject*.  This will go away with Pepper V2
  // so I don't think it's worth thinking about too hard.
  NPObject* window_object() const;

  // A hidden class that wraps the NPObject, preserving its memory layout
  // for the browser.
  friend class npapi::BrowserBinding;

 private:
  typedef std::map<std::string,
                   SharedMethodCallbackExecutor> MethodDictionary;
  typedef std::map<std::string, Property> PropertyDictionary;

  ScriptingBridge();  // Not implemented, do not use.
  explicit ScriptingBridge(npapi::BrowserBinding* browser_binding);

  // Support for browser-exposed methods.  The browser proxy (a private,
  // platform-specific implementation) invokes a method by first calling
  // HasScriptMethod(), and if that returns |true|, calls InvokeScriptMethod().
  // The browser proxy is responsible for all the variant marshaling from
  // platform-specific types (for example NPVariant or pp::Var) into c_salt
  // Types.
  virtual bool HasScriptMethod(const std::string& name);
  virtual bool InvokeScriptMethod(const std::string& method_name,
                                  const ::c_salt::SharedVariant* params_begin,
                                  const ::c_salt::SharedVariant* params_end,
                                  ::c_salt::SharedVariant* return_value_var);

  // Support for browser-exposed properties.  The browser proxy (which is
  // platform-specific) first calls HasProperty() before getting or setting;
  // the Get or Set is performed only if HasProperty() returns |true|.  The
  // brwoser proxy is responsible for all the variant marshaling.
  virtual bool HasScriptProperty(const std::string& name);
  // Set |return_value| to the value associated with property |name|.  If
  // property |name| doesn't exist, then set |return_value| to the null type
  // and return |false|.
  virtual bool GetScriptProperty(const std::string& name,
                                 SharedVariant* return_value) const;
  // If |name| is associated with a static property, return that value.  Else,
  // if there is no property associated with |name|, add it as a dynamic
  // property.  See property.h for definitions and more details.
  virtual bool SetScriptProperty(const std::string& name,
                                 const SharedVariant& value);
  // This succeeds only if |name| is associated with a dynamic property.
  virtual bool RemoveScriptProperty(const std::string& name);

  // Return the names of all enumerable properties in to the provided vector.
  virtual void GetAllPropertyNames(std::vector<std::string>* prop_names) const;

  // This is called by some browser proxies when all references to a proxy
  // object have been deallocated, but the proxy's ref count has not gone to 0.
  // It's kind of an anti-leak clean-up mechanism.
  virtual void Invalidate();

  virtual bool IsNative() const { return true; }

  // This is a weak reference.  Some kind of smart_ptr would be useful here,
  // but the |browser_binding_| instance is actually a proxy object that is
  // managed by the browser.  In addition, this pointer is actually a circular
  // reference to the BrowserBinding object that owns this ScriptingBridge
  // instance.
  npapi::BrowserBinding* browser_binding_;
  // |window_object_| is mutable so that the const accessor can create it
  // lazily.
  // TODO(dspringer): move this into BrowserBinding.
  mutable NPObject* window_object_;

  MethodDictionary method_dictionary_;
  PropertyDictionary property_dictionary_;
  // ScriptingBridge owns the native object to ensure it stays alive as long as
  // the browser needs it, and it goes away as soon as all clients are done with
  // it.  I.e., ScriptingBridge is ref-counted using the appropriate browser
  // mechanism, and makes sure the target object has the same lifespan.
  SharedScriptableNativeObject native_object_;
};

}  // namespace c_salt

#endif  // C_SALT_SCRIPTING_BRIDGE_H_
