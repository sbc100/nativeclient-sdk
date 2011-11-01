// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/test_utils/nacl_sdk_utils.h"
#include "debugger/test_utils/sys_utils.h"

namespace {
const char* kWebServerPath = "\\examples\\httpd.cmd";
const char* kBinutilsPath = "\\toolchain\\win_x86\\bin\\";

#ifdef _WIN64
const int kDefaultArchSize = 64;
const char* kBinutilsPrefix = "x86_64";
#else
const int kDefaultArchSize = 32;
const char* kBinutilsPrefix = "i686";
#endif
}  // namespace

namespace nacl_sdk_utils {
Paths::Paths() {
  sdk_root_ = sys_utils::GetStringEnvVar("NACL_SDK_ROOT", "");
  arch_size_ = sys_utils::GetIntEnvVar("ARCH_SIZE", kDefaultArchSize);
}

std::string Paths::GetWebServerPath() const {
  return sdk_root_ + kWebServerPath;
}

std::string Paths::GetObjdumpPath() const {
  return sdk_root_ + kBinutilsPath + kBinutilsPrefix + "-nacl-objdump.exe";
}

std::string Paths::GetAddrToLinePath() const {
  return sdk_root_ + kBinutilsPath + kBinutilsPrefix + "-nacl-addr2line.exe";
}

WebServer::WebServer()
    : h_web_server_proc_(NULL) {
}

bool WebServer::Start() {
  Paths sdk_paths;
  std::string web_server = sdk_paths.GetWebServerPath();
  std::string path = sys_utils::DirFromPath(web_server);

  h_web_server_proc_ = proc_tree_.StartProcess(web_server, path.c_str());
  return (NULL != h_web_server_proc_);
}

void WebServer::Stop() {
  if (NULL != h_web_server_proc_) {
    proc_tree_.KillProcessTree(h_web_server_proc_);
    h_web_server_proc_ = NULL;
  }
}

}  // namespace nacl_sdk_utils

