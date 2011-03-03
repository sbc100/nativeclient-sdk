// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_NPAPI_JAVASCRIPT_OBJECT_PROXY_H_
#define C_SALT_NPAPI_JAVASCRIPT_OBJECT_PROXY_H_

#include <nacl/npruntime.h>

#include <string>
#include <vector>

#include "c_salt/instance.h"
#include "c_salt/npapi/variant_converter.h"
#include "c_salt/scripting_interface.h"
#include "c_salt/variant_ptrs.h"

namespace c_salt {
namespace npapi {

// npapi::JavaScriptObjectProxy is a proxy to allow us to communicate with
// JavaScript objects via NPAPI.
class JavaScriptObjectProxy : public ScriptingInterface {
 public:
  JavaScriptObjectProxy(NPObject* object, NPP instance);
  virtual ~JavaScriptObjectProxy();

  virtual bool HasScriptMethod(const std::string& name);
  virtual bool InvokeScriptMethod(const std::string& method_name,
                                  const SharedVariant* params_begin,
                                  const SharedVariant* params_end,
                                  SharedVariant* return_value_var);
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

  virtual bool IsNative() const { return false; }

  // Returns true iff the object is valid (i.e., instance_ and np_object_ are
  // non-null).
  bool Valid() const;

  NPObject* np_object() {
    return np_object_;
  }
  NPP instance() {
    return instance_;
  }

 private:
  NPP instance_;
  NPObject* np_object_;
  VariantConverter variant_converter_;
};

}  // namespace npapi
}  // namespace c_salt

#endif  // C_SALT_NPAPI_JAVASCRIPT_OBJECT_PROXY_H_
