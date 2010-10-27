// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
#include "c_salt/npapi/variant_converter.h"

#include <nacl/npruntime.h>

#include <limits>
#include <string>
#include <vector>

#include "c_salt/instance.h"
#include "c_salt/module.h"
#include "gtest/gtest.h"

namespace {

// An overload for streaming a vector of streamable types.
template <class T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& vector) {
  typename std::vector<T>::const_iterator iter(vector.begin()),
                                          last(vector.end()-1);
  for (; iter != last; ++iter) {
    s << *iter << ", ";
  }
  s << *iter << "\n";
  return s;
}

class NPVariantConverterTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}
};

// Create a c_salt::Variant with the given value.  Convert it to NPVariant and
// back to make sure the result matches.
template <class T>
void CheckConversion(T value) {
  c_salt::SharedVariant c_salt_var(new c_salt::Variant(value));
  NPVariant np_var;
  c_salt::SharedVariant dest_c_salt_var;
  c_salt::npapi::VariantConverter converter(NULL);
  converter.ConvertVariantToNPVariant(*c_salt_var, &np_var);
  dest_c_salt_var = converter.CreateVariantFromNPVariant(np_var);
  EXPECT_EQ(c_salt_var->variant_type(), dest_c_salt_var->variant_type());
  EXPECT_EQ(c_salt_var->GetValue<T>(),
            dest_c_salt_var->GetValue<T>());
}

TEST_F(NPVariantConverterTest, SameType) {
  // strings
  CheckConversion(std::string(""));
  CheckConversion(std::string("Hello World!"));
  CheckConversion(std::string("3.14"));
  CheckConversion(std::string("42"));

  // int32_t
  CheckConversion(static_cast<int32_t>(-1));
  CheckConversion(static_cast<int32_t>(0));
  CheckConversion(static_cast<int32_t>(1));
  CheckConversion(static_cast<int32_t>(42));
  CheckConversion(std::numeric_limits<int32_t>::max());
  CheckConversion(std::numeric_limits<int32_t>::min());

  // double
  CheckConversion(static_cast<double>(0));
  CheckConversion(3.1415);
  CheckConversion(std::numeric_limits<double>::max());
  CheckConversion(std::numeric_limits<double>::min());
  CheckConversion(std::numeric_limits<double>::infinity());

  // bool
  CheckConversion(true);
  CheckConversion(false);
}

}  // unnamed namespace

class MyInstance : public c_salt::Instance {
 public:
  explicit MyInstance(const NPP& instance) : c_salt::Instance(instance) {}
};

class TestModule : public c_salt::Module {
 public:
  virtual c_salt::Instance* CreateInstance(const NPP& instance) {
    return new MyInstance(instance);
  }
};
