// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <assert.h>
#if defined (__native_client__)
#include <nacl/npapi_extensions.h>
#include <nacl/npupp.h>
#else
#include "third_party/npapi/bindings/npapi.h"
#include "third_party/npapi/bindings/npapi_extensions.h"
#include "third_party/npapi/bindings/nphostapi.h"
#endif

// This file implements functions that are used by the plugin to call into the
// browser.  With the exception of any Pepper extensions, They only need to be
// implemented for the TRUSTED build.  In the published build they are
// provided by the environment.


#if !defined(__native_client__)
// The development version needs to call through to the browser directly.
// These wrapper routines are not required when making the published version.

// These functions are commented here because tracing them to their
// declarations can be cumbersome and this code is meant to serve as an
// example that should be easy to understand.  Also, the declarations for
// these functions lack comments.
// For each function, the declaring header is listed.  The NPN functions are
// declared in npapi.h and npruntime.h
// Also, a searchable location in the appropriate reference material is
// included (example: "Gecko Plugin API Reference->Scripting Plugins")

static NPNetscapeFuncs kBrowserFuncs = { 0 };

extern "C" {

// Called by NP_Initialize for platform specific trusted instances of this
// program.  Not used by untrusted version.  Not declared anywhere, used
// as 'extern' only.  This is implemented here so that the trusted runtime
// can call it to tell this module what browser functions are available
// to it.
void InitializeBrowserFunctions(NPNetscapeFuncs* browser_functions) {
  memcpy(&kBrowserFuncs, browser_functions, sizeof(kBrowserFuncs));
}

}  // extern "C"

// Translates between browser and machine repesentations of |identifier|.
// Declaration: npruntime.h
// Included via npupp.h (if native client) or nphostapi.h in trusted build.
// Web Reference: Gecko Plugin API Reference->Scripting Plugins
NPUTF8* NPN_UTF8FromIdentifier(NPIdentifier identifier) {
  return kBrowserFuncs.utf8fromidentifier(identifier);
}

// Allocates |size| bytes in the browser's memory pool and returns a pointer.
// Declaration: npapi.h
// Web Reference: Gecko Plugin API Reference->Plug-in Side Plug-in API
void* NPN_MemAlloc(uint32 size) {
  return kBrowserFuncs.memalloc(size);
}

// Frees the memory referenced by |mem|
// Declaration: npapi.h
// Web Reference: Gecko Plugin API Reference->Plug-in Side Plug-in API
void NPN_MemFree(void* mem) {
  kBrowserFuncs.memfree(mem);
}

// Allocates memory for an object using |np_class| to get definition info.
// The object is then associated with |npp|, the plugin that is requesting it.
// Returns a reference counted point to an |NPObject|.
// Declaration: npruntime.h
// Web Reference: Gecko Plugin API Reference->Scripting Plugins
NPObject* NPN_CreateObject(NPP npp, NPClass* np_class) {
  return kBrowserFuncs.createobject(npp, np_class);
}

// Increments the reference count for |obj| and returns a pointer to the same
// object.
// Declaration: npruntime.h
// Web Reference: Gecko Plugin API Reference->Scripting Plugins
NPObject* NPN_RetainObject(NPObject* obj) {
  return kBrowserFuncs.retainobject(obj);
}

#endif
