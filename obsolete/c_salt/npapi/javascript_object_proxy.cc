// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/npapi/javascript_object_proxy.h"

#include "c_salt/npapi/scoped_npid_to_string_converter.h"
#include "c_salt/variant.h"

namespace c_salt {

namespace npapi {

namespace {
  // RAII wrappers for NPVariant that calls NPN_ReleaseVariantValue on
  // destruction.  NPVariantSentry is for most uses where you want to create an
  // NPVariant on the stack and have it released at the end of scope.  In those
  // cases, simply create an NPVariantSentry, and when you need to use the
  // NPVariant it owns, call my_sentry.np_variant() to get a reference to it.
  class NPVariantSentry {
   public:
    NPVariantSentry() : np_variant_() {}
    NPVariant& np_variant() { return np_variant_; }
    ~NPVariantSentry() { NPN_ReleaseVariantValue(&np_variant_); }
   private:
    NPVariant np_variant_;
  };
  // NPVariantPtrSentry is used when you want to own the NPVariant some other
  // way, but you want NPN_ReleaseVariantValue to be invoked when the
  // NPVariantPtrSentry goes out of scope.
  class NPVariantPtrSentry {
   public:
    NPVariantPtrSentry() : np_variant_(NULL) {}
    explicit NPVariantPtrSentry(NPVariant* np_variant)
        : np_variant_(np_variant) {}
    NPVariant* np_variant() { return np_variant_; }
    void set_np_variant(NPVariant* np_variant) {
      release();
      np_variant_ = np_variant;
    }
    void release() {
      if (np_variant_) {
        NPN_ReleaseVariantValue(np_variant_);
      }
      np_variant_ = NULL;
    }
    ~NPVariantPtrSentry() { release(); }
   private:
    NPVariant* np_variant_;
  };
}

JavaScriptObjectProxy::JavaScriptObjectProxy(NPObject* np_object,
                                             NPP instance)
    : instance_(instance), np_object_(np_object), variant_converter_(instance) {
  NPN_RetainObject(np_object_);
}

JavaScriptObjectProxy::~JavaScriptObjectProxy() {
  NPN_ReleaseObject(np_object_);
}

void JavaScriptObjectProxy::Invalidate() {
  instance_ = NULL;
  np_object_ = NULL;
}

bool JavaScriptObjectProxy::Valid() const {
  return (instance_ && np_object_);
}

bool JavaScriptObjectProxy::HasScriptMethod(const std::string& name) {
  if (!Valid()) return false;
  NPIdentifier name_id = NPN_GetStringIdentifier(name.c_str());
  return NPN_HasMethod(instance_, np_object_, name_id);
}

bool
JavaScriptObjectProxy::InvokeScriptMethod(
    const std::string& method_name,
    const SharedVariant* params_begin,
    const SharedVariant* params_end,
    ::c_salt::SharedVariant* return_value) {
  if (!Valid()) return false;
  // Allocate a vector of NPVariants to hold the parameters and convet the
  // c_salt variants to them.
  std::size_t arg_count = params_end - params_begin;
  std::vector<NPVariant> np_var_vector(arg_count);
  // Make a vector of sentries to ensure the NPVariants are cleaned up
  // regardless of how we exit this function.
  std::vector<NPVariantPtrSentry> np_var_sentry_vector(arg_count);
  for (size_t i = 0; i < arg_count; ++i) {
    // Point the i-th sentry at the i-th NPVariant.  Note that it's not
    // necessary prior to this, as the i-th sentry is still NULL until after the
    // conversion.
    np_var_sentry_vector[i].set_np_variant(&np_var_vector[i]);
    variant_converter_.ConvertVariantToNPVariant(*params_begin[i],
                                                 &np_var_vector[i]);
  }
  bool success = false;
  NPVariantSentry return_var;
  if (!method_name.empty()) {
    NPIdentifier name_id = NPN_GetStringIdentifier(method_name.c_str());
    success = NPN_Invoke(instance_,
                         np_object_,
                         name_id,
                         &(*np_var_vector.begin()),
                         np_var_vector.size(),
                         &return_var.np_variant());
  } else {
    success = NPN_InvokeDefault(instance_,
                                np_object_,
                                &(*np_var_vector.begin()),
                                np_var_vector.size(),
                                &return_var.np_variant());
  }
  if (success) {
    (*return_value) =
        variant_converter_.CreateVariantFromNPVariant(return_var.np_variant());
  }
  return success;
}

void JavaScriptObjectProxy::GetAllPropertyNames(
    std::vector<std::string>* prop_names)
  const {
  if (!Valid()) return;
  if (prop_names) {
    // Use the copy-and-swap idiom to ensure prop_names is only modified if
    // everything succeeds.
    std::vector<std::string> keys;
    NPIdentifier* npid_array;
    uint32_t array_size;
    if (NPN_Enumerate(instance_, np_object_, &npid_array, &array_size)) {
      keys.resize(array_size);
      for (uint32_t i = 0; i < array_size; ++i) {
        keys[i] = ScopedNPIdToStringConverter(npid_array[i]).string_value();
      }
    }
    NPN_MemFree(npid_array);
    prop_names->swap(keys);
  }
}

bool JavaScriptObjectProxy::HasScriptProperty(const std::string& name) {
  if (!Valid()) return false;
  NPIdentifier name_id = NPN_GetStringIdentifier(name.c_str());
  return NPN_HasProperty(instance_, np_object_, name_id);
}

bool
JavaScriptObjectProxy::GetScriptProperty(const std::string& name,
                                         SharedVariant* return_value) const {
  if (!Valid()) return false;
  assert(return_value);
  if (NULL == return_value) {
    return false;
  }
  NPIdentifier name_id = NPN_GetStringIdentifier(name.c_str());
  NPVariantSentry result;
  bool success = NPN_GetProperty(instance_,
                                 np_object_,
                                 name_id,
                                 &result.np_variant());
  if (success) {
    *return_value = variant_converter_.
        CreateVariantFromNPVariant(result.np_variant());
  }
  return success;
}

bool JavaScriptObjectProxy::SetScriptProperty(const std::string& name,
                                        const SharedVariant& value) {
  if (!Valid()) return false;
  NPIdentifier name_id = NPN_GetStringIdentifier(name.c_str());
  NPVariantSentry np_var;
  variant_converter_.ConvertVariantToNPVariant(*value, &np_var.np_variant());
  bool success = NPN_SetProperty(instance_,
                                 np_object_,
                                 name_id,
                                 &np_var.np_variant());
  return success;
}

bool JavaScriptObjectProxy::RemoveScriptProperty(const std::string& name) {
  if (!Valid()) return false;
  NPIdentifier name_id = NPN_GetStringIdentifier(name.c_str());
  return NPN_RemoveProperty(instance_, np_object_, name_id);
}

}  // namespace npapi
}  // namespace c_salt
