// Copyright (c) 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef C_SALT_INTEGRATION_TESTS_TEST_MODULE_H_
#define C_SALT_INTEGRATION_TESTS_TEST_MODULE_H_

#include <cstdio>

#include "boost/shared_ptr.hpp"
#include "c_salt/integration_tests/fake_instance.h"
#include "c_salt/module.h"

// This is the connection to the c_salt Module machinery.  The Module singleton
// calls CreateInstance to make new copies of each in-browser instance of
// FakeInstance.
class TestModule : public c_salt::Module {
 public:
  virtual c_salt::Instance* CreateInstance(const NPP& npp_instance) {
    std::printf("Creating instance %p\n", npp_instance);
    boost::shared_ptr<FakeInstance> instance_ptr(
        new FakeInstance(npp_instance));
    // The ScriptingBridge takes ownership of FakeInstance via the shared_ptr.
    instance_ptr->CreateScriptingBridgeForObject(instance_ptr);
    return instance_ptr.get();
  }
};

#endif  // C_SALT_INTEGRATION_TESTS_TEST_MODULE_H_
