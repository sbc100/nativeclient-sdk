// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/notification.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
const char* const kNotificationName = "notification";
const char* const kOtherNotificationName = "other notification";
const char* const kPublisher1 = "publisher1";
const int kIntValue42 = 42;
}  // namespace

using c_salt::Notification;

class NotificationTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}
};

// Verify that the default ctor caches the name,and produces an anonymous
// notification with an empty payload.
TEST_F(NotificationTest, DefaultCtor) {
  Notification note(kNotificationName);
  EXPECT_EQ(kNotificationName, note.name());
  EXPECT_TRUE(note.data()->empty());
  EXPECT_EQ("", note.publisher_name());
}

TEST_F(NotificationTest, IntCtor) {
  Notification::SharedNotificationValue
      int_value(new Notification::NotificationValue(kIntValue42));
  Notification note(kNotificationName, int_value, kPublisher1);
  EXPECT_FALSE(note.data()->empty());
  EXPECT_EQ(kIntValue42, boost::any_cast<int>(*note.data()));
}
