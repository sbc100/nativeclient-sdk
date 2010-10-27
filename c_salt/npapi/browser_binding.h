// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_NPAPI_BROWSER_BINDING_H_
#define C_SALT_NPAPI_BROWSER_BINDING_H_

#include <nacl/nacl_npapi.h>
#include <nacl/npruntime.h>

#include <string>

#include "boost/shared_ptr.hpp"
#include "c_salt/instance.h"
#include "c_salt/npapi/scoped_npid_to_string_converter.h"
#include "c_salt/npapi/variant_converter.h"
#include "c_salt/scripting_bridge_ptrs.h"
#include "c_salt/scripting_bridge.h"

namespace c_salt {

namespace npapi {

// A thin wrapper that owns the ScriptingBridge class.  This is necessary
// because the NPObject layout has to be preserved, and it cannot have things
// like a vtable inserted into it.
class BrowserBinding : public NPObject {
 public:
  explicit BrowserBinding(NPP npp)
      : npp_(npp),
        scripting_bridge_(new ScriptingBridge(this)),
        variant_converter_(npp) {}
  // The dtor *cannot* be virtual because this object must preserve NPObject's
  // POD memory layout.
  ~BrowserBinding() {}

  // Factory method to create a browser binding.  This asks the browser to
  // create the proxy object via NPN_CreateObject() that represents this
  // instance.  Calling this causes the browser to call the NPAPI Allocate()
  // function, which then calls the ctor for this class.
  // This is a synchronous call to the browser.  Memory has been allocated
  // and ctors called by the time it returns.
  static BrowserBinding* CreateBrowserBinding(const c_salt::Instance& instance);

  // Bump the retain count of the proxy object in the browser.
  void Retain();

  const NPP& npp() const {
    return npp_;
  }

  SharedScriptingBridge scripting_bridge() {
    return scripting_bridge_;
  }

 private:
  // NPAPI support methods; the browser calls these on scriptable objects.
  bool HasMethod(NPIdentifier name) const;
  void Invalidate();
  bool Invoke(NPIdentifier name,
              const NPVariant* args,
              uint32_t arg_count,
              NPVariant* return_value);
  bool HasProperty(NPIdentifier name) const;
  bool GetProperty(NPIdentifier name, NPVariant* return_value) const;
  bool SetProperty(NPIdentifier name, const NPVariant& np_value);
  bool RemoveProperty(NPIdentifier name);

  // These are the free functions that the browser actually calls.  They are
  // all simple wrappers to the above NPAPI support methods.
  friend void Invalidate(NPObject* object);
  friend bool Invoke(NPObject* object, NPIdentifier name,
                     const NPVariant* args,
                     uint32_t arg_count,
                     NPVariant* return_value);
  friend bool HasMethod(NPObject* object, NPIdentifier name);
  friend bool HasProperty(NPObject* object, NPIdentifier name);
  friend bool GetProperty(NPObject* object,
                          NPIdentifier name,
                          NPVariant* result);
  friend bool SetProperty(NPObject* object,
                          NPIdentifier name,
                          const NPVariant* value);
  friend bool RemoveProperty(NPObject* object, NPIdentifier name);

  NPP npp_;
  SharedScriptingBridge scripting_bridge_;
  VariantConverter variant_converter_;
};

}  // namespace npapi
}  // namespace c_salt

#endif  // C_SALT_NPAPI_BROWSER_BINDING_H_
