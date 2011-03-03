// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_SCRIPTING_INTERFACE_H_
#define C_SALT_SCRIPTING_INTERFACE_H_

#include <string>
#include <vector>

#include "c_salt/variant_ptrs.h"

namespace c_salt {

// ScriptingInterface represents an interface to any object shared between
// JavaScript and native code.
class ScriptingInterface {
 public:
  virtual ~ScriptingInterface() {}

  virtual bool HasScriptMethod(const std::string& name) = 0;
  virtual bool InvokeScriptMethod(const std::string& method_name,
                                  const SharedVariant* params_begin,
                                  const SharedVariant* params_end,
                                  SharedVariant* return_value_var) = 0;
  // Support for browser-exposed properties.  The browser proxy (which is
  // platform-specific) first calls HasProperty() before getting or setting;
  // the Get or Set is performed only if HasProperty() returns |true|.  The
  // brwoser proxy is responsible for all the variant marshaling.
  virtual bool HasScriptProperty(const std::string& name) = 0;
  // Set |return_value| to the value associated with property |name|.  If
  // property |name| doesn't exist, then set |return_value| to the null type
  // and return |false|.
  virtual bool GetScriptProperty(const std::string& name,
                                 SharedVariant* return_value) const = 0;
  // If |name| is associated with a static property, return that value.  Else,
  // if there is no property associated with |name|, add it as a dynamic
  // property.  See property.h for definitions and more details.
  virtual bool SetScriptProperty(const std::string& name,
                                 const SharedVariant& value) = 0;
  // This succeeds only if |name| is associated with a dynamic property.
  virtual bool RemoveScriptProperty(const std::string& name) = 0;

  // Return the names of all enumerable properties in to the provided vector.
  virtual void GetAllPropertyNames(
      std::vector<std::string>* prop_names) const = 0;

  // Return true iff this object is implemented by a native object (as opposed
  // to a JavaScript object in the browser).
  virtual bool IsNative() const = 0;

  // This is called by some browser proxies when all references to a proxy
  // object have been deallocated, but the proxy's ref count has not gone to 0.
  // It's kind of an anti-leak clean-up mechanism.
  virtual void Invalidate() = 0;
};

}  // namespace c_salt

#endif  // C_SALT_SCRIPTING_INTERFACE_H_
