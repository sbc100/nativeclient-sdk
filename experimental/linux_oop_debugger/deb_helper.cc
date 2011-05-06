// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//----------------------debug_helper:end---------------------//
#include <memory.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
namespace debug_helper {
const int kOutputDebugStringSignal = SIGUSR1;
const int kOutputDebugStringSize = 4 * 1024;
void OutputDebugString_sigaction(int signo, siginfo_t* sig_inf, void* data) {}
bool OutputDebugString_Init() {
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = OutputDebugString_sigaction;
  int res = sigaction(kOutputDebugStringSignal, &act, NULL);
  return (0 == res);
}
bool OutputDebugString(const char* str) {
  static char static_buff[kOutputDebugStringSize + 1];
  size_t num = strlen(str) + 1;
  if (num > sizeof(static_buff))
    num = sizeof(static_buff);
  memcpy(static_buff, str, num);
  static_buff[num] = 0;
  sigval val;
  val.sival_ptr = static_buff;
  return (0 == sigqueue(getpid(), kOutputDebugStringSignal, val));
}
}  // namesdpace debug_helper
//----------------------debug_helper:end---------------------//

