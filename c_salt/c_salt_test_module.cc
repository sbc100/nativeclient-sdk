// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

// This is the connection to the c_salt Module machinery.  The Module singleton
// calls CreateInstance to make new copies of each in-browser instance of
// Ginsu.  This gets linked with all the tests.

#include "c_salt/instance.h"
#include "c_salt/module.h"

class MyInstance : public c_salt::Instance {
 public:
  explicit MyInstance(const NPP& instance) : c_salt::Instance(instance) {}
};

class TestModule : public c_salt::Module {
 public:
  virtual c_salt::Instance* CreateInstance(const NPP& instance) {
    return new MyInstance(instance);
  }
};

namespace c_salt {
Module* CreateModule() {
  return new TestModule();
}
}  // namespace c_salt
