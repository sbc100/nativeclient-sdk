// Copyright (c) 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "c_salt/integration_tests/property_tester.h"

#include <cstdio>

#include "c_salt/property.h"
#include "c_salt/scripting_bridge.h"
#include "c_salt/variant.h"
#include "c_salt/variant_ptrs.h"

void PropertyTester::InitializeMethods(c_salt::ScriptingBridge* bridge) {
  // No methods in this class.
}

void PropertyTester::InitializeProperties(c_salt::ScriptingBridge* bridge) {
  c_salt::SharedVariant value(new c_salt::Variant(static_cast<int32_t>(42)));
  c_salt::PropertyAttributes prop1("intProp", value);
  prop1.set_static().set_immutable();
  bridge->AddProperty(c_salt::Property(prop1));

  value.reset(new c_salt::Variant("A string."));
  c_salt::PropertyAttributes prop2("stringProp", value);
  prop2.set_mutable();
  bridge->AddProperty(c_salt::Property(prop2));
}
