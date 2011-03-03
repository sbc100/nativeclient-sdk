// Copyright (c) 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef C_SALT_INTEGRATION_TESTS_METHOD_TESTER_H_
#define C_SALT_INTEGRATION_TESTS_METHOD_TESTER_H_

#include <string>
#include <map>
#include <vector>

#include "c_salt/scriptable_native_object.h"
#include "c_salt/scripting_bridge_ptrs.h"
#include "c_salt/scripting_interface.h"

// A class to test calling methods across the ScriptingBridge.
class MethodTester : public c_salt::ScriptableNativeObject {
 public:
  MethodTester() {}
  virtual ~MethodTester() {}

 private:
  // Methods to implement ScriptableNativeObject:
  virtual void InitializeMethods(c_salt::ScriptingBridge* bridge);
  virtual void InitializeProperties(c_salt::ScriptingBridge* bridge);

  // Methods to expose to JavaScript:

  // Append 2 strings and return the result.
  // Note that the 1st arg is const-ref and the 2nd by-value on purpose, to make
  // sure we exercise both styles of argument passing.
  std::string AppendStrings(const std::string& s1, std::string s2);

  // TODO(dmichael):  Add support for containers so we can do this
  // std::string AppendStringArray(const std::vector<std::string>& s_vec);

  // Add 2 doubles and return the result.
  double AddDoubles(const double& d1, double d2);
  // Add 2 integers and return the result.
  int32_t AddInts(const int32_t& i1, int32_t i2);
  // Return (b1 && b2).
  bool AndBools(const bool& b1, bool b2);

  // This one tests the ability to call methods via the ScriptingInterface.
  // |method_name| is the name of a method to invoke, |script_object| is a
  // ScriptingInterface (could be a JavaScript object or a ScriptingBridge) on
  // which we should invoke the method, and |parameter| is a parameter to
  // pass to the method.  When the method returns, CallMethodOnScriptObject
  // returns that return value back out.
  c_salt::Variant CallMethodOnScriptObject(
      const std::string& method_name,
      boost::shared_ptr<c_salt::ScriptingInterface> script_object,
      const c_salt::SharedVariant& parameter);

  // Call the function that is passed in as |script_object| with |parameter|.
  c_salt::Variant CallAnonymousFunction(
      boost::shared_ptr<c_salt::ScriptingInterface> script_object,
      const c_salt::SharedVariant& parameter);
};

#endif  // C_SALT_INTEGRATION_TESTS_METHOD_TESTER_H_
