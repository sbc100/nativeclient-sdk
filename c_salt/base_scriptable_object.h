// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
#ifndef C_SALT_BASE_SCRIPTABLE_OBJECT_H_
#define C_SALT_BASE_SCRIPTABLE_OBJECT_H_

#include <map>
#include <string>

#include <boost/signals2.hpp>
#include <nacl/npruntime.h>

#include "c_salt/type.h"

namespace c_salt {
// BaseScriptableObject is a base class that implements the common functionality
// to communicate over the js bridge of NPAPI.  Making calls from the browser to
// c++ code and vice versa requires a fair amount of boilerplate code.  By
// privately subclassing BaseScriptableObject, a class inherits the
// implemenations to the boilerplate code.  BaseScriptableObject maintains
// dispatch tables providing access to methods and properties of the subclasses
// from the browser.  For each method the subclass wants to expose to the
// browser, it must declare and define the method with the following signature:
//   bool ExposedMethod(const NPVariant* args, uint32_t arg_count,
//                      NPVariant* result);
// where args is the arguments passed in from the browser and result is the
// return value that is passed to the browser.
// Subsequently the subclass must register the method with the dispatch table in
// it's ctor by calling BaseScriptableObject::RegisterMethod;
// Similarly, named property getters and setters must be declared and
// defined with the signatures:
//   bool PropertyGetter(NPVariant* property);
//   bool PropertySetter(const NPVariant* property);
// and registered with either BaseScriptableObject::RegisterReadOnlyProperty
// or BaseScriptableObject::RegisterProperty in the ctor.
// Example use:
//   class ScriptableObject : private BaseScriptableObject {
//     public:
//       explicit ScriptableObject(NPP npp) : BaseScriptableObject(npp) {
//         RegisterMethod("exposedMethod", this, &ScriptableObject::Method);
//         RegisterProperty("someProperty", this,
//                          &ScriptableObject::PropertyGetter,
//                          &ScriptableObject::PropertySetter);
//       }
//
//     private:
//       bool Method(const NPVariant* args, uint32_t arg_count, NPVariant*
//                   result);
//      bool PropertyGetter(NPVariant* property);
//      bool PropertySetter(const NPVariant* property);
//   };
class BaseScriptableObject {
 public:
  // SetValueForKey is called by the browser to retrieve the value for key.
  // @param args The key of the value retrieved.
  // @param arg_count Ignored.
  // @param result The destination of the retrieved value.
  // @return bool true if the value for the key is successfully retrieved.
  bool GetValueForKey(const NPVariant* args, uint32_t arg_count,
                      NPVariant* result);

  // SetValueForKey is called by the browser to assign value to key.
  // @param args The key of the value assigned.
  // @param arg_count Ignored.
  // @param result The destination of the retrieved value.
  // @return bool true if the value for the key is successfully assigned.
  bool SetValueForKey(const NPVariant* args, uint32_t arg_count,
                      NPVariant* result);

  // Invalidate, Invoke, HasMethod, GetProperty, and SetProperty are
  // implementations of the function pointers of an NPClass.  They are called by
  // the browser and should not be called directly.
  void Invalidate(NPObject *obj);
  bool Invoke(NPIdentifier name, const NPVariant* args,
              uint32_t arg_count, NPVariant* result);
  bool HasMethod(NPIdentifier name) const;
  bool HasProperty(NPIdentifier name) const;
  bool GetProperty(NPIdentifier name, NPVariant* value) const;
  bool SetProperty(NPIdentifier name, const NPVariant* value);

 protected:
  // Subclasses call RegisterReadOnlyProperty and RegisterProperty
  // to register their named properties with the dispatch
  // table.
  template <typename T>
  void RegisterReadOnlyProperty(
      const char* name,
      T* obj,
      bool (T::*property_getter)(NPVariant*));
  template <typename T>
  void RegisterProperty(
      const char* name,
      T* obj,
      bool (T::*property_getter)(NPVariant*),
      bool (T::*property_setter)(const NPVariant*));
  // Register a given object and it's method with the dispatch
  // table.
  template <typename T>
  void RegisterMethod(
      const char* name,
      T* obj,
      bool (T::*method)(const NPVariant*, uint32_t, NPVariant*));

  explicit BaseScriptableObject(const NPP& npp);
  virtual ~BaseScriptableObject() = 0;

 private:
  typedef boost::signals2::signal<bool (const NPVariant*, uint32_t, NPVariant*)>
      MethodType;
  typedef MethodType::slot_type Method;
  typedef boost::shared_ptr<MethodType> MethodHandle;

  typedef boost::signals2::signal<bool (NPVariant*)>
      PropertyGetterType;
  typedef PropertyGetterType::slot_type PropertyGetter;
  typedef boost::shared_ptr<PropertyGetterType> PropertyGetterHandle;

  typedef boost::signals2::signal<bool (const NPVariant*)>
      PropertySetterType;
  typedef PropertySetterType::slot_type PropertySetter;
  typedef boost::shared_ptr<PropertySetterType> PropertySetterHandle;

  // Subclasses must implement if they want to respond to kvc.
  virtual bool GetValueForKeyImpl(const std::string& key,
                                  NPVariant* result) {
    return false;
  };
  virtual bool SetValueForKeyImpl(const std::string& key,
                                  const NPVariant& value) {
    return false;
  }

  // Subclasses must implement if they want to responds to invalidate calls from
  // the browser.  Invalidate calls are made by the browser right before the
  // object is deallocated.
  virtual void InvalidateImpl(NPObject* obj) {
  }

  const NPP npp_;
  // hash_map not supported in nacl-g++
  std::map<const std::string, MethodHandle> method_table_;
  std::map<const std::string, PropertyGetterHandle> property_table_;
  std::map<const std::string, PropertySetterHandle> set_property_table_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(BaseScriptableObject);
};

template <typename T /* BaseScriptableObject subclass */>
void BaseScriptableObject::RegisterMethod(
    const char* name,
    T* obj,
    bool (T::*method)(const NPVariant*, uint32_t, NPVariant*)) {
  MethodHandle obj_method(new MethodType);
  obj_method->connect(boost::bind(method, obj, _1, _2, _3));
  method_table_.insert(std::pair<const std::string, MethodHandle>(
      name, obj_method));
}

template <typename T /* BaseScriptableObject subclass */>
void BaseScriptableObject::RegisterReadOnlyProperty(
    const char* name,
    T* obj,
    bool (T::*property_getter)(NPVariant*)) {
  PropertyGetterHandle property_getter_handle(new PropertyGetterType);
  property_getter_handle->connect(boost::bind(property_getter, obj, _1));
  property_table_.insert(std::pair<const std::string, PropertyGetterHandle>(
      name, property_getter_handle));
}

template <typename T /* BaseScriptableObject subclass */>
void BaseScriptableObject::RegisterProperty(
    const char* name,
    T* obj,
    bool (T::*property_getter)(NPVariant*),
    bool (T::*property_setter)(const NPVariant*)) {
  RegisterReadOnlyProperty(name, obj, property_getter);
  if (NULL != property_setter) {
    PropertySetterHandle property_setter_handle(new PropertySetterType);
    property_setter_handle->connect(boost::bind(property_setter, obj, _1));
    set_property_table_.insert(
        std::pair<const std::string, PropertySetterHandle>(
            name, property_setter_handle));
  }
}

}  // namespace c_salt

#endif  // C_SALT_BASE_SCRIPTABLE_OBJECT_H_
