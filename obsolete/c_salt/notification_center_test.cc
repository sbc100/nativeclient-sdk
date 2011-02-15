// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <iostream>
#include <string>

#include "c_salt/instance.h"
#include "c_salt/notification_center.h"
#include "c_salt/notification.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using c_salt::NotificationCenter;
using c_salt::Notification;

namespace {
const char* const kPublisherName = "publisher name";
const char* const kPublisherName2 = "publisher name 2";
const char* const kTestNotification = "Test Notification";
const char* const kTestNotification2 = "Test Notification 2";
}  // namespace

// A dummy class that implements a notification response method.  This method
// is expected to be called from a notification.
class Subscriber {
 public:
  Subscriber() : call_count_(0), call_count2_(0) {}
  void NotificationHandler(const Notification& notification) {
    ++call_count_;
  }

  void NotificationHandler2(const Notification& notification) {
    ++call_count2_;
  }

  int call_count() const {
    return call_count_;
  }

  int call_count2() const {
    return call_count2_;
  }

 private:
  int call_count_;
  int call_count2_;
};

class NotificationCenterTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}
};

// Verify the default center is the same all the time.
TEST_F(NotificationCenterTest, DefaultCenter) {
  c_salt::Instance instance(NULL);
  NotificationCenter* default_center_1 =
      NotificationCenter::DefaultCenter(instance);
  NotificationCenter* default_center_2 =
      NotificationCenter::DefaultCenter(instance);
  EXPECT_EQ(default_center_1, default_center_2);
}

// Add a subscriber and verify that it gets called when a notification is
// posted.
TEST_F(NotificationCenterTest, PublishNotification) {
  Subscriber subscriber;
  NotificationCenter center;
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(1, subscriber.call_count());
}

// Add a couple of subscribers and verify that they both get called when a
// notification is posted.
TEST_F(NotificationCenterTest, Post2Notifications) {
  Subscriber subscriber;
  Subscriber subscriber2;
  NotificationCenter center;
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.AddSubscriber(kTestNotification,
                       &subscriber2,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(1, subscriber.call_count());
  EXPECT_EQ(1, subscriber2.call_count());
}

// Verify that a subscriber can listen for notifications from a specific object.
TEST_F(NotificationCenterTest, SpecificPublisher) {
  Subscriber subscriber;
  NotificationCenter center;
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       kPublisherName);
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler2,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(1, subscriber.call_count());
  EXPECT_EQ(1, subscriber.call_count2());
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName2);
  EXPECT_EQ(1, subscriber.call_count());  // Should not increment.
  // Subscribes to kTestNotifications from *any* publisher, so the call count
  // should increment.
  EXPECT_EQ(2, subscriber.call_count2());
}

// Verify that subscribers can be added after an initial post.
TEST_F(NotificationCenterTest, AddSubscriber) {
  Subscriber subscriber;
  NotificationCenter center;
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.AddSubscriber(kTestNotification2,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  center.PublishNotification(kTestNotification2,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(2, subscriber.call_count());
  Subscriber subscriber2;
  center.AddSubscriber(kTestNotification,
                       &subscriber2,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(3, subscriber.call_count());
  EXPECT_EQ(1, subscriber2.call_count());
}

// Observe two notifications with the same subscriber, then remove one of the
// notificaiton handlers and verify that the second one continues to function.
// This depends on the AddSubscriber test.
TEST_F(NotificationCenterTest, RemoveSubscriberFromNotification) {
  Subscriber subscriber;
  Subscriber subscriber2;
  NotificationCenter center;
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.AddSubscriber(kTestNotification2,
                       &subscriber,
                       &Subscriber::NotificationHandler2,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  center.PublishNotification(kTestNotification2,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(1, subscriber.call_count());
  EXPECT_EQ(1, subscriber.call_count2());
  center.RemoveSubscriberFromNotification(&subscriber, kTestNotification);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  center.PublishNotification(kTestNotification2,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(1, subscriber.call_count());  // Should not increment.
  // Still observes |kTestNotification|.
  EXPECT_EQ(2, subscriber.call_count2());
}

// Verify that subscribers can be removed.  This depends on the AddSubscriber
// test.
TEST_F(NotificationCenterTest, RemoveSubscriber) {
  Subscriber subscriber;
  Subscriber subscriber2;
  NotificationCenter center;
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.AddSubscriber(kTestNotification,
                       &subscriber2,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.AddSubscriber(kTestNotification2,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  center.PublishNotification(kTestNotification2,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(2, subscriber.call_count());
  EXPECT_EQ(1, subscriber2.call_count());
  // Remove |subscriber| from the notifications it observes.
  center.RemoveSubscriber(&subscriber);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  // There are no more subscribers of |kTestNotification2|.
  center.PublishNotification(kTestNotification2,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(2, subscriber.call_count());  // Should not increment.
  // Still observes |kTestNotification|.
  EXPECT_EQ(2, subscriber2.call_count());
  // Remove all remaining subscribers of |kTestNotification|.
  center.RemoveSubscriberFromNotification(&subscriber2,
                                        kTestNotification);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(2, subscriber.call_count());  // Should not increment.
  EXPECT_EQ(2, subscriber2.call_count());  // Should not increment.
}

// Remove a notification, verify that all its subscribers get disconnected.
TEST_F(NotificationCenterTest, RemoveNotification) {
  Subscriber subscriber;
  Subscriber subscriber2;
  NotificationCenter center;
  center.AddSubscriber(kTestNotification,
                       &subscriber,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.AddSubscriber(kTestNotification,
                       &subscriber2,
                       &Subscriber::NotificationHandler,
                       NotificationCenter::kAnonymousPublisherName);
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  // Make sure the connections are made and functioning.
  EXPECT_EQ(1, subscriber.call_count());
  EXPECT_EQ(1, subscriber2.call_count());
  center.RemoveNotification(kTestNotification);
  // Make sure no further handling of |kTestNotification| happens.
  center.PublishNotification(kTestNotification,
                             Notification(kTestNotification),
                             kPublisherName);
  EXPECT_EQ(1, subscriber.call_count());  // Should not increment.
  EXPECT_EQ(1, subscriber2.call_count());
}

