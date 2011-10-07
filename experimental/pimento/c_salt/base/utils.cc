// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "c_salt/base/utils.h"

namespace c_salt {
namespace utils {

TimeSpec GetTimeFromNow(time_t usec_elapse_time) {
  static const time_t kNanoSecPerMicroSec = 1000;
  static const time_t kMicroSecPerSec = 1000000;
  struct timeval now;
  gettimeofday(&now, NULL);

  TimeSpec time_out;
  time_out.tv_sec = now.tv_sec + (usec_elapse_time / kMicroSecPerSec);
  time_out.tv_nsec = (now.tv_usec + (usec_elapse_time % kMicroSecPerSec)) *
                     kNanoSecPerMicroSec;
  time_out.tv_sec += time_out.tv_nsec / kNanoSecPerMicroSec;
  time_out.tv_nsec %= kNanoSecPerMicroSec;
  return time_out;
}

}  // namespace utils
}  // namespace c_salt
