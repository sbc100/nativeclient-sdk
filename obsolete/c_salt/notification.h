// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_NOTIFICATION_H_
#define C_SALT_NOTIFICATION_H_

#include <map>
#include <string>

#include "boost/any.hpp"
#include "boost/shared_ptr.hpp"

namespace c_salt {

// A Notification encapsulates information so that it can be sent to observers
// by the NotificationCenter.  A Notification holds a name, a reference to the
// payload and the publisher name.  The payload is copied in the accessor. Note
// that the payload data object has to implement a thread-safe copy ctor.
class Notification {
 public:
  typedef boost::any NotificationValue;
  typedef boost::shared_ptr<NotificationValue> SharedNotificationValue;

  // Creates a Notification with an empty payload and an anonymous publisher.
  explicit Notification(const std::string& name)
      : name_(name),
        data_(new NotificationValue()),
        publisher_name_("") {}

  // Create a Notification with |data| as the payload and |publisher_name| as
  // the named publisher (set this to the empty string for the anonynmous
  // publisher).  This copies |data| into local storage.
  // Note: DataType must implement a thread-safe copy ctor.  For example, an
  // object that has STL containers in it might want to implement a deep-copy
  // in the copy ctor.
  Notification(const std::string& name,
               SharedNotificationValue data,
               const std::string& publisher_name)
      : name_(name),
        data_(data),
        publisher_name_(publisher_name) {}

  virtual ~Notification() {}

  const std::string& name() const {
    return name_;
  }

  // Return a copy of the internal payload.  This copy is made here so that no
  // other copies are needed when publishing this Notification.
  SharedNotificationValue data() const {
    return SharedNotificationValue(new NotificationValue(*data_));
  }
  void set_data(SharedNotificationValue data) {
    data_ = data;
  }

  const std::string& publisher_name() const {
    return publisher_name_;
  }
  void set_publisher_name(const std::string& publisher_name) {
    publisher_name_ = publisher_name;
  }

 private:
  std::string name_;
  SharedNotificationValue data_;
  std::string publisher_name_;

  Notification();  // Not implemented, do not use.
};
}  // namespace c_salt
#endif  // C_SALT_NOTIFICATION_H_
