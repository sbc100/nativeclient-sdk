// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/module.h"

#include <nacl/npupp.h>
#include <pgl/pgl.h>

// These functions are called when module code is first loaded, and when the
// module code text gets unloaded.  They must use C-style linkage.

// TODO(dspringer): This file will disappear when we migrate to Pepper v2 API.
// It gets replaced by pp::Module.

namespace c_salt {
// This singleton is a pointer to the module as loaded by the browser.  When
// we move to Pepper V2, this singleton is managed by the browser and will
// disappear from this code.
Module* Module::module_singleton_ = NULL;

Module::Module() : is_opengl_initialized_(false) {
}

Module::~Module() {
  // Perform any specialized shut-down procedures here.
  if (is_opengl_initialized_) {
    pglTerminate();
    is_opengl_initialized_ = false;
  }
}

bool Module::InitializeOpenGL() {
  if (is_opengl_initialized_) return true;
  is_opengl_initialized_ = pglInitialize() == PGL_TRUE ? true : false;
  return is_opengl_initialized_;
}

void Module::CleanUp() {
  delete module_singleton_;
}

Module& Module::GetModuleSingleton() {
  if (module_singleton_ == NULL) {
    module_singleton_ = CreateModule();
  }
  // Crash on failure.
  return *module_singleton_;
}
}  // namespace c_salt

// The browser bindings that get called to load and unload a Module.
extern "C" {
NPError NP_GetEntryPoints(NPPluginFuncs* plugin_funcs) {
  // Defined in npp_gate.cc
  extern NPError InitializePepperGateFunctions(NPPluginFuncs* plugin_funcs);
  return InitializePepperGateFunctions(plugin_funcs);
}

NPError NP_Initialize(NPNetscapeFuncs* browser_functions,
                      NPPluginFuncs* plugin_functions) {
  NPError np_err = NP_GetEntryPoints(plugin_functions);
  return np_err;
}

NPError NP_Shutdown() {
  c_salt::Module::CleanUp();
  return NPERR_NO_ERROR;
}
}  // extern "C"
