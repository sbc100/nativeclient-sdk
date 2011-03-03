// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/property.h"

namespace c_salt {
Property::Property(const PropertyAttributes& attributes)
    : name_(attributes.name_),
      is_static_(attributes.is_static_),
      is_mutable_(attributes.is_mutable_),
      value_(attributes.value_) {
}

SharedVariant Property::GetValue() const {
  // TODO(dspringer,dmichael): Add the observer calls here.  boost::signals2?
  // observers.WillGetProperty(this, value_);
  return value_;
}

void Property::SetValue(const SharedVariant& new_value) {
  // TODO(dspringer,dmichael): Add the observer calls here.  boost::signals2?
  // observers.WillSetProperty(this, value_, new_value);
  value_ = new_value;
  // observers.DidSetProperty(this, new_value);
}
}  // namespace c_salt
