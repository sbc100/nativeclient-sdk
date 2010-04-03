// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "examples/pi_generator/scripting_bridge.h"

#include "examples/pi_generator/pi_generator.h"

namespace pi_generator {

NPIdentifier ScriptingBridge::id_paint;

std::map<NPIdentifier, ScriptingBridge::Method>*
    ScriptingBridge::method_table;

std::map<NPIdentifier, ScriptingBridge::Property>*
    ScriptingBridge::property_table;

bool ScriptingBridge::InitializeIdentifiers() {
  id_paint = NPN_GetStringIdentifier("paint");

  method_table =
    new(std::nothrow) std::map<NPIdentifier, Method>;
  if (method_table == NULL) {
    return false;
  }
  method_table->insert(
    std::pair<NPIdentifier, Method>(id_paint,
                                    &ScriptingBridge::Paint));

  property_table =
    new(std::nothrow) std::map<NPIdentifier, Property>;
  if (property_table == NULL) {
    return false;
  }

  return true;
}

ScriptingBridge::~ScriptingBridge() {
}

bool ScriptingBridge::HasMethod(NPIdentifier name) {
  std::map<NPIdentifier, Method>::iterator i;
  i = method_table->find(name);
  return i != method_table->end();
}

bool ScriptingBridge::HasProperty(NPIdentifier name) {
  std::map<NPIdentifier, Property>::iterator i;
  i = property_table->find(name);
  return i != property_table->end();
}

bool ScriptingBridge::GetProperty(NPIdentifier name, NPVariant *result) {
  VOID_TO_NPVARIANT(*result);

  std::map<NPIdentifier, Property>::iterator i;
  i = property_table->find(name);
  if (i != property_table->end()) {
    return (this->*(i->second))(result);
  }
  return false;
}

bool ScriptingBridge::SetProperty(NPIdentifier name, const NPVariant* value) {
  return false;  // Not implemented.
}

bool ScriptingBridge::RemoveProperty(NPIdentifier name) {
  return false;  // Not implemented.
}

bool ScriptingBridge::InvokeDefault(const NPVariant* args,
                                    uint32_t arg_count,
                                    NPVariant* result) {
  return false;  // Not implemented.
}

bool ScriptingBridge::Invoke(NPIdentifier name,
                             const NPVariant* args, uint32_t arg_count,
                             NPVariant* result) {
  std::map<NPIdentifier, Method>::iterator i;
  i = method_table->find(name);
  if (i != method_table->end()) {
    return (this->*(i->second))(args, arg_count, result);
  }
  return false;
}

void ScriptingBridge::Invalidate() {
  // Not implemented.
}

bool ScriptingBridge::Paint(const NPVariant* args,
                            uint32_t arg_count,
                            NPVariant* result) {
  PiGenerator* pi_generator = static_cast<PiGenerator*>(npp_->pdata);
  if (pi_generator) {
    DOUBLE_TO_NPVARIANT(pi_generator->pi(), *result);
    return pi_generator->Paint();
  }
  return false;
}

// These are the function wrappers that the browser calls.

void Invalidate(NPObject* object) {
  return static_cast<ScriptingBridge*>(object)->Invalidate();
}

bool HasMethod(NPObject* object, NPIdentifier name) {
  return static_cast<ScriptingBridge*>(object)->HasMethod(name);
}

bool Invoke(NPObject* object, NPIdentifier name,
            const NPVariant* args, uint32_t arg_count,
            NPVariant* result) {
  return static_cast<ScriptingBridge*>(object)->Invoke(
      name, args, arg_count, result);
}

bool InvokeDefault(NPObject* object, const NPVariant* args, uint32_t arg_count,
                   NPVariant* result) {
  return static_cast<ScriptingBridge*>(object)->InvokeDefault(
      args, arg_count, result);
}

bool HasProperty(NPObject* object, NPIdentifier name) {
  return static_cast<ScriptingBridge*>(object)->HasProperty(name);
}

bool GetProperty(NPObject* object, NPIdentifier name, NPVariant* result) {
  return static_cast<ScriptingBridge*>(object)->GetProperty(name, result);
}

bool SetProperty(NPObject* object, NPIdentifier name, const NPVariant* value) {
  return static_cast<ScriptingBridge*>(object)->SetProperty(name, value);
}

bool RemoveProperty(NPObject* object, NPIdentifier name) {
  return static_cast<ScriptingBridge*>(object)->RemoveProperty(name);
}

NPObject* Allocate(NPP npp, NPClass* npclass) {
  return new ScriptingBridge(npp);
}

void Deallocate(NPObject* object) {
  delete static_cast<ScriptingBridge*>(object);
}

}  // namespace pi_generator

NPClass pi_generator::ScriptingBridge::np_class = {
  NP_CLASS_STRUCT_VERSION,
  pi_generator::Allocate,
  pi_generator::Deallocate,
  pi_generator::Invalidate,
  pi_generator::HasMethod,
  pi_generator::Invoke,
  pi_generator::InvokeDefault,
  pi_generator::HasProperty,
  pi_generator::GetProperty,
  pi_generator::SetProperty,
  pi_generator::RemoveProperty
};
