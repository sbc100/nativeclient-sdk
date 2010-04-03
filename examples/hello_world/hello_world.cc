// Copyright 2008 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__native_client__)
#include <nacl/nacl_npapi.h>
#else
// Building a trusted plugin for debugging.
#include "third_party/npapi/bindings/npapi.h"
#include "third_party/npapi/bindings/nphostapi.h"
#endif

// Forward declarations of npruntime entry points.
static NPObject* Allocate(NPP npp, NPClass* npclass);
static void Deallocate(NPObject* object);
static void Invalidate(NPObject* object);
static bool HasMethod(NPObject* obj, NPIdentifier methodName);
static bool Invoke(NPObject* obj, NPIdentifier methodName,
                   const NPVariant *args, uint32_t argCount, NPVariant *result);
static bool InvokeDefault(NPObject *obj, const NPVariant *args,
                          uint32_t argCount, NPVariant *result);
static bool HasProperty(NPObject *obj, NPIdentifier propertyName);
static bool GetProperty(NPObject *obj, NPIdentifier propertyName,
                        NPVariant *result);
static bool SetProperty(NPObject* object,
                        NPIdentifier name,
                        const NPVariant* value);

/* Global Variables */
static NPClass npcRefClass = {
  NP_CLASS_STRUCT_VERSION,
  Allocate,
  Deallocate,
  Invalidate,
  HasMethod,
  Invoke,
  InvokeDefault,
  HasProperty,
  GetProperty,
  SetProperty
};

static NPObject* Allocate(NPP npp, NPClass* npclass) {
  return new NPObject;
}

static void Deallocate(NPObject* object) {
  delete object;
}

static void Invalidate(NPObject* object) {
}

/* Note that this HasMethod() always returns true, and below Invoke    */
/* always uses InvokeDefault which returns 42. The commented code      */
/* below shows how you would examine the method name in case you       */
/* wanted to have multiple methods with different behaviors.           */
static bool HasMethod(NPObject* obj, NPIdentifier methodName) {
  return true;
}

static bool InvokeDefault(NPObject *obj, const NPVariant *args,
                          uint32_t argCount, NPVariant *result) {
  if (result) {
    INT32_TO_NPVARIANT(42, *result);
  }
  return true;
}

static bool InvokeHW(NPObject *obj, const NPVariant *args,
                     uint32_t argCount, NPVariant *result) {
  if (result) {
    const char *msg = "hello, world.";
    const int msglength = strlen(msg) + 1;
    /* Note: msgcopy will be freed downstream by NPAPI, so */
    /* it needs to be allocated here with NPN_MemAlloc.    */
    char *msgcopy = reinterpret_cast<char*>(NPN_MemAlloc(msglength));
    strncpy(msgcopy, msg, msglength);
    STRINGN_TO_NPVARIANT(msgcopy, msglength - 1, *result);
  }
  return true;
}

/* Note that this Invoke() always calls InvokeDefault, which returns 42.    */
/* The commented code below shows how you would examine the method name     */
/* in case you wanted to have multiple methods with different behaviors.    */
static bool Invoke(NPObject* obj, NPIdentifier methodName,
                   const NPVariant *args, uint32_t argCount,
                   NPVariant *result) {
  char *name = NPN_UTF8FromIdentifier(methodName);
  bool rval = false;

  if (name && !strcmp((const char *)name, "helloworld")) {
    rval = InvokeHW(obj, args, argCount, result);
  } else if (name && !strcmp((const char *)name, "loopforever")) {
    /* for testing purposes only */
    while (1);
  } else {
    rval = InvokeDefault(obj, args, argCount, result);
  }
  /* Since name was allocated above by NPN_UTF8FromIdentifier,      */
  /* it needs to be freed here.                                     */
  NPN_MemFree(name);
  return rval;
}

static bool HasProperty(NPObject *obj, NPIdentifier propertyName) {
  return false;
}

static bool GetProperty(NPObject *obj, NPIdentifier propertyName,
                        NPVariant *result) {
  return false;
}

static bool SetProperty(NPObject* object, NPIdentifier name,
                        const NPVariant* value) {
  return false;
}

NPClass *GetNPSimpleClass() {
  return &npcRefClass;
}
