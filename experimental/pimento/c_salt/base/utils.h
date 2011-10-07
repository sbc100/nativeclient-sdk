// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef C_SALT_BASE_UTILS_H_
#define C_SALT_BASE_UTILS_H_

#include <sys/time.h>

namespace c_salt {

namespace utils {

/// Get the time as an elapsed time from now. This function is used by many
/// c_salt function that take an absolute time out value.
///
/// @param usec_elapsed_time elapsed time expressed in microseconds.
typedef struct ::timespec TimeSpec;
TimeSpec GetTimeFromNow(time_t usec_elapse_time);

}  // namespace utils
}  // namespace c_salt

#endif  // C_SALT_BASE_UTILS_H_
