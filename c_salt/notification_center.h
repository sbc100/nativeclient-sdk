// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_NOTIFICATION_CENTER_H_
#define C_SALT_NOTIFICATION_CENTER_H_

#include <pthread.h>

#include <cstddef>
#include <map>
#include <string>

#include "boost/noncopyable.hpp"
#include "boost/function.hpp"
#include "boost/signals2.hpp"
#include "c_salt/instance.h"
#include "c_salt/notification_ptrs.h"

namespace c_salt {
// NotificationCenter is a simple mechanism for broadcasting notifications
// within a process.
// Objects register with the NotificationCenter as subscribers of notifications
// using string identifiers to specify the notification of interest.
// Subscribers must subsequently remove themselves if they go out of scope
// before the NotificationCenter is deleted, or there will be dangling
// references.
class NotificationCenter : public boost::noncopyable {
 public:
  static const char* const kAnonymousPublisherName;

  // Return the common NotificationCenter used by |instance|.  c_salt objects
  // that post notifications all post to the DefaultCenter.  This call is
  // mutex locked.
  static NotificationCenter* DefaultCenter(const c_salt::Instance& instance);

  NotificationCenter();
  virtual ~NotificationCenter();

  // Add a subscriber with its method to invoke.  The subscriber doesn't have
  // to be created in the same thread as the NotificationCenter.  When a
  // notification gets published, however, the handler for the notification is
  // called from the publisher's thread.  The |subscriber| gets a copy of the
  // Notification data when |handler| is called, which can in turn be copied
  // to another thread.  If |publisher_name| is the enpty string, then
  // |subscriber| will receive notifications named |notification_name| from
  // any publisher.  Otherwise, it will only receive notifications from
  // calls to PublishNotification() when |publisher_name| matches.
  // Returns |true| on success.
  template <typename SubscriberType>
  bool AddSubscriber(const std::string& notification_name,
                     SubscriberType* subscriber,
                     void (SubscriberType::*handler)(const Notification&),
                     const std::string& publisher_name) {
    AddSubscriberImpl(notification_name,
                      CreateSubscriberId(subscriber),
                      boost::bind(handler, subscriber, _1),
                      publisher_name);
  }

  // Remove a subscriber from all notifications in the dispatch table.
  // Returns |true| on success.
  template <typename SubscriberType>
  bool RemoveSubscriber(SubscriberType* subscriber) {
    RemoveSubscriberImpl(CreateSubscriberId(subscriber));
  }

  // Remove a subscriber from the specified notification.  Returns |true| on
  // success.
  template <typename SubscriberType>
  bool RemoveSubscriberFromNotification(
      SubscriberType* subscriber,
      const std::string& notification_name) {
    RemoveSubscriberFromNotificationImpl(CreateSubscriberId(subscriber),
                                         notification_name);
  }
  // Remove all subscribers from a specified notification and then remove the
  // notification.  Future calls to PostNotification() to |notification_name|
  // will have no effect.  Returns |true| on success.
  bool RemoveNotification(const std::string& notification_name);

  // Publish a notification to be dispatched, specifying notification-specific
  // data.  Publishing all happens on a single thread, all subscribers are
  // signaled on the calling thread.  To further process a notification on
  // another thread you have to do the inter-thread communication yourself.
  // Returns |true| on success.
  bool PublishNotification(const std::string& notification_name,
                           const Notification& notification,
                           const std::string& publisher_name);

 private:
  typedef boost::signals2::signal<void(const Notification&)> NotificationSignal;
  typedef boost::shared_ptr<NotificationSignal> SharedNotificationSignal;
  // Map publisher ids to a set of boost::signals.  This map is used when
  // publishing a notification.  Note that boost::signals2::signal<> has no
  // copy ctor, so this dictionary has to use shared_ptrs.
  typedef std::map<std::string, SharedNotificationSignal> PublisherSignalDict;
  typedef std::map<uint64_t, std::vector<boost::signals2::connection> >
      ConnectionDict;

  // A small helper class that holds the signal and the function which gets
  // fired during PublishNotification().  A notification is published by firing
  // a signal; there are two possible signals, one that is always fired which
  // publishes a notification to subscribers of any publisher, and then another
  // signal which is fired only if the publisher's id exists in the publisher
  // ==> signal dictionary.  In this way, every subscriber that wants
  // notifications from either any publisher or from a specific publisher will
  // get notified.  Each NotificationSlot also holds a dictionary which maps
  // subscriber_id ==> vector<connections>.  This is done to support the
  // RemoveSubscriber(SubscriberType) and to support disconnection of a
  // boost::signals2::signal without relying on the peculiar equality semantics
  // of boost::function in boost::signals.
  struct NotificationSlot {
    // Dtor disconnects all the signals in |connections_|.  Assumes that any
    // necessary mutex locking has been done by the caller.
    ~NotificationSlot();

    // Disconnect all the signals connected to the subscriber represented by
    // this NotificationSlot.  Assumes that any necessary mutex locking has
    // been done by the caller.
    void DisconnectSubscriber(uint64_t subscriber_id);
    void DisconnectAll(const ConnectionDict::iterator& conn_iter);

    // This signal set is fired every time PublishNotification() is called,
    // and will notify all subscribers to any publisher.
    NotificationSignal any_publisher_signal_;
    // Fire the signal set from this dictionary that matches |publisher_id_|.
    PublisherSignalDict publisher_signals_;
    ConnectionDict connections_;
  };
  typedef boost::shared_ptr<NotificationSlot> SharedNotificationSlot;
  typedef std::map<std::string, SharedNotificationSlot> NotificationDict;

  // Return a unique hash id for |subscriber|.  This function has to return a
  // unique id for each subscriber.
  template <typename SubscriberType>
  uint64_t CreateSubscriberId(SubscriberType* subscriber) const {
    return reinterpret_cast<uint64_t>(subscriber);
  }

  // The implementation behind the template methods.
  bool AddSubscriberImpl(
      const std::string& notification_name,
      uint64_t subscriber_id,
      boost::function<void(const Notification&)> subscriber_functor,
      const std::string& publisher_name);
  bool RemoveSubscriberImpl(uint64_t subscriber_id);
  bool RemoveSubscriberFromNotificationImpl(
      uint64_t subscriber_id,
      const std::string& notification_name);

  mutable pthread_mutex_t mutex_;
  NotificationDict notifications_;
};
}  // namespace c_salt
#endif  // C_SALT_NOTIFICATION_CENTER_H_
