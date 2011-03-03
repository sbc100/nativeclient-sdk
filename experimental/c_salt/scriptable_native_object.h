// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_SCRIPTABLE_NATIVE_OBJECT_H_
#define C_SALT_SCRIPTABLE_NATIVE_OBJECT_H_

#include "c_salt/scripting_bridge_ptrs.h"

namespace c_salt {

// The base class for C++ classes which are Scriptable, where Scriptable means
// they can be accessed from JavaScript.  Scriptable C++ classes expose their
// methods and properties to JavaScript via ScriptingBridge.
class ScriptableNativeObject {
 public:
  ScriptableNativeObject() {}
  virtual ~ScriptableNativeObject() {}

  // Initialize the ScriptableNativeObject.  This invokes appropriate virtual
  // functions on the child class to give it an opportunity to register methods
  // and properties with the ScriptingBridge.
  void Initialize(SharedScriptingBridge bridge);

  // Return the ScriptingBridge which is associated with this object.
  WeakScriptingBridge GetScriptingBridge();


 private:
  // Called during initialization to publish the module's method names that
  // can be called from JavaScript.
  virtual void InitializeMethods(ScriptingBridge* bridge) = 0;

  // Called during initialization to publish the module's properties that can
  // be called from JavaScript.
  virtual void InitializeProperties(ScriptingBridge* bridge) = 0;

 private:
  // Copy and assign are unsupported and therefore not implemented.
  ScriptableNativeObject(const ScriptableNativeObject&);
  ScriptableNativeObject& operator=(const ScriptableNativeObject&);
  WeakScriptingBridge scripting_bridge_;
};

}  // namespace c_salt

#endif  // C_SALT_SCRIPTABLE_NATIVE_OBJECT_H_
