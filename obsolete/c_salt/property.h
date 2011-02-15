// Copyright (c) 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef C_SALT_PROPERTY_H_
#define C_SALT_PROPERTY_H_

#include <string>

#include "c_salt/variant.h"
#include "c_salt/variant_ptrs.h"

namespace c_salt {

// Class group that provides a wrapper for generic properties.  A Property can
// be published to the browser via a ScriptingBridge.  A Property maintains a
// variant value (currently a c_salt::Type), and sends notifications to
// observers about changes to the value.  Properties have a name that used by
// browser code.  You create a Property by setting up a PropertyParameter
// object.

// Support class for the Named Parameter Idiom.  To create a static,
// immutable Property using this idiom:
//   SharedVariant value(new c_salt::Variant(42));
//   PropertyAttributes prop_attrib("myProp", value)
//                                 .set_static()
//                                 .set_immutable();
//   Property *property = new Property(prop_attribs);
// Glossary:
//   mutable means the property value can be changed from the browser.  For
//       example, this JavaScript code changes the value of "myProp" to 42:
//         myModule.myProp = 42;
//   immutable means the property is read-only to the browser.  Attempting to
//       change an immutable property from the browser has no effect.
//   static means the property was added by the NaCl module code, usually
//       during initialization of a ScriptingBridge.  These properties cannot
//       be deleted by the browser.  All properties added by NaCl code should
//       static.
//   dynamic means the property was added by the browser.  All properties added
//       by the browser are dynamic.  For example, if "dynProp" was _not_
//       added by the NaCl code, then this JavaScript will add a dynamic
//       property and set its value:
//         myModule.dynProp = "hello, world";
//       Dynamic properties are always mutable.  Dynamic properties can be
//       removed by the browser, for example by using the JavaScript delete
//       operator.
class PropertyAttributes {
 public:
  PropertyAttributes(const std::string& name, const SharedVariant& value);
  PropertyAttributes& set_dynamic();
  PropertyAttributes& set_static();
  PropertyAttributes& set_immutable();
  PropertyAttributes& set_mutable();

 private:
  friend class Property;
  std::string name_;  // Must be set in the ctor.
  bool is_static_;  // Default is |true|.
  bool is_mutable_;  // Default is |true|.
  SharedVariant value_;
};

inline PropertyAttributes::PropertyAttributes(const std::string& name,
                                              const SharedVariant& value)
    : name_(name),
      is_static_(true),
      is_mutable_(true),
      value_(value) {
}

inline PropertyAttributes& PropertyAttributes::set_dynamic() {
  is_static_ = false;
  return *this;
}

inline PropertyAttributes& PropertyAttributes::set_static() {
  is_static_ = true;
  return *this;
}

inline PropertyAttributes& PropertyAttributes::set_immutable() {
  is_mutable_ = false;
  return *this;
}

inline PropertyAttributes& PropertyAttributes::set_mutable() {
  is_mutable_ = true;
  return *this;
}

class Property {
 public:

  explicit Property(const PropertyAttributes& attributes);

  // Get the value of the property.  This triggers the observer's
  // WillGetProperty and DidGetProperty protocol methods.
  SharedVariant GetValue() const;

  // Set the value of the property.  This triggers the observer's
  // WillSetProperty and DidSetProperty protocol methods.
  void SetValue(const SharedVariant& new_value);

  // Accessors for various attributes.  These cannot be changed during the
  // life of the Property instance.
  const std::string& name() const {
    return name_;
  }
  bool is_mutable() const {
    return is_mutable_;
  }
  bool is_static() const {
    return is_static_;
  }

 private:
  std::string name_;
  bool is_static_;
  bool is_mutable_;
  SharedVariant value_;

  Property();  // Not implemented, do not use.
};

}  // namespace c_salt

#endif  // C_SALT_PROPERTY_H_
