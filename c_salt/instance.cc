// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// TODO(c_salt authors):  Make this API agnostic.  Also maybe don't force it to
// be a ScriptableNativeObjectInterface.

#include "c_salt/instance.h"

#include "c_salt/npapi/browser_binding.h"

namespace c_salt {

Instance::~Instance() {
  set_is_loaded(false);
}

void Instance::InitializeMethods(ScriptingBridge* bridge) {
}

void Instance::InitializeProperties(ScriptingBridge* bridge) {
}

bool Instance::InstanceDidLoad(int width, int height) {
  return true;
}

void Instance::WindowDidChangeSize(int width, int height) {
}

bool Instance::ReceiveEvent(const NPPepperEvent& event) {
  return false;
}

void Instance::CreateScriptingBridgeForObject(
    SharedScriptableNativeObject native_object) {
  // Create the browser_binding.  This is a synchronous call to the Browser.
  c_salt::npapi::BrowserBinding* browser_binding =
      c_salt::npapi::BrowserBinding::CreateBrowserBinding(*this);
  if (browser_binding) {
    SharedScriptingBridge bridge(browser_binding->scripting_bridge());
    // Tell the ScriptingBridge the object for which it is the bridge.
    bridge->set_native_object(native_object);
    // And initialize the native object with the bridge.  This initializes
    // methods and properties and sets the ScriptableNativeObject up with a weak
    // reference back to the ScriptingBridge.
    native_object->Initialize(bridge);
  }
}

}  // namespace c_salt
