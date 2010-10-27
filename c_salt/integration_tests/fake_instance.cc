// Copyright (c) 2010 The FakeInstance Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "c_salt/integration_tests/fake_instance.h"

FakeInstance::FakeInstance(const NPP& npp_instance)
    : c_salt::Instance(npp_instance),
      method_tester_(new MethodTester()),
      property_tester_(new PropertyTester()) {
  this->CreateScriptingBridgeForObject(method_tester_);
  this->CreateScriptingBridgeForObject(property_tester_);
}

FakeInstance::~FakeInstance() {
}

void FakeInstance::InitializeMethods(c_salt::ScriptingBridge* bridge) {
  bridge->AddMethodNamed("getMethodTester",
                         this,
                         &FakeInstance::GetMethodTester);
  bridge->AddMethodNamed("getPropertyTester",
                         this,
                         &FakeInstance::GetPropertyTester);
}

void FakeInstance::InitializeProperties(c_salt::ScriptingBridge* bridge) {
  // No properties (yet)
}

boost::shared_ptr<MethodTester> FakeInstance::GetMethodTester() {
  return method_tester_;
}

boost::shared_ptr<PropertyTester> FakeInstance::GetPropertyTester() {
  return property_tester_;
}
