// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "examples/pepper_3d/nacl_module/scripting_bridge.h"

#include <assert.h>

#include "examples/pepper_3d/nacl_module/pepper_3d.h"

namespace pepper_3d {

NPIdentifier ScriptingBridge::id_get_camera_orientation;
NPIdentifier ScriptingBridge::id_set_camera_orientation;

static const uint32_t kQuaternionElementCount = 4;

std::map<NPIdentifier, ScriptingBridge::Method>*
    ScriptingBridge::method_table;

std::map<NPIdentifier, ScriptingBridge::Property>*
    ScriptingBridge::property_table;

bool ScriptingBridge::InitializeIdentifiers() {
  id_get_camera_orientation = NPN_GetStringIdentifier("getCameraOrientation");
  id_set_camera_orientation = NPN_GetStringIdentifier("setCameraOrientation");

  method_table = new(std::nothrow) std::map<NPIdentifier, Method>;
  if (method_table == NULL) {
    return false;
  }
  method_table->insert(
    std::pair<NPIdentifier, Method>(
        id_get_camera_orientation,
        &ScriptingBridge::GetCameraOrientation));
  method_table->insert(
    std::pair<NPIdentifier, Method>(
        id_set_camera_orientation,
        &ScriptingBridge::SetCameraOrientation));

  property_table =
    new(std::nothrow) std::map<NPIdentifier, Property>;
  if (property_table == NULL) {
    return false;
  }

  return true;
}

ScriptingBridge::ScriptingBridge(NPP npp)
    : npp_(npp), window_object_(NULL) {
  NPError error = NPN_GetValue(npp_, NPNVWindowNPObject, &window_object_);
  assert(NPERR_NO_ERROR == error);
}

ScriptingBridge::~ScriptingBridge() {
  if (window_object_) {
    NPN_ReleaseObject(window_object_);
  }
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

bool ScriptingBridge::GetProperty(NPIdentifier name,
                                         NPVariant *result) {
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

bool ScriptingBridge::GetCameraOrientation(const NPVariant* args,
                                           uint32_t arg_count,
                                           NPVariant* result) {
  Pepper3D* pepper_3d = static_cast<Pepper3D*>(npp_->pdata);
  if (pepper_3d && window_object_) {
    float orientation[4];
    if (!pepper_3d->GetCameraOrientation(orientation))
      return false;

    // Initialize the return value.
    NULL_TO_NPVARIANT(*result);
    // Ask the browser to create a new JavaScript array object.
    NPVariant variant;
    NPString npstr;
    npstr.UTF8Characters = "new Array();";
    npstr.UTF8Length = static_cast<uint32>(strlen(npstr.UTF8Characters));
    if (!NPN_Evaluate(npp_, window_object_, &npstr, &variant) ||
        !NPVARIANT_IS_OBJECT(variant)) {
      return false;
    }
    // Set the properties, each array subscript has its own property id.
    NPObject* array_object = NPVARIANT_TO_OBJECT(variant);
    if (array_object) {
      for (size_t j = 0; j < kQuaternionElementCount; ++j) {
        NPVariant array_value;
        DOUBLE_TO_NPVARIANT(static_cast<double>(orientation[j]), array_value);
        NPN_SetProperty(npp_,
                        array_object,
                        NPN_GetIntIdentifier(j),
                        &array_value);
      }
      OBJECT_TO_NPVARIANT(array_object, *result);
    }
    return true;
  }
  return false;
}

bool ScriptingBridge::SetCameraOrientation(const NPVariant* args,
                                           uint32_t arg_count,
                                           NPVariant* value) {
  Pepper3D* pepper_3d = static_cast<Pepper3D*>(npp_->pdata);
  if (!pepper_3d || arg_count != 1 || !NPVARIANT_IS_OBJECT(*args))
    return false;

  // Unpack the array object.  This is done by enumerating the identifiers on
  // the array; the identifiers are the array subscripts.
  bool success = false;
  NPIdentifier* identifier = NULL;
  uint32_t element_count = 0;
  NPObject* array_object = NPVARIANT_TO_OBJECT(*args);
  if (NPN_Enumerate(npp_, array_object, &identifier, &element_count)) {
    if (element_count == kQuaternionElementCount) {
      float orientation[4] = {0.0f, 0.0f, 0.0f, 1.0f};
      pepper_3d->GetCameraOrientation(orientation);
      for (uint32_t j = 0; j < element_count; ++j) {
        if (NPN_HasProperty(npp_, array_object, identifier[j])) {
          // Get each element out of the array by accessing the property whose
          // identifier is the array subscript.
          NPVariant array_elem;
          VOID_TO_NPVARIANT(array_elem);
          if (NPN_GetProperty(npp_,
                              array_object,
                              identifier[j],
                              &array_elem)) {
            // Process both integer and double values.  Other value types are
            // not handled.
            switch (array_elem.type) {
            case NPVariantType_Int32:
              orientation[j] =
                  static_cast<float>(NPVARIANT_TO_INT32(array_elem));
              break;
            case NPVariantType_Double:
              orientation[j] =
                  static_cast<float>(NPVARIANT_TO_DOUBLE(array_elem));
              break;
            default:
              break;
            }
          }
        }
      }
      success = pepper_3d->SetCameraOrientation(orientation);
      NPN_MemFree(identifier);
    }
  }

  return success;
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

}  // namespace pepper_3d

NPClass pepper_3d::ScriptingBridge::np_class = {
  NP_CLASS_STRUCT_VERSION,
  pepper_3d::Allocate,
  pepper_3d::Deallocate,
  pepper_3d::Invalidate,
  pepper_3d::HasMethod,
  pepper_3d::Invoke,
  pepper_3d::InvokeDefault,
  pepper_3d::HasProperty,
  pepper_3d::GetProperty,
  pepper_3d::SetProperty,
  pepper_3d::RemoveProperty
};
