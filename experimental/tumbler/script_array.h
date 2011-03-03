// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_TUMBLER_SCRIPT_ARRAY_H_
#define EXAMPLES_TUMBLER_SCRIPT_ARRAY_H_

#include <boost/noncopyable.hpp>
#include <ppapi/cpp/var.h>

namespace tumbler {

// This class wraps a Javascript Array object, providing convenience methods
// to set and get array values.
class ScriptArray : public boost::noncopyable {
 public:
  explicit ScriptArray(pp::Var array) : array_(array) {}

  // Return the number of elements in the array.  If an error occurs, return
  // -1.
  int32_t GetElementCount() const;

  // Set the element at |index| to |value|.  If there is no such element at
  // index, do nothing.
  bool SetValueAtIndex(const pp::Var& value, int32_t index);

  // Get the value at |index|.  If index < 0 or if the array has no property
  // associated with |index|, return an undefined pp::Var.
  pp::Var GetValueAtIndex(int32_t index) const;

 private:
  pp::Var array_;
};
}  // namespace tumbler
#endif  // EXAMPLES_TUMBLER_SCRIPT_ARRAY_H_

