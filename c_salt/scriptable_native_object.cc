// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/scriptable_native_object.h"

#include <cassert>

namespace c_salt {

  void ScriptableNativeObject::Initialize(SharedScriptingBridge bridge) {
    scripting_bridge_ = bridge;
    this->InitializeMethods(bridge.get());
    this->InitializeProperties(bridge.get());
  }

  WeakScriptingBridge ScriptableNativeObject::GetScriptingBridge() {
    return scripting_bridge_;
  }

}  // namespace c_salt
