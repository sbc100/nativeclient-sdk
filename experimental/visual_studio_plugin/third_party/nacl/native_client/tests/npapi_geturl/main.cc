// Copyright (c) 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>

#include <nacl/nacl_npapi.h>
#include <nacl/npapi.h>
#include <nacl/npruntime.h>
#include <nacl/npupp.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "native_client/tests/npapi_runtime/plugin.h"

NPNetscapeFuncs* browser;

// Create a new instance of a plugin.
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
  Plugin *plugin = Plugin::New(instance, mime_type, argc, argn, argv);
  if (NULL == plugin) {
    return NPERR_GENERIC_ERROR;
  }
  instance->pdata = reinterpret_cast<void*>(plugin);
  return NPERR_NO_ERROR;
}

// Destroy an instance of a plugin.
NPError NPP_Destroy(NPP instance,
                    NPSavedData** save) {
  if (NULL == instance) {
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  delete reinterpret_cast<Plugin*>(instance->pdata);
  return NPERR_NO_ERROR;
}

NPError NPP_GetValue(NPP instance,
                     NPPVariable variable,
                     void* ret_value) {
  if (NPPVpluginScriptableNPObject == variable) {
    if (NULL == instance || NULL == instance->pdata) {
      return NPERR_GENERIC_ERROR;
    }
    *reinterpret_cast<NPObject**>(ret_value) =
        NPN_RetainObject(reinterpret_cast<NPObject*>(instance->pdata));
    return NPERR_NO_ERROR;
  }
  return NPERR_INVALID_PARAM;
}

NPError NPP_SetWindow(NPP instance,
                      NPWindow* window) {
  return NPERR_NO_ERROR;
}

namespace {

static void ReportResult(NPP npp,
                         const char* fname,
                         char* string,
                         bool success) {
  // Get window.
  NPObject* window_object;
  NPError error = NPN_GetValue(npp, NPNVWindowNPObject, &window_object);
  if (error != NPERR_NO_ERROR) {
    return;
  }

  // Invoke the function.
  NPVariant args[3];
  uint32_t arg_count = sizeof(args) / sizeof(args[0]);
  STRINGZ_TO_NPVARIANT(fname, args[0]);
  STRINGZ_TO_NPVARIANT(string, args[1]);
  BOOLEAN_TO_NPVARIANT(success, args[2]);
  NPVariant retval;
  VOID_TO_NPVARIANT(retval);
  NPIdentifier report_result_id = NPN_GetStringIdentifier("ReportResult");
  if (!NPN_Invoke(npp,
                  window_object,
                  report_result_id,
                  args,
                  arg_count,
                  &retval)) {
    NPN_ReleaseVariantValue(&retval);
  }
}

}  // namespace

void NPP_StreamAsFile(NPP instance,
                      NPStream* stream,
                      const char* fname) {
  // fname is actually a pointer to a file descriptor.
  const int fd = *reinterpret_cast<const int*>(fname);
  struct stat stb;
  FILE* iob = NULL;
  char* buf = NULL;
  size_t size = 0;
  size_t nchar = 0;
  int ch;

  if (NULL == stream) {
    char* str = strdup("bad stream pointer\n");
    char* url = strdup("NONE");
    ReportResult(instance, url, str, false);
    return;
  }
  if (-1 == fd) {
    char* str = strdup("Bad file descriptor received\n");
    ReportResult(instance, stream->url, str, false);
    return;
  }
  if (-1 == fstat(fd, &stb)) {
    char* str = strdup("fstat failed\n");
    ReportResult(instance, stream->url, str, false);
    return;
  }
  size = stream->end;
  buf = reinterpret_cast<char*>(NPN_MemAlloc(size));
  if ((stb.st_mode & S_IFMT) == S_IFSHM) {
    // Chrome integration returns a shared memory descriptor for this now.
    char* file_map = reinterpret_cast<char *>(mmap(NULL,
                                                   stb.st_size,
                                                   PROT_READ,
                                                   MAP_SHARED,
                                                   fd,
                                                   0));
    if (MAP_FAILED == file_map) {
      char* str = strdup("mmap failed\n");
      ReportResult(instance, stream->url, str, false);
      return;
    }
    for (nchar = 0; nchar < size - 1; ++nchar) {
      ch = file_map[nchar];
      buf[nchar] = ch;
    }
    buf[nchar] = '\0';
  } else {
    iob = fdopen(fd, "r");
    if (NULL == iob) {
      return;
    }
    for (nchar = 0; EOF != (ch = getc(iob)) && nchar < size - 1; ++nchar) {
      buf[nchar] = ch;
    }
    buf[nchar] = '\0';
  }
  ReportResult(instance, stream->url, buf, true);
  fclose(iob);
}

void NPP_URLNotify(NPP instance,
                   const char* url,
                   NPReason reason,
                   void* notifyData) {
  if (NPRES_DONE != reason) {
    const char* reason_name;
    if (NPRES_NETWORK_ERR == reason) {
      reason_name = "NPRES_NETWORK_ERR";
    } else if (NPRES_USER_BREAK == reason) {
      reason_name = "NPRES_USER_BREAK";
    } else {
      reason_name = "(BAD NPReason)";
    }
    char buf[1024];
    snprintf(buf,
             sizeof(buf),
             "NPP_URLNotify returned reason %s\n",
             reason_name);
    ReportResult(instance, url, buf, false);
  }
}

// NP_Initialize
NPError NP_Initialize(NPNetscapeFuncs* browser_funcs,
                      NPPluginFuncs* plugin_funcs) {
  // Imported APIs from the browser.
  browser = browser_funcs;
  // Exported APIs from the plugin.
  plugin_funcs->newp = NPP_New;
  plugin_funcs->destroy = NPP_Destroy;
  plugin_funcs->setwindow = NPP_SetWindow;
  plugin_funcs->getvalue = NPP_GetValue;
  plugin_funcs->asfile = NPP_StreamAsFile;
  plugin_funcs->urlnotify = NPP_URLNotify;
  return NPERR_NO_ERROR;
}
