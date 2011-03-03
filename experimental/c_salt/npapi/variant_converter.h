// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_NPAPI_VARIANT_CONVERTER_H_
#define C_SALT_NPAPI_VARIANT_CONVERTER_H_

#include <nacl/npruntime.h>

#include "c_salt/variant_ptrs.h"

namespace c_salt {

namespace npapi {

// VariantConverter is a class that knows how to convert NPVariants to and
// from c_salt::Variants.
class VariantConverter {
 public:
  explicit VariantConverter(NPP instance);

  SharedVariant CreateVariantFromNPVariant(const NPVariant& np_var) const;
  void ConvertVariantToNPVariant(const c_salt::Variant& c_salt_var,
                                 NPVariant* np_var) const;
 private:
  NPP instance_;
};

}  // namespace npapi
}  // namespace c_salt

#endif  // C_SALT_NPAPI_VARIANT_CONVERTER_H_
