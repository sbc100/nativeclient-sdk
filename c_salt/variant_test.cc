// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include "c_salt/variant.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using c_salt::Variant;

namespace {
const char* kTestBoolTrueString = "true";
const double kTestDoubleValue = 3.141592653589793;
const char* kTestDoubleAsString = "3.1415926535897";
const int32_t kTestIntValue = 42;
const char* kTestIntAsString = "42";
const char* kTestString = "this is a test string";
// |kTestTruncatedIntValue| is the integer equivalent to
// floor(kTestDoubleValue).
const int32_t kTestTruncatedIntValue = 3;
}  // namespace

class VariantTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}
};

// Create a Variant from string.  Tests all the typed return values.
TEST_F(VariantTest, CreateFromStr) {
  Variant str_var(kTestString);
  EXPECT_EQ(Variant::kStringVariantType, str_var.variant_type());
  EXPECT_EQ(kTestString, str_var.StringValue());
  // BoolValue() should return |true| because the test string begins with a
  // 't'.
  EXPECT_TRUE(str_var.BoolValue());
  EXPECT_EQ(0, str_var.Int32Value());
  EXPECT_EQ(0.0, str_var.DoubleValue());
}

// Create a Variant from an int32.  Tests all the typed return values.
TEST_F(VariantTest, CreateFromInt) {
  Variant int_var(kTestIntValue);
  EXPECT_EQ(Variant::kInt32VariantType, int_var.variant_type());
  EXPECT_EQ(kTestIntAsString, int_var.StringValue());
  EXPECT_TRUE(int_var.BoolValue());
  EXPECT_EQ(kTestIntValue, int_var.Int32Value());
  EXPECT_TRUE(std::fabs(int_var.DoubleValue() - kTestIntValue) <=
              std::numeric_limits<double>::epsilon());
  // TODO(dspringer): Add this test when ObjectValue() is implemented.
  // EXPECT_EQ(NULL, int_var.ObjectValue());
}

// Create a Variant from a double.  Tests all the typed return values.
TEST_F(VariantTest, CreateFromDouble) {
  Variant dbl_var(kTestDoubleValue);
  EXPECT_EQ(Variant::kDoubleVariantType, dbl_var.variant_type());
  EXPECT_EQ(0, strncmp(dbl_var.StringValue().c_str(),
                       kTestDoubleAsString,
                       strlen(kTestDoubleAsString)));
  EXPECT_TRUE(dbl_var.BoolValue());
  EXPECT_EQ(kTestTruncatedIntValue, dbl_var.Int32Value());
  EXPECT_TRUE(std::fabs(dbl_var.DoubleValue() - kTestDoubleValue) <=
              std::numeric_limits<double>::epsilon());
}

// Create a Variant from an bool.  Tests all the typed return values.
TEST_F(VariantTest, CreateFromBoolNPVariant) {
  Variant bool_var(true);
  EXPECT_EQ(Variant::kBoolVariantType, bool_var.variant_type());
  EXPECT_EQ("true", bool_var.StringValue());
  EXPECT_TRUE(bool_var.BoolValue());
  EXPECT_EQ(1, bool_var.Int32Value());
  EXPECT_TRUE(std::fabs(bool_var.DoubleValue() - 1) <=
              std::numeric_limits<double>::epsilon());
}

// Make sure well-known string values convert correctly to bool types, and
// bool types convert to the known string values.
TEST_F(VariantTest, TypeConvertStringToBool) {
  Variant bool_var(true);
  EXPECT_EQ(Variant::kBoolVariantType, bool_var.variant_type());
  // Convert bool to string and back.
  Variant str_from_bool(bool_var.StringValue());
  EXPECT_EQ(0, strncmp(str_from_bool.StringValue().c_str(),
                       kTestBoolTrueString,
                       strlen(kTestBoolTrueString)));
  Variant bool_from_str(str_from_bool.BoolValue());
  EXPECT_TRUE(bool_from_str.BoolValue());
  // Step 2: Convert certain well-known strings to bools.
  Variant str_true("true");
  EXPECT_TRUE(str_true.BoolValue());
  Variant str_yes("Yes");
  EXPECT_TRUE(str_yes.BoolValue());
  Variant str_1("1");
  EXPECT_TRUE(str_1.BoolValue());
  Variant str_0("0");
  EXPECT_FALSE(str_0.BoolValue());
  Variant str_false("hello?");
  EXPECT_FALSE(str_false.BoolValue());
}

// Make sure the conversion from a double type to string works both ways.
TEST_F(VariantTest, TypeConverStringToDouble) {
  Variant dbl_var(kTestDoubleValue);
  EXPECT_EQ(Variant::kDoubleVariantType, dbl_var.variant_type());
  // Convert double to string and back.
  Variant str_from_dbl(dbl_var.StringValue());
  EXPECT_EQ(0, strncmp(str_from_dbl.StringValue().c_str(),
                       kTestDoubleAsString,
                       strlen(kTestDoubleAsString)));
  Variant dbl_from_str(str_from_dbl.DoubleValue());
  EXPECT_TRUE(std::fabs(dbl_from_str.DoubleValue() - kTestDoubleValue) <=
              std::numeric_limits<double>::epsilon());
}

// Make sure the coinversion from an int type to string works both ways.
TEST_F(VariantTest, TypeConvertIntToString) {
  Variant int_var(kTestIntValue);
  EXPECT_EQ(Variant::kInt32VariantType, int_var.variant_type());
  // Convert int to string and back.
  Variant str_from_int(int_var.StringValue());
  EXPECT_EQ(kTestIntAsString, str_from_int.StringValue());
  Variant int_from_str(str_from_int.Int32Value());
  EXPECT_EQ(kTestIntValue, int_from_str.Int32Value());
}
