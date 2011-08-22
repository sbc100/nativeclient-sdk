// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stdio.h>
#include "debugger/base/debug_command_line.h"
#include "debugger/core/debug_api.h"
#include "debugger/nacl-gdb_server/debug_server.h"

namespace {
const int kErrNoProgramSpecified = 1;
const int kErrListenOnPortFailed = 2;
const int kErrStartProcessFailed = 3;
const int kErrDebugServerInitFailed = 4;

const int kDefaultPort = 4014;
const int kErrorBuffSize = 2000;
const int kWaitForDebugEventMilliseconds = 20;

const char* kVersionString = "nacl-gdb_server v0.002";
#ifdef _WIN64
const char* kBitsString = "64-bits";
#else
const char* kBitsString = "32-bits";
#endif
const char* kHelpString =
    "Usage: nacl-gdb_server [options] --program \"program to debug\"\n"
    "Options:\n"
    "    --port <number> : port to listen for a TCP connection\n"
    "Example:\n"
    "nacl-gdb_server --port 4014 --program \"c:\\chrome.exe --no-sandbox\"\n"
    "Note: there's no need to specify --no-sandbox flag.\n"
    "Type Ctrl-C or 'quit' to exit.";
}  // namespace

int main(int argc, char **argv) {
  printf("%s %s\n\n", kVersionString, kBitsString);

  debug::CommandLine command_line(argc, argv);
  if ((0 == command_line.GetParametersNum()) || command_line.HasSwitch("-h")) {
    printf("%s\n", kHelpString);
    return 0;
  }
  std::string cmd = command_line.GetStringSwitch("-program", "");
  if (0 == cmd.size()) {
    printf("Error: program to debug shall be specified with "
           "\"--program\" switch.");
    return kErrNoProgramSpecified;
  }
  int port = command_line.GetIntSwitch("-port", kDefaultPort);

  debug::DebugAPI debug_api;
  debug::DebugServer debug_server(&debug_api, port);

  if (!debug_server.Init()) {
    printf("ERR101.01: msg='debug_server.Init failed'");
    return kErrDebugServerInitFailed;
  }

  pid_t pid = 0;
  if (!debug_api.StartProcess(cmd.c_str(), true, &pid)) {
    printf("ERR101.03: "
           "msg='gdb_server.StartProcess failed' 'cmd=[%s]'",
            cmd.c_str());
    return kErrStartProcessFailed;
  }

  printf("TR101.04: msg='Debug server started' port=%d cmd='%s'", port, cmd.c_str());
  while (true)
    debug_server.DoWork();
  return 0;
}

