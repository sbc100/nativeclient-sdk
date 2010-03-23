// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#if defined (__native_client__)
#include <nacl/npupp.h>
#include <pgl/pgl.h>
#else
#include "third_party/include/pgl/pgl.h"
#include "third_party/npapi/bindings/npapi.h"
#include "third_party/npapi/bindings/nphostapi.h"
#endif

// These functions are required by both the trusted and untrusted loaders,
// they are called when a module instance is first loaded, and when the module
// instance is finally deleted.  They must use C-style linkage.

extern "C" {

NPError NP_GetEntryPoints(NPPluginFuncs* plugin_funcs) {
  extern NPError InitializePluginFunctions(NPPluginFuncs* plugin_funcs);
  return InitializePluginFunctions(plugin_funcs);
}

// Some platforms, including Native Client uses the two-parameter version of
// NP_Initialize(), and do not call NP_GetEntryPoints().  Others (Mac, e.g.)
// use single-parameter version of NP_Initialize(), and then call
// NP_GetEntryPoints() to get the NPP functions.  Also, the NPN entry points are
// defined by the Native Client loader, but are not defined in the trusted
// plugin loader (and must be filled in in NP_Initialize()).
#if defined(__native_client__) || defined(OS_LINUX)
NPError NP_Initialize(NPNetscapeFuncs* browser_funcs,
                      NPPluginFuncs* plugin_funcs) {
  NPError np_err = NP_GetEntryPoints(plugin_funcs);
  if (NPERR_NO_ERROR == np_err)
    pglInitialize();
  return np_err;
}
#elif defined(OS_MACOSX)
NPError NP_Initialize(NPNetscapeFuncs* browser_functions) {
  extern void InitializeBrowserFunctions(NPNetscapeFuncs* browser_functions);
  InitializeBrowserFunctions(browser_functions);
  pglInitialize();
  return NPERR_NO_ERROR;
}
#elif defined(OS_WIN)
NPError NP_Initialize(NPNetscapeFuncs* browser_functions) {
  extern void InitializeBrowserFunctions(NPNetscapeFuncs* browser_functions);
  InitializeBrowserFunctions(browser_functions);
  pglInitialize();
  return NPERR_NO_ERROR;
}
#else
#error "Unrecognized platform"
#endif

NPError NP_Shutdown() {
  pglTerminate();
  return NPERR_NO_ERROR;
}

}  // extern "C"
