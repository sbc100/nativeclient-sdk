// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_TEST_UTILS_NACL_SDK_UTILS_H_
#define DEBUGGER_TEST_UTILS_NACL_SDK_UTILS_H_
#include <string>
#include "debugger/test_utils/process_utils.h"

namespace nacl_sdk_utils {
class Paths {
 public:
  Paths();

  std::string sdk_root() const {return sdk_root_;}
  std::string GetWebServerPath() const;
  std::string GetObjdumpPath() const;
  std::string GetAddrToLinePath() const;

 private:
  std::string sdk_root_;
  int arch_size_;
};

class WebServer {
 public:
  WebServer();

  bool Start();
  void Stop();

 private:
  process_utils::ProcessTree proc_tree_;
  HANDLE h_web_server_proc_;
};

}  // namespace nacl_sdk_utils

#endif  // DEBUGGER_TEST_UTILS_NACL_SDK_UTILS_H_

