// Copyright (c) 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "c_salt/integration_tests/method_tester.h"

#include <cstdio>

#include "c_salt/scripting_bridge.h"

void MethodTester::InitializeMethods(c_salt::ScriptingBridge* bridge) {
  bridge->AddMethodNamed("appendStrings",
                         this,
                         &MethodTester::AppendStrings);
  bridge->AddMethodNamed("addDoubles",
                         this,
                         &MethodTester::AddDoubles);
  bridge->AddMethodNamed("addInts",
                         this,
                         &MethodTester::AddInts);
  bridge->AddMethodNamed("andBools",
                         this,
                         &MethodTester::AndBools);
  bridge->AddMethodNamed("callMethodOnScriptObject",
                         this,
                         &MethodTester::CallMethodOnScriptObject);
  bridge->AddMethodNamed("callAnonymousFunction",
                         this,
                         &MethodTester::CallAnonymousFunction);
}

void MethodTester::InitializeProperties(c_salt::ScriptingBridge* bridge) {
  // No properties in this class.
}

std::string MethodTester::AppendStrings(const std::string& s1,
                                              std::string s2) {
  return s1 + s2;
}

double MethodTester::AddDoubles(const double& d1, double d2) {
  return d1 + d2;
}

int32_t MethodTester::AddInts(const int32_t& i1, int32_t i2) {
  return i1 + i2;
}

bool MethodTester::AndBools(const bool& b1, bool b2) {
  return b1 && b2;
}

// TODO(dmichael):  We could easily support returning boost::shared_ptr<Variant>
//                  too.  For now, we just return by-value.
c_salt::Variant MethodTester::CallMethodOnScriptObject(
    const std::string& method_name,
    boost::shared_ptr<c_salt::ScriptingInterface> script_object,
    const c_salt::SharedVariant& parameter) {
  c_salt::SharedVariant return_value;
  if (script_object->HasScriptMethod(method_name)) {
    std::printf("Invoking method %s, param is %s\n",
                method_name.c_str(),
                c_salt::Variant(script_object).StringValue().c_str());
    script_object->InvokeScriptMethod(method_name,
                                      &parameter,
                                      (&parameter) + 1u,
                                      &return_value);
  }
  return *return_value;
}

c_salt::Variant MethodTester::CallAnonymousFunction(
      boost::shared_ptr<c_salt::ScriptingInterface> script_object,
      const c_salt::SharedVariant& parameter) {
  c_salt::SharedVariant return_value;
  script_object->InvokeScriptMethod("",
                                    &parameter,
                                    &parameter + 1u,
                                    &return_value);
  return *return_value;
}
