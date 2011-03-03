// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <assert.h>
#include <nacl/npapi_extensions.h>
#include <nacl/npupp.h>

// PINPAPI extensions.  These get filled in when NPP_New is called.
static NPExtensions* kPINPAPIExtensions = NULL;

void InitializePepperExtensions(NPP instance) {
  // Grab the PINPAPI extensions.
  NPN_GetValue(instance, NPNVPepperExtensions,
               reinterpret_cast<void*>(&kPINPAPIExtensions));
  assert(NULL != kPINPAPIExtensions);
}

// These are PINPAPI extensions.
NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device) {
  return kPINPAPIExtensions ?
      kPINPAPIExtensions->acquireDevice(instance, device) : NULL;
}

