// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_INSTANCE_H_
#define C_SALT_INSTANCE_H_

#include <nacl/nacl_npapi.h>
#include <nacl/npruntime.h>
#include <nacl/npapi_extensions.h>

#include "boost/noncopyable.hpp"
#include "boost/scoped_ptr.hpp"
#include "c_salt/scriptable_native_object.h"
#include "c_salt/scripting_bridge.h"
#include "c_salt/scripting_bridge_ptrs.h"

namespace c_salt {

// The base class for the Native Client module instance.  An Instance can
// publish a ScriptingBridge to the browser, that ScriptingBridge binds to
// methods and properties declared on this particulare Instance object.  The
// Instance is also responsible for creating new ScriptingBridges as needed.
// Other objects are free to publish their own ScriptingBridges, that bind to
// those objects.
// Repeated calls to GetScriptingBridge() will simply increment
// a ref count of the published ScriptingBridge associated with this Instance.

// TODO(c_salt authors):  Make this API agnostic.  Also maybe don't force it to
// be a ScriptableNativeObject?
class Instance : public boost::noncopyable,
                 public ScriptableNativeObject {
 public:
  explicit Instance(const NPP& npp_instance)
      : is_loaded_(false), npp_instance_(npp_instance) {}
  virtual ~Instance();

  // Called during initialization to publish the module's method names that
  // can be called from JavaScript.
  virtual void InitializeMethods(ScriptingBridge* bridge);

  // Called during initialization to publish the module's properties that can
  // be called from JavaScript.
  virtual void InitializeProperties(ScriptingBridge* bridge);

  // Called the first time this module instance is loaded into the browser
  // document.  When this method is called, all the Pepper devices are
  // available to the module instance.
  virtual bool InstanceDidLoad(int width, int height);

  // Called when there is a valid browser window for rendering, or whenever the
  // in-browser view changes size.
  virtual void WindowDidChangeSize(int width, int height);

  // Receive an event from the browser.  Return |false| if the module does not
  // handle the event.
  virtual bool ReceiveEvent(const NPPepperEvent& event);

  // Create a ScriptingBridge that will expose the object to the browser.  The
  // ScriptingBridge takes ownership of the object.  When a new
  // ScriptingBridge instance is created, both InitializeMethods() and
  // InitializeProperties() are called on the ScriptableNativeObject.
  void CreateScriptingBridgeForObject(
    SharedScriptableNativeObject native_object);

  // Accessor for the in-browser NPAPI instance associated with this Instance.
  const NPP npp_instance() const {
    return npp_instance_;
  }

  // Accessor/mutator for |is_loaded_|.  This is used to determine when to call
  // the InstanceDidLoad() method.
  bool is_loaded() const {
    return is_loaded_;
  }
  void set_is_loaded(bool flag) {
    is_loaded_ = flag;
  }

  // Access to window object in the scripting bridge is necessary for now in
  // order to support NPAPI coding for subclasses.
  NPObject* WindowObject() const {
    return scripting_bridge_->window_object();
  }

 private:
  bool is_loaded_;

  boost::scoped_ptr<ScriptingBridge> scripting_bridge_;

  // TODO(c_salt_authors): this needs to be turned into a BrowserInstanceImpl.
  NPP npp_instance_;

  Instance();  // Not implemented, do not use.
};

}  // namespace c_salt

#endif  // C_SALT_INSTANCE_H_
