// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_PI_GENERATOR_BASE_OBJECT_H_
#define EXAMPLES_PI_GENERATOR_BASE_OBJECT_H_

#if defined(__native_client__)
#include <nacl/nacl_npapi.h>
#else
// Building a trusted plugin for debugging.
#include "third_party/npapi/bindings/npapi.h"
#include "third_party/npapi/bindings/nphostapi.h"
#endif

// Helper class that maps calls to the NPObject into virtual methods.
class BaseObject : public NPObject {
 public:
  virtual ~BaseObject() {
  }

  virtual void Invalidate() {
  }

  virtual bool HasMethod(NPIdentifier name) {
    return false;
  }

  virtual bool Invoke(NPIdentifier name,
                      const NPVariant* args, uint32_t arg_count,
                      NPVariant* result) {
    return false;
  }

  virtual bool InvokeDefault(const NPVariant* args, uint32_t arg_count,
                             NPVariant* result) {
    return false;
  }

  virtual bool HasProperty(NPIdentifier name) {
    return false;
  }

  virtual bool GetProperty(NPIdentifier name, NPVariant* result) {
    return false;
  }

  virtual bool SetProperty(NPIdentifier name, const NPVariant* value) {
    return false;
  }

  virtual bool RemoveProperty(NPIdentifier name) {
    return false;
  }
};

#endif  // EXAMPLES_PI_GENERATOR_BASE_OBJECT_H_
