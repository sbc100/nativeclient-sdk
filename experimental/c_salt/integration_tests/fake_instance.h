// Copyright (c) 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef C_SALT_INTEGRATION_TESTS_FAKE_INSTANCE_H_
#define C_SALT_INTEGRATION_TESTS_FAKE_INSTANCE_H_

#include "boost/shared_ptr.hpp"
#include "c_salt/instance.h"
#include "c_salt/integration_tests/method_tester.h"
#include "c_salt/integration_tests/property_tester.h"

class FakeInstance : public c_salt::Instance {
 public:
  explicit FakeInstance(const NPP& npp_instance);
  virtual ~FakeInstance();

  // Specialization of c_salt::Instance
  virtual void InitializeMethods(c_salt::ScriptingBridge* bridge);
  virtual void InitializeProperties(c_salt::ScriptingBridge* bridge);

 private:
  FakeInstance();  // Not implemented, do not use.

  // ScriptableNativeObjects for testing c_salt features.
  boost::shared_ptr<MethodTester> method_tester_;
  boost::shared_ptr<PropertyTester> property_tester_;

  // Getters for tester objects.  These are exposed via the ScriptingBridge.
  boost::shared_ptr<MethodTester> GetMethodTester();
  boost::shared_ptr<PropertyTester> GetPropertyTester();
};

#endif  // C_SALT_INTEGRATION_TESTS_FAKE_INSTANCE_H_
