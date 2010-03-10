/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#ifndef EXAMPLES_NPAPI_PI_GENERATOR_PLUGIN_H_
#define EXAMPLES_NPAPI_PI_GENERATOR_PLUGIN_H_

#include <pthread.h>
#include <nacl/nacl_npapi.h>
#include <map>

#include "npapi_pi_generator/base_object.h"

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

 private:
  NPP       npp_;
  NPObject* scriptable_object_;  // strong reference

  NPWindow* window_;
  void* bitmap_data_;  // strong reference
  size_t bitmap_size_;

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
