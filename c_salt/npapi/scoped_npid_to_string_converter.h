// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_NPAPI_SCOPED_NPID_TO_STRING_CONVERTER_H_
#define C_SALT_NPAPI_SCOPED_NPID_TO_STRING_CONVERTER_H_

#include <nacl/nacl_npapi.h>
#include <nacl/npruntime.h>

#include <string>

namespace c_salt {
namespace npapi {

// A small helper class that converts an NPIdentifier into a std::string.
// It follows the RAII pattern: when the class goes out of scope, the memory
// used to get the string is freed with NPN_MemFree().
class ScopedNPIdToStringConverter {
 public:
  explicit ScopedNPIdToStringConverter(const NPIdentifier& np_id);
  ~ScopedNPIdToStringConverter();
  const std::string& string_value() const;
 private:
  NPUTF8* np_string_;
  std::string string_value_;
};

}  // namespace npapi
}  // namespace c_salt

#endif  // C_SALT_NPAPI_SCOPED_NPID_TO_STRING_CONVERTER_H_
