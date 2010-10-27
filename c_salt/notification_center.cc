// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/notification_center.h"

#include "c_salt/notification.h"

namespace {
const int kPthreadMutexSuccess = 0;

// The class singleton and its mutex.
// TODO(c_salt_authors): This mutex should really be some kind of scoped
// mutex class, such as boost::mutex and boost::lock_guard.
pthread_mutex_t notification_center_mutex = PTHREAD_MUTEX_INITIALIZER;
c_salt::NotificationCenter* notification_center = NULL;
}  // namespace

namespace c_salt {

const char* const NotificationCenter::kAnonymousPublisherName = "";

NotificationCenter* NotificationCenter::DefaultCenter(
    const c_salt::Instance& instance) {
  // TODO(c_salt_authors): There needs to be one DefaultCenter() per
  // Instance.  We need to implement this.
  if (pthread_mutex_lock(&notification_center_mutex) != kPthreadMutexSuccess)
    return NULL;
  if (notification_center == NULL) {
    notification_center = new NotificationCenter();
  }
  pthread_mutex_unlock(&notification_center_mutex);
  return notification_center;
}

NotificationCenter::NotificationCenter() {
  pthread_mutex_init(&mutex_, NULL);
}

NotificationCenter::~NotificationCenter() {
  pthread_mutex_destroy(&mutex_);
}

bool NotificationCenter::AddSubscriberImpl(
    const std::string& notification_name,
    uint64_t subscriber_id,
    boost::function<void(const Notification&)> subscriber_functor,
    const std::string& publisher_name) {
  if (pthread_mutex_lock(&mutex_) != kPthreadMutexSuccess) {
    return false;
  }
  SharedNotificationSlot notification_slot;
  std::pair<NotificationDict::iterator, bool> slot_iter =
      notifications_.insert(NotificationDict::value_type(notification_name,
                                                         notification_slot));
  // If this is a newly inserted notification signal, then reset the
  // value with a new signal object.
  if (slot_iter.second) {
    slot_iter.first->second.reset(new NotificationSlot());
  }
  // Grab the actual NotificationSlot and hook up the subscriber.
  notification_slot = slot_iter.first->second;
  boost::signals2::connection connection;
  if (publisher_name.empty()) {
    connection = notification_slot->any_publisher_signal_
                 .connect(subscriber_functor);
  } else {
    // Add a new shared_ptr if there isn't one already.  The [] notation on
    // std::map doesn't help here.  This is to work around the lack of a copy
    // ctor on boost::signals2::signal.
    SharedNotificationSignal publisher_signal(new NotificationSignal());
    std::pair<PublisherSignalDict::iterator, bool> pub_sig_iter =
        notification_slot->publisher_signals_.insert(
            PublisherSignalDict::value_type(publisher_name, publisher_signal));
    connection = pub_sig_iter.first->second->connect(subscriber_functor);
  }
  notification_slot->connections_[subscriber_id].push_back(connection);
  pthread_mutex_unlock(&mutex_);
  return true;
}

bool NotificationCenter::RemoveSubscriberImpl(uint64_t subscriber_id) {
  if (pthread_mutex_lock(&mutex_) != kPthreadMutexSuccess) {
    return false;
  }
  NotificationDict::iterator note_iter = notifications_.begin();
  const NotificationDict::const_iterator end = notifications_.end();
  for (; note_iter != end; ++note_iter) {
    // Disconnect all the connections this subscriber might have for the
    // notification at |note_iter|.
    (note_iter->second)->DisconnectSubscriber(subscriber_id);
  }
  pthread_mutex_unlock(&mutex_);
  return true;
}

bool NotificationCenter::RemoveSubscriberFromNotificationImpl(
    uint64_t subscriber_id,
    const std::string& notification_name) {
  if (pthread_mutex_lock(&mutex_) != kPthreadMutexSuccess) {
    return false;
  }
  const NotificationDict::iterator note_iter =
      notifications_.find(notification_name);
  if (note_iter != notifications_.end()) {
    // Disconnect all the connections this subscriber might have for the
    // notification at |note_iter|.
    (note_iter->second)->DisconnectSubscriber(subscriber_id);
  }
  pthread_mutex_unlock(&mutex_);
  return true;
}

bool NotificationCenter::RemoveNotification(
    const std::string& notification_name) {
  if (pthread_mutex_lock(&mutex_) != kPthreadMutexSuccess) {
    return false;
  }
  notifications_.erase(notification_name);
  pthread_mutex_unlock(&mutex_);
  return true;
}

bool NotificationCenter::PublishNotification(
    const std::string& notification_name,
    const Notification& notification,
    const std::string& publisher_name) {
  if (pthread_mutex_lock(&mutex_) != kPthreadMutexSuccess) {
    return false;
  }
  const NotificationDict::const_iterator note_iter =
      notifications_.find(notification_name);
  SharedNotificationSlot notification_slot;
  SharedNotificationSignal publisher_signal;
  if (note_iter != notifications_.end()) {
    // Make a local copy of the signals, in the event that the
    // notification gets removed before signals can be fired (which will
    // make the iterator invalid).  The local copy of the NotificationSlot
    // shared_ptr manages |any_publisher_signal_|.
    notification_slot = note_iter->second;
    // See if there are any publisher-specific signals.  The signal itself is
    // managed with a shared_ptr, because boost::signals2::signal does not have
    // a copy ctor.
    PublisherSignalDict::const_iterator sig_iter =
        (note_iter->second)->publisher_signals_.find(publisher_name);
    if (sig_iter != (note_iter->second)->publisher_signals_.end()) {
      publisher_signal = sig_iter->second;
    }
  }
  // Signal the subscribers.  boost makes the actual signalling thread-safe, so
  // it is ok to unlock the mutex before signaling the subscribers.  Note that
  // it's possible for another thread to disconnect the signal before this gets
  // reached, in which case the signal will not fire.
  pthread_mutex_unlock(&mutex_);
  // Always fire the any-publisher matches.
  if (notification_slot != NULL) {
    (notification_slot->any_publisher_signal_)(notification);
  }
  // Fire any publisher-specific notifications.  If there were none, then this
  // signal set will be empty and signaling will have no effect.
  if (publisher_signal != NULL) {
    (*publisher_signal)(notification);
  }
  return true;
}

NotificationCenter::NotificationSlot::~NotificationSlot() {
  ConnectionDict::iterator conn_iter;
  for (conn_iter = connections_.begin();
       conn_iter != connections_.end();
       ++conn_iter) {
    DisconnectAll(conn_iter);
    // No need to erase(conn_iter), since this will be done as part of the
    // std::map<> dtor.
  }
}

void NotificationCenter::NotificationSlot::DisconnectSubscriber(
    uint64_t subscriber_id) {
  ConnectionDict::iterator conn_iter = connections_.find(subscriber_id);
  if (conn_iter != connections_.end()) {
    DisconnectAll(conn_iter);
    connections_.erase(conn_iter);
  }
}

void NotificationCenter::NotificationSlot::DisconnectAll(
    const ConnectionDict::iterator& conn_iter) {
  std::vector<boost::signals2::connection>::iterator signal_iter;
  for (signal_iter = conn_iter->second.begin();
      signal_iter != conn_iter->second.end();
      ++signal_iter) {
    // Note that ~connection() does *not* disconnect the signal, so this has
    // to be done manually in a loop.
    signal_iter->disconnect();
  }
}

}  // namespace c_salt
