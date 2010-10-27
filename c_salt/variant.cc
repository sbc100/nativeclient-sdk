// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/variant.h"

#include <cassert>
#include "c_salt/scripting_bridge.h"

namespace c_salt {

Variant::~Variant() {
}

bool operator==(const Variant& left, const Variant& right) {
  return ((left.variant_type() == right.variant_type()) &&
          (left.value_ == right.value_));
}

}  // namespace c_salt
