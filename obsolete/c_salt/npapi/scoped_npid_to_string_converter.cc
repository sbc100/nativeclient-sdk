// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/npapi/scoped_npid_to_string_converter.h"

namespace c_salt {
namespace npapi {

ScopedNPIdToStringConverter::ScopedNPIdToStringConverter(
    const NPIdentifier& np_id) : np_string_(NPN_UTF8FromIdentifier(np_id)) {
  string_value_ = static_cast<const char*>(np_string_);
}

ScopedNPIdToStringConverter::~ScopedNPIdToStringConverter() {
  NPN_MemFree(np_string_);
}

const std::string& ScopedNPIdToStringConverter::string_value() const {
  return string_value_;
}

}  // namespace npapi
}  // namespace c_salt
