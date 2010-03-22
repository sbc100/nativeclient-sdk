/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#if defined (__native_client__)
#include <nacl/npapi_extensions.h>
#include <nacl/npupp.h>
#else
// Building a trusted plugin for debugging.
#include "third_party/npapi/bindings/npapi_extensions.h"
#include "third_party/npapi/bindings/nphostapi.h"
#endif
#include <new>

#include "examples/npapi_pi_generator/plugin.h"

#if !defined(__native_client__)
// The trusted plugin needs to call through to the browser directly.  These
// wrapper routines are not required whne making a Native Client module.

static NPNetscapeFuncs kBrowserFuncs = { 0 };

NPError NPN_GetValue(NPP instance, NPNVariable variable, void* value) {
  return kBrowserFuncs.getvalue(instance, variable, value);
}

// Returns an opaque identifier for the string that is passed in.
NPIdentifier NPN_GetStringIdentifier(const NPUTF8* name) {
  return kBrowserFuncs.getstringidentifier(name);
}

NPUTF8* NPN_UTF8FromIdentifier(NPIdentifier identifier) {
  return kBrowserFuncs.utf8fromidentifier(identifier);
}

void* NPN_MemAlloc(uint32 size) {
  return kBrowserFuncs.memalloc(size);
}

void NPN_MemFree(void* mem) {
  kBrowserFuncs.memfree(mem);
}

NPObject* NPN_CreateObject(NPP npp, NPClass* np_class) {
  return kBrowserFuncs.createobject(npp, np_class);
}

NPObject* NPN_RetainObject(NPObject* obj) {
  return kBrowserFuncs.retainobject(obj);
}

void NPN_ReleaseObject(NPObject* obj) {
  kBrowserFuncs.releaseobject(obj);
}

#endif

// PINPAPI extensions.  These get filled in when NPP_New is called.
static NPExtensions* kPINPAPIExtensions = NULL;

// These are PINPAPI extensions.
NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device) {
  return kPINPAPIExtensions ?
      kPINPAPIExtensions->acquireDevice(instance, device) : NULL;
}

// Please refer to the Gecko Plugin API Reference for the description of
// NPP_New.
NPError NPP_New(NPMIMEType mime_type,
                NPP instance,
                uint16_t mode,
                int16_t argc,
                char* argn[],
                char* argv[],
                NPSavedData* saved) {
  if (instance == NULL) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  // Grab the PINPAPI extensions.
  NPN_GetValue(instance, NPNVPepperExtensions,
               reinterpret_cast<void*>(&kPINPAPIExtensions));
  assert(NULL != kPINPAPIExtensions);

  Plugin* plugin = new(std::nothrow) Plugin(instance);
  if (plugin == NULL) {
    return NPERR_OUT_OF_MEMORY_ERROR;
  }

  instance->pdata = plugin;
  return NPERR_NO_ERROR;
}

// Please refer to the Gecko Plugin API Reference for the description of
// NPP_Destroy.
// In the NaCl module, NPP_Destroy is called from NaClNP_MainLoop().
NPError NPP_Destroy(NPP instance, NPSavedData** save) {
  if (instance == NULL) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  Plugin* plugin = static_cast<Plugin*>(instance->pdata);
  if (plugin != NULL) {
    delete plugin;
  }
  return NPERR_NO_ERROR;
}

// NPP_GetScriptableInstance retruns the NPObject pointer that corresponds to
// NPPVpluginScriptableNPObject queried by NPP_GetValue() from the browser.
NPObject* NPP_GetScriptableInstance(NPP instance) {
  if (instance == NULL) {
    return NULL;
  }

  NPObject* object = NULL;
  Plugin* plugin = static_cast<Plugin*>(instance->pdata);
  if (plugin) {
    object = plugin->GetScriptableObject();
  }
  return object;
}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
  if (NPPVpluginScriptableNPObject == variable) {
    NPObject* scriptable_object = NPP_GetScriptableInstance(instance);
    if (scriptable_object == NULL)
      return NPERR_INVALID_INSTANCE_ERROR;
    *reinterpret_cast<NPObject**>(value) = scriptable_object;
    return NPERR_NO_ERROR;
  }
  return NPERR_INVALID_PARAM;
}

int16_t NPP_HandleEvent(NPP instance, void* event) {
  return 0;
}

NPError NPP_SetWindow(NPP instance, NPWindow* window) {
  if (instance == NULL) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }
  if (window == NULL) {
    return NPERR_GENERIC_ERROR;
  }
  Plugin* plugin = static_cast<Plugin*>(instance->pdata);
  if (plugin != NULL) {
    return plugin->SetWindow(window);
  }
  return NPERR_GENERIC_ERROR;
}

// These functions are needed for both the trusted and untrused loader.  They
// must use C-style linkage.

extern "C" {

NPError NP_GetEntryPoints(NPPluginFuncs* plugin_funcs) {
  memset(plugin_funcs, 0, sizeof(*plugin_funcs));
  plugin_funcs->version = 11;
  plugin_funcs->size = sizeof(*plugin_funcs);
  plugin_funcs->newp = NPP_New;
  plugin_funcs->destroy = NPP_Destroy;
  plugin_funcs->setwindow = NPP_SetWindow;
  plugin_funcs->event = NPP_HandleEvent;
  plugin_funcs->getvalue = NPP_GetValue;
  return NPERR_NO_ERROR;
}

// Some platforms, including Native Client, use the two-parameter version of
// NP_Initialize(), and do not call NP_GetEntryPoints().  Others (Mac, e.g.)
// use single-parameter version of NP_Initialize(), and then call
// NP_GetEntryPoints() to get the NPP functions.  Also, the NPN entry points are
// defined by the Native Client loader, but are not defined in the trusted
// plugin loader (and must be filled in in NP_Initialize()).
#if defined(__native_client__)
NPError NP_Initialize(NPNetscapeFuncs* browser_funcs,
                      NPPluginFuncs* plugin_funcs) {
  return NP_GetEntryPoints(plugin_funcs);
}
#else
NPError NP_Initialize(NPNetscapeFuncs* browser_funcs) {
  memcpy(&kBrowserFuncs, browser_funcs, sizeof(kBrowserFuncs));
  return NPERR_NO_ERROR;
}
#endif

NPError NP_Shutdown() {
  return NPERR_NO_ERROR;
}

}  // extern "C"
