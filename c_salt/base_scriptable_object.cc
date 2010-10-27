// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
#include <algorithm>

#include "c_salt/base_scriptable_object.h"

namespace c_salt {
BaseScriptableObject::BaseScriptableObject(const NPP& npp) : npp_(npp) {
  // All ginsu objects exposed to the browser responds to key-value coding.
  RegisterMethod("getValueForKey", this, &BaseScriptableObject::GetValueForKey);
  RegisterMethod("setValueForKey", this, &BaseScriptableObject::SetValueForKey);
}

BaseScriptableObject::~BaseScriptableObject() {
}

void BaseScriptableObject::Invalidate(NPObject* obj) {
  InvalidateImpl(obj);
}

// Invoke a method by name from the dispatch table.
bool BaseScriptableObject::Invoke(NPIdentifier name, const NPVariant* args,
                                  uint32_t arg_count, NPVariant* result) {
  bool rv = false;
  const std::string method_name = NPN_UTF8FromIdentifier(name);
  std::map<const std::string, MethodHandle>::const_iterator it =
      method_table_.find(method_name);
  if (it != method_table_.end()) {
    MethodHandle obj_method = it->second;
    rv = obj_method->operator()(args, arg_count, result);
  }
  return rv;
}

// Look up a method by name from the dispatch table.
bool BaseScriptableObject::HasMethod(NPIdentifier name) const {
  bool rv = false;
  const std::string method_name = NPN_UTF8FromIdentifier(name);
  rv = method_table_.find(method_name) !=
      method_table_.end();
  return rv;
}

// Look up a property from the dispatch table.
bool BaseScriptableObject::HasProperty(NPIdentifier name) const {
  return property_table_.find(NPN_UTF8FromIdentifier(name)) !=
      property_table_.end();
}

// Retrieve a property value by name using the appropriate getter
// from the dispatch table.
bool BaseScriptableObject::GetProperty(NPIdentifier name,
                                       NPVariant* value) const {
  bool rv = false;
  const std::map<const std::string, PropertyGetterHandle>::const_iterator it =
      property_table_.find(NPN_UTF8FromIdentifier(name));
  if (it != property_table_.end()) {
    PropertyGetterHandle property_getter = it->second;
    rv = property_getter->operator()(value);
  }
  return rv;
}

// Assign a property value by name using the appropriate setter
// from the dispatch table.
bool BaseScriptableObject::SetProperty(NPIdentifier name,
                                       const NPVariant* value) {
  bool rv = false;
  const std::map<const std::string, PropertySetterHandle>::iterator it =
      set_property_table_.find(NPN_UTF8FromIdentifier(name));
  if (it != set_property_table_.end()) {
    PropertySetterHandle property_setter = it->second;
    rv = property_setter->operator()(value);
  }
  return rv;
}

// Handler for "getValueForKey" js method.
bool BaseScriptableObject::GetValueForKey(const NPVariant* args,
                                          uint32_t arg_count,
                                          NPVariant* result) {
  bool rv = false;
  if (1 == arg_count) {
    const NPVariant& var = args[0];
    if (NPVARIANT_IS_STRING(var)) {
      const NPString& np_string = NPVARIANT_TO_STRING(var);
      std::string key;
      key.resize(np_string.UTF8Length);
      std::transform(np_string.UTF8Characters,
                     np_string.UTF8Characters + np_string.UTF8Length,
                     key.begin(),
                     ::tolower);
      rv = GetValueForKeyImpl(key, result);
    }
  }
  return rv;
}

// Handler for "setValueForKey" js method.
bool BaseScriptableObject::SetValueForKey(const NPVariant* args,
                                          uint32_t arg_count,
                                          NPVariant* result) {
  bool rv = false;
  if (2 == arg_count) {
    const NPVariant& var = args[0];
    if (NPVARIANT_IS_STRING(var)) {
      const NPString& np_string = NPVARIANT_TO_STRING(var);
      std::string key;
      key.resize(np_string.UTF8Length);
      std::transform(np_string.UTF8Characters,
                     np_string.UTF8Characters + np_string.UTF8Length,
                     key.begin(),
                     ::tolower);
      rv = SetValueForKeyImpl(key, args[1]);
    }
  }
  return rv;
}
}  // namespace c_salt
