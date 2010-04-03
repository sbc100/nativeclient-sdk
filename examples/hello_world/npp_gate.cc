/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__native_client__)
#include <nacl/nacl_npapi.h>
#include <nacl/npupp.h>
#else
// Building a development version.
#include "third_party/npapi/bindings/npapi.h"
#include "third_party/npapi/bindings/nphostapi.h"
#endif

struct HelloWorld {
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

  struct HelloWorld *hello_world = NULL;
  hello_world = new HelloWorld;
  hello_world->npp = instance;
  hello_world->npobject = NULL;

  instance->pdata = hello_world;
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
    HelloWorld* hello_world = static_cast<HelloWorld*>(instance->pdata);
    delete hello_world;
    instance->pdata = NULL;
  }
  return NPERR_NO_ERROR;
}

NPObject *NPP_GetScriptableInstance(NPP instance) {
  struct HelloWorld* hello_world;

  extern NPClass *GetNPSimpleClass();

  if (NULL == instance) {
    return NULL;
  }
  hello_world = static_cast<HelloWorld*>(instance->pdata);
  if (NULL == hello_world->npobject) {
    hello_world->npobject = NPN_CreateObject(instance, GetNPSimpleClass());
  }
  if (NULL != hello_world->npobject) {
    NPN_RetainObject(hello_world->npobject);
  }
  return hello_world->npobject;
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

extern "C" {

NPError InitializePluginFunctions(NPPluginFuncs* plugin_funcs) {
  memset(plugin_funcs, 0, sizeof(*plugin_funcs));
  plugin_funcs->version = NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL;
  plugin_funcs->size = sizeof(*plugin_funcs);
  plugin_funcs->newp = NPP_New;
  plugin_funcs->destroy = NPP_Destroy;
  plugin_funcs->setwindow = NPP_SetWindow;
  plugin_funcs->getvalue = NPP_GetValue;
  return NPERR_NO_ERROR;
}

}  // extern "C"
