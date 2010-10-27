// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_CONVERTING_VISITOR_H_
#define C_SALT_CONVERTING_VISITOR_H_

#include <string>

#include "boost/shared_ptr.hpp"
#include "boost/variant/static_visitor.hpp"
#include "c_salt/scriptable_native_object_ptrs.h"
#include "c_salt/scriptable_native_object.h"
#include "c_salt/scripting_interface_ptrs.h"

namespace c_salt {

class ScriptingInterface;

// The following set of ConvertingVisitor template specializations implement the
// Visitor concept, as described here:
// http://live.boost.org/doc/libs/1_41_0/doc/html/variant/reference.html
//   #variant.concepts.static-visitor
// Each instantiation of ConvertingVisitor can be passed as a static visitor to
// the boost::apply_visitor function.  ConvertingVisitor<TargetType> converts
// the value held by the boost variant in to type TargetType.
//
// This one is the default, which has no implementation.  We only have
// specializations implement ConvertingVisitor, as we want to have very
// fine-grained control of what each conversion does depending on the type.
template <class TargetType>
class ConvertingVisitor;

template <>
class ConvertingVisitor<std::string>
  : public boost::static_visitor<std::string> {
 public:
  std::string operator()(bool value) const;
  std::string operator()(int32_t value) const;
  std::string operator()(double value) const;
  // In this case, we can just pass the same const-reference out, since it
  // exists in the variant.  In all other cases, we have to copy because we are
  // creating a std::string on the stack.
  const std::string& operator()(const std::string& value) const;
  std::string operator()(const SharedScriptingInterface& value) const;
};

template <>
class ConvertingVisitor<SharedScriptingInterface >
  : public boost::static_visitor<SharedScriptingInterface > {
 public:
  template <class T>
  SharedScriptingInterface operator()(T value) const {
    // This is a catch all for types other than shared_ptr to
    // ScriptingInterface, none of which can be converted to
    // shared_ptr<ScriptingInterface>, so we just return a default-initialized
    // shared_ptr.
    return SharedScriptingInterface();
  }
  SharedScriptingInterface operator()(
      const SharedScriptingInterface& value) const;
};

// This specialization handles getting a ScriptableNativeObject.  Currently, we
// do NOT support converting to boost::shared_ptr<T> where T is a class which
// inherits from ScriptableNativeObject.  To support it safely, we would require
// a dynamic_cast to verify the type is correct, which would violate the Google
// style guide by requiring RTTI.  To support it unsafely, we could static_cast,
// but that would make strange failures happen if the user got it even slightly
// wrong.  This forces them to get a SharedScriptableNativeObject and then do
// the cast themselves, which at least puts any failures at the right place
// (user code).
template <>
class ConvertingVisitor<SharedScriptableNativeObject>
  : public boost::static_visitor<SharedScriptableNativeObject> {
 public:
  template <class T>
  SharedScriptableNativeObject operator()(T value) const {
    // This is a catch all for types other than shared_ptr to
    // ScriptingInterface, none of which can be converted to
    // shared_ptr<ScriptingInterface>, so we just return a default-initialized
    // shared_ptr.
    return SharedScriptableNativeObject();
  }
  SharedScriptableNativeObject operator()(
      const SharedScriptingInterface& value) const;
};

template <>
class ConvertingVisitor<bool> : public boost::static_visitor<bool> {
 public:
  bool operator()(bool value) const;
  bool operator()(int32_t value) const;
  bool operator()(double value) const;
  bool operator()(const std::string& value) const;
  bool operator()(const SharedScriptingInterface& value) const;
};

template <>
class ConvertingVisitor<int32_t> : public boost::static_visitor<int32_t> {
 public:
  int32_t operator()(bool value) const;
  int32_t operator()(int32_t value) const;
  int32_t operator()(double value) const;
  int32_t operator()(const std::string& value) const;
  int32_t operator()(const SharedScriptingInterface& value) const;
};

template <>
class ConvertingVisitor<double> : public boost::static_visitor<double> {
 public:
  double operator()(bool value) const;
  double operator()(int32_t value) const;
  double operator()(double value) const;
  double operator()(const std::string& value) const;
  double operator()(const SharedScriptingInterface& value) const;
};

}  // namespace c_salt

#endif  // C_SALT_CONVERTING_VISITOR_H_
