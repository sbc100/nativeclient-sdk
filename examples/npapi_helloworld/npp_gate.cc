/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__native_client__)
#include <nacl/nacl_npapi.h>
#include <nacl/npupp.h>
#else
// Building a trusted plugin for debugging.
#include "third_party/npapi/bindings/npapi.h"
#include "third_party/npapi/bindings/nphostapi.h"

// The trusted plugin needs to call through to the browser directly.  These
// wrapper routines are not required whne making a Native Client module.

static NPNetscapeFuncs kBrowserFuncs = { 0 };

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

NPError NPN_GetValue(NPP instance, NPNVariable variable, void* value) {
  return kBrowserFuncs.getvalue(instance, variable, value);
}

#endif

struct PlugIn {
  NPP npp;
  NPObject *npobject;
};

/*
 * Please refer to the Gecko Plugin API Reference for the description of
 * NPP_New.
 */
NPError NPP_New(NPMIMEType mime_type,
                NPP instance,
                uint16_t mode,
                int16_t argc,
                char* argn[],
                char* argv[],
                NPSavedData* saved) {
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  struct PlugIn *plugin = NULL;
  plugin = new PlugIn;
  plugin->npp = instance;
  plugin->npobject = NULL;

  instance->pdata = plugin;
  return NPERR_NO_ERROR;
}

/*
 * Please refer to the Gecko Plugin API Reference for the description of
 * NPP_Destroy.
 * In the NaCl module, NPP_Destroy is called from NaClNP_MainLoop().
 */
NPError NPP_Destroy(NPP instance, NPSavedData** save) {
  if (NULL == instance)
    return NPERR_INVALID_INSTANCE_ERROR;

  // free plugin
  if (NULL != instance->pdata) {
    PlugIn* plugin = static_cast<PlugIn*>(instance->pdata);
    delete plugin;
    instance->pdata = NULL;
  }
  return NPERR_NO_ERROR;
}

NPObject *NPP_GetScriptableInstance(NPP instance) {
  struct PlugIn* plugin;

  extern NPClass *GetNPSimpleClass();

  if (NULL == instance) {
    return NULL;
  }
  plugin = (struct PlugIn *)instance->pdata;
  if (NULL == plugin->npobject) {
    plugin->npobject = NPN_CreateObject(instance, GetNPSimpleClass());
  }
  if (NULL != plugin->npobject) {
    NPN_RetainObject(plugin->npobject);
  }
  return plugin->npobject;
}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void* ret_value) {
  if (NPPVpluginScriptableNPObject == variable) {
    void** v = reinterpret_cast<void**>(ret_value);
    *v = NPP_GetScriptableInstance(instance);
    return NPERR_NO_ERROR;
  } else {
    return NPERR_GENERIC_ERROR;
  }
}

NPError NPP_SetWindow(NPP instance, NPWindow* window) {
  return NPERR_NO_ERROR;
}

/*
 * NP_Initialize
 */

NPError NP_Initialize(NPNetscapeFuncs* browser_funcs,
                      NPPluginFuncs* plugin_funcs) {
#if !defined(__native_client__)
  memcpy(&kBrowserFuncs, browser_funcs, sizeof(kBrowserFuncs));
#endif
  plugin_funcs->newp = NPP_New;
  plugin_funcs->destroy = NPP_Destroy;
  plugin_funcs->setwindow = NPP_SetWindow;
  plugin_funcs->getvalue = NPP_GetValue;

  return NPERR_NO_ERROR;
}
