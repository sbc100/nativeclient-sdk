// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_TUMBLER_SCRIPTING_BRIDGE_H_
#define EXAMPLES_TUMBLER_SCRIPTING_BRIDGE_H_

#include <boost/shared_ptr.hpp>
#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/var.h>

#include <map>
#include <string>
#include <vector>

#include "examples/tumbler/callback.h"

namespace tumbler {

class PropertyAccessorCallbackExecutor;
class PropertyMutatorCallbackExecutor;
class MethodCallbackExecutor;

// This class represents the Tumbler application object that gets exposed to
// the browser code.
class ScriptingBridge : public pp::deprecated::ScriptableObject {
 public:
  // Shared pointer types used in the method and property maps.
  typedef boost::shared_ptr<MethodCallbackExecutor>
      SharedMethodCallbackExecutor;
  typedef boost::shared_ptr<PropertyAccessorCallbackExecutor>
      SharedPropertyAccessorCallbackExecutor;
  typedef boost::shared_ptr<PropertyMutatorCallbackExecutor>
      SharedPropertyMutatorCallbackExecutor;

  virtual ~ScriptingBridge() {}

  /// Called by the browser to decide whether @a method is provided by this
  /// plugin's scriptable interface.
  /// @param[in] method The name of the method
  /// @param[out] exception A pointer to an exception.  May be used to notify
  ///     the browser if an exception occurs.
  /// @return true iff @a method is one of the exposed method names.
  virtual bool HasMethod(const pp::Var& method, pp::Var* exception);
  virtual bool HasProperty(const pp::Var& name, pp::Var* exception);
  virtual pp::Var GetProperty(const pp::Var& name, pp::Var* exception);
  virtual void SetProperty(const pp::Var& name,
                           const pp::Var& value,
                           pp::Var* exception);

  /// Invoke the function associated with @a method.  The argument list passed
  /// in via JavaScript is marshalled into a vector of pp::Vars.  None of the
  /// functions in this example take arguments, so this vector is always empty.
  /// @param[in] method The name of the method to be invoked.
  /// @param[in] args The arguments to be passed to the method.
  /// @param[out] exception A pointer to an exception.  May be used to notify
  ///     the browser if an exception occurs.
  /// @return true iff @a method was called successfully.
  virtual pp::Var Call(const pp::Var& method,
                       const std::vector<pp::Var>& args,
                       pp::Var* exception);

  // Causes |method_name| to be published as a method that can be called by
  // JavaScript.  Associated this method with |method|.
  bool AddMethodNamed(const std::string& method_name,
                      SharedMethodCallbackExecutor method);

  // Associate property accessor and mutator with |property_name|.  This
  // publishes |property_name| to the JavaScript.  |get_property| must not
  // be NULL; if |set_property| is NULL the property is considered read-only.
  bool AddPropertyNamed(const std::string& property_name,
      SharedPropertyAccessorCallbackExecutor property_accessor,
      SharedPropertyMutatorCallbackExecutor property_mutator);

 private:
  typedef std::map<std::string, SharedMethodCallbackExecutor> MethodDictionary;
  typedef std::map<std::string, SharedPropertyAccessorCallbackExecutor>
      PropertyAccessorDictionary;
  typedef std::map<std::string, SharedPropertyMutatorCallbackExecutor>
      PropertyMutatorDictionary;

  MethodDictionary method_dictionary_;
  PropertyAccessorDictionary property_accessor_dictionary_;
  PropertyMutatorDictionary property_mutator_dictionary_;
};

}  // namespace tumbler
#endif  // EXAMPLES_TUMBLER_SCRIPTING_BRIDGE_H_
