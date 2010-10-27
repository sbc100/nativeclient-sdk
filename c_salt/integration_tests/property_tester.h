// Copyright (c) 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef C_SALT_INTEGRATION_TESTS_PROPERTY_TESTER_H_
#define C_SALT_INTEGRATION_TESTS_PROPERTY_TESTER_H_

#include "c_salt/scriptable_native_object.h"
#include "c_salt/scripting_bridge_ptrs.h"

// A class to test calling methods across the ScriptingBridge.
class PropertyTester : public c_salt::ScriptableNativeObject {
 public:
  PropertyTester() {}
  virtual ~PropertyTester() {}

 private:
  // Methods to implement ScriptableNativeObject:
  virtual void InitializeMethods(c_salt::ScriptingBridge* bridge);
  virtual void InitializeProperties(c_salt::ScriptingBridge* bridge);
};

#endif  // C_SALT_INTEGRATION_TESTS_PROPERTY_TESTER_H_
