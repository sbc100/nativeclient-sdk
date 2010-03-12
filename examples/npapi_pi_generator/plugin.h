/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#ifndef EXAMPLES_NPAPI_PI_GENERATOR_PLUGIN_H_
#define EXAMPLES_NPAPI_PI_GENERATOR_PLUGIN_H_

#include <pthread.h>
#if defined(__native_client__)
#include <nacl/nacl_npapi.h>
#include <nacl/npapi_extensions.h>
#else
// Building a trusted plugin for debugging.
#include "third_party/npapi/bindings/npapi.h"
#include "third_party/npapi/bindings/npapi_extensions.h"
#include "third_party/npapi/bindings/nphostapi.h"
#endif
#include <map>

#include "examples/npapi_pi_generator/base_object.h"

class Plugin {
 public:
  explicit Plugin(NPP npp);
  ~Plugin();

  NPObject* GetScriptableObject();
  NPError SetWindow(NPWindow* window);
  bool Paint();
  bool quit() const {
    return quit_;
  }
  double pi() const {
    return pi_;
  }
  void* pixels() const {
    return context2d_.region;
  }
  int width() const {
    return window_ ? window_->width : 0;
  }
  int height() const {
    return window_ ? window_->height : 0;
  }

 private:
  NPP       npp_;
  NPObject* scriptable_object_;  // strong reference

  NPWindow* window_;
  NPDevice* device2d_;  // The PINPAPI 2D device.
  NPDeviceContext2D context2d_;  // The PINPAPI 2D drawing context.

  bool quit_;
  pthread_t thread_;
  double pi_;

  static void* pi(void* param);
};

class ScriptablePluginObject : public BaseObject {
 public:
  explicit ScriptablePluginObject(NPP npp)
    : npp_(npp) {
  }

  virtual bool HasMethod(NPIdentifier name);
  virtual bool Invoke(NPIdentifier name,
                      const NPVariant* args, uint32_t arg_count,
                      NPVariant* result);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant* result);
  static bool InitializeIdentifiers();

  static NPClass np_class;

 private:
  typedef bool (ScriptablePluginObject::*Method)(const NPVariant* args,
                                                 uint32_t arg_count,
                                                 NPVariant* result);
  typedef bool (ScriptablePluginObject::*Property)(NPVariant* result);


  bool Paint(const NPVariant* args, uint32_t arg_count, NPVariant* result);

  NPP npp_;

  static NPIdentifier id_paint;
  static std::map<NPIdentifier, Method>* method_table;
  static std::map<NPIdentifier, Property>* property_table;
};

#endif  // EXAMPLES_NPAPI_PI_GENERATOR_PLUGIN_H_
