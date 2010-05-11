// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "examples/sine_synth/scripting_bridge.h"

#include <math.h>
#include <stdlib.h>

#include "examples/sine_synth/sine_synth.h"

namespace sine_synth {

NPIdentifier ScriptingBridge::id_play_sound;
NPIdentifier ScriptingBridge::id_stop_sound;
NPIdentifier ScriptingBridge::id_frequency;

std::map<NPIdentifier, ScriptingBridge::MethodSelector>*
    ScriptingBridge::method_table;
std::map<NPIdentifier, ScriptingBridge::GetPropertySelector>*
    ScriptingBridge::get_property_table;
std::map<NPIdentifier, ScriptingBridge::SetPropertySelector>*
    ScriptingBridge::set_property_table;

bool ScriptingBridge::InitializeIdentifiers() {
  id_play_sound = NPN_GetStringIdentifier("playSound");
  id_stop_sound = NPN_GetStringIdentifier("stopSound");
  id_frequency = NPN_GetStringIdentifier("frequency");

  method_table =
    new(std::nothrow) std::map<NPIdentifier, MethodSelector>;
  if (method_table == NULL) {
    return false;
  }
  method_table->insert(
    std::pair<NPIdentifier, MethodSelector>(id_play_sound,
                                            &ScriptingBridge::PlaySound));
  method_table->insert(
    std::pair<NPIdentifier, MethodSelector>(id_stop_sound,
                                            &ScriptingBridge::StopSound));

  get_property_table =
    new(std::nothrow) std::map<NPIdentifier, GetPropertySelector>;
  if (get_property_table == NULL) {
    return false;
  }
  set_property_table =
    new(std::nothrow) std::map<NPIdentifier, SetPropertySelector>;
  if (set_property_table == NULL) {
    return false;
  }
  get_property_table->insert(
    std::pair<NPIdentifier, GetPropertySelector>(id_frequency,
                                                 &ScriptingBridge::GetFrequency));
  set_property_table->insert(
    std::pair<NPIdentifier, SetPropertySelector>(id_frequency,
                                                 &ScriptingBridge::SetFrequency));

  return true;
}

ScriptingBridge::~ScriptingBridge() {
}

bool ScriptingBridge::HasMethod(NPIdentifier name) {
  std::map<NPIdentifier, MethodSelector>::iterator i;
  i = method_table->find(name);
  return i != method_table->end();
}

bool ScriptingBridge::HasProperty(NPIdentifier name) {
  std::map<NPIdentifier, GetPropertySelector>::iterator i;
  i = get_property_table->find(name);
  return i != get_property_table->end();
}

bool ScriptingBridge::GetProperty(NPIdentifier name, NPVariant *value) {
  VOID_TO_NPVARIANT(*value);
  std::map<NPIdentifier, GetPropertySelector>::iterator i;
  i = get_property_table->find(name);
  if (i != get_property_table->end()) {
    return (this->*(i->second))(value);
  }
  return false;
}

bool ScriptingBridge::SetProperty(NPIdentifier name, const NPVariant* value) {
  std::map<NPIdentifier, SetPropertySelector>::iterator i;
  i = set_property_table->find(name);
  if (i != set_property_table->end()) {
    return (this->*(i->second))(value);
  }
  return false;
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
  std::map<NPIdentifier, MethodSelector>::iterator i;
  i = method_table->find(name);
  if (i != method_table->end()) {
    return (this->*(i->second))(args, arg_count, result);
  }
  return false;
}

void ScriptingBridge::Invalidate() {
  // Not implemented.
}

bool ScriptingBridge::PlaySound(const NPVariant* args,
                                uint32_t arg_count,
                                NPVariant* result) {
  SineSynth* sine_synth = static_cast<SineSynth*>(npp_->pdata);
  if (sine_synth) {
    return sine_synth->PlaySound();
  }
  return false;
}

bool ScriptingBridge::StopSound(const NPVariant* args,
                                uint32_t arg_count,
                                NPVariant* result) {
  SineSynth* sine_synth = static_cast<SineSynth*>(npp_->pdata);
  if (sine_synth) {
    return sine_synth->StopSound();
  }
  return false;
}

bool ScriptingBridge::GetFrequency(NPVariant* value) {
  SineSynth* sine_synth = static_cast<SineSynth*>(npp_->pdata);
  if (sine_synth) {
    INT32_TO_NPVARIANT(sine_synth->frequency(), *value);
    return true;
  }
  VOID_TO_NPVARIANT(*value);
  return false;
}

bool ScriptingBridge::SetFrequency(const NPVariant* value) {
  SineSynth* sine_synth = static_cast<SineSynth*>(npp_->pdata);
  if (!sine_synth)
    return false;
  int32 freq = -1;
  switch (value->type) {
  case NPVariantType_Int32:
    freq = NPVARIANT_TO_INT32(*value);
    break;
  case NPVariantType_Double:
    freq = static_cast<int32>(floor(NPVARIANT_TO_DOUBLE(*value) + 0.5));
    break;
  case NPVariantType_String:
    freq = atol(value->value.stringValue.UTF8Characters);
    break;
  default:
    break;
  }
  if (freq >= 0) {
    sine_synth->set_frequency(freq);
    return true;
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

}  // namespace sine_synth

NPClass sine_synth::ScriptingBridge::np_class = {
  NP_CLASS_STRUCT_VERSION,
  sine_synth::Allocate,
  sine_synth::Deallocate,
  sine_synth::Invalidate,
  sine_synth::HasMethod,
  sine_synth::Invoke,
  sine_synth::InvokeDefault,
  sine_synth::HasProperty,
  sine_synth::GetProperty,
  sine_synth::SetProperty,
  sine_synth::RemoveProperty
};
