// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/converting_visitor.h"

#include <cctype>
#include <cmath>
#include <limits>
#include <sstream>

#include "c_salt/scripting_bridge.h"
#include "c_salt/scripting_interface.h"

namespace c_salt {

std::string ConvertingVisitor<std::string>::operator()(bool value) const {
  return std::string(value ? "true" : "false");
}

std::string ConvertingVisitor<std::string>::operator()(double value) const {
  std::ostringstream stream;
  // Try to get enough places after the decimal so that converting back
  // to a double type preserves as much of the precision as possible.
  // This says to represent floating point types with a fixed number of
  // digits.
  stream.setf(std::ios::fixed, std::ios::floatfield);
  // digits10 means "the smallest number of digits in base 10 that can
  // represent double without loss of precision"
  stream.precision(std::numeric_limits<double>::digits10);
  stream << value;
  return stream.str();
}

std::string ConvertingVisitor<std::string>::operator()(int32_t value) const {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

// In this case, we can just pass the same const-reference out, since it
// exists in the variant.  In all other cases, we have to copy because we are
// creating a std::string on the stack.
const std::string&
ConvertingVisitor<std::string>::operator()(const std::string& value) const {
  return value;
}

std::string ConvertingVisitor<std::string>::operator()(
    const SharedScriptingInterface& value) const {
  // TODO(dspringer, dmichael):  Should we bother converting it to a string,
  //                             a-la JSON?
  return std::string();
}

SharedScriptingInterface
ConvertingVisitor<SharedScriptingInterface>::operator()(
      const SharedScriptingInterface& value) const {
    return value;
}

SharedScriptableNativeObject
ConvertingVisitor<SharedScriptableNativeObject>::operator()(
      const SharedScriptingInterface& value) const {
  // We can only convert it to a native object if it is in fact implemented on
  // the Native side.
  if (value->IsNative()) {
    SharedScriptingBridge bridge =
        boost::static_pointer_cast<ScriptingBridge>(value);
    SharedScriptableNativeObject sno = bridge->native_object();
    return sno;
  }
  // If it's not native, we can't support this conversion.  Return a NULL.
  return SharedScriptableNativeObject();
}

bool ConvertingVisitor<bool>::operator()(bool value) const {
  return value;
}

bool ConvertingVisitor<bool>::operator()(int32_t value) const {
  return (value != 0);
}

bool ConvertingVisitor<bool>::operator()(double value) const {
  // Return |true| if |value| is non-0 within machine epsilon.
  return (std::fabs(value) > std::numeric_limits<double>::epsilon());
}

bool ConvertingVisitor<bool>::operator()(const std::string& value) const {
  if (value.empty()) return false;
  int ch = std::tolower(value[0]);
  return ch == 'y' || ch == '1' || ch == 't';
}

bool ConvertingVisitor<bool>::operator()(
    const SharedScriptingInterface& value) const {
  return static_cast<bool>(value);  // Return true if value is not null.
}

int32_t ConvertingVisitor<int32_t>::operator()(bool value) const {
  return value ? 1 : 0;
}

int32_t ConvertingVisitor<int32_t>::operator()(int32_t value) const {
  return value;
}

int32_t ConvertingVisitor<int32_t>::operator()(double value) const {
  return static_cast<int32_t>(value);
}

int32_t ConvertingVisitor<int32_t>::operator()(const std::string& value) const {
  std::istringstream input_stream(value);
  int32_t int_value(0);
  // This may fail, in which case int_value remains 0.
  input_stream >> int_value;
  return int_value;
}

int32_t ConvertingVisitor<int32_t>::operator()(
    const SharedScriptingInterface& value) const {
  return 0;
}

double ConvertingVisitor<double>::operator()(bool value) const {
  return value ? 1.0 : 0.0;
}

double ConvertingVisitor<double>::operator()(int32_t value) const {
  // Truncate value.  E.g., 3.5 -> 3, and -3.5 -> -3.
  // We don't use floor because it always converts to the largest
  // integer _less_than_or_equal_ to the given double.  E.g.,
  // -3.5 -> -4
  return static_cast<double>(value);
}

double ConvertingVisitor<double>::operator()(double value) const {
  return value;
}

double ConvertingVisitor<double>::operator()(const std::string& value) const {
  std::istringstream input_stream(value);
  double double_value(0.0);
  // This may fail, in which case double_value remains 0.0.
  input_stream >> double_value;
  return double_value;
}

double ConvertingVisitor<double>::operator()(
    const SharedScriptingInterface& value) const {
  return 0.0;
}

}  // namespace c_salt
