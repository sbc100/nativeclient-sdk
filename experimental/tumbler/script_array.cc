// Copyright 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "examples/tumbler/script_array.h"

#include <nacl/nacl_inttypes.h>
#include <ppapi/cpp/var.h>
#include <cstdio>

namespace {
const char* const kLengthProperty = "length";
const size_t kMaxPropertyStringLength = 64;

// Return |index| as a string Var suitable for use as a property name.
pp::Var IndexAsStringVar(int32_t index) {
  char string_buffer[kMaxPropertyStringLength + 1];
  // Note: There is a problem using PRId32 with int32_t types in printf().  Use
  // NACL_PRId32 as a work-around.  See:
  // http://code.google.com/p/nativeclient/issues/detail?id=971
  snprintf(string_buffer, kMaxPropertyStringLength, "%"NACL_PRId32, index);
  return pp::Var(string_buffer);
}
}  // namespace

namespace tumbler {

int32_t ScriptArray::GetElementCount() const {
  pp::Var exception;  // Use initial "void" Var for exception
  pp::Var element_count = array_.GetProperty(kLengthProperty, &exception);
  if (!exception.is_undefined()) {
    return -1;
  }
  return element_count.AsInt();
}

bool ScriptArray::SetValueAtIndex(const pp::Var& value, int32_t index) {
  // Create a string property name for the index, and attempt to get the
  // value of the property from the array object.
  pp::Var exception;
  array_.SetProperty(IndexAsStringVar(index), value, &exception);
  if (!exception.is_undefined()) {
    return false;
  }
  return true;
}

pp::Var ScriptArray::GetValueAtIndex(int32_t index) const {
  if (index < 0) return pp::Var(false);
  // Create a string property name for the index, and attempt to set the
  // value of the property in the array object.
  pp::Var exception;
  pp::Var index_property = IndexAsStringVar(index);
  if (array_.HasProperty(index_property, &exception)) {
    pp::Var index_value = array_.GetProperty(index_property, &exception);
    if (!exception.is_undefined()) {
      return pp::Var(false);
    }
    return index_value;
  }
  return pp::Var(false);
}
}  // namespace tumbler

