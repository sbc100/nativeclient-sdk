// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <conio.h>
#include "debugger/base/debug_command_line.h"
#include "debugger/core/debug_logger.h"
#include "debugger/nacl-gdb_server/debug_server.h"

namespace {
const int kErrNoProgramSpecified = 1;
const int kErrListenOnPortFailed = 2;
const int kErrStartProcessFailed = 3;
const int kErrDebugServerInitFailed = 4;

const int kDefaultPort = 4014;
const int kErrorBuffSize = 2000;
const int kWaitForDebugEventMilliseconds = 20;

const char* kVersionString = "nacl-gdb_server v0.003";
#ifdef _WIN64
const char* kBitsString = "64-bits";
#else
const char* kBitsString = "32-bits";
#endif
const char* kHelpString =
    "Usage: nacl-gdb_server [options] --program \"program to debug\"\n"
    "Options:\n"
    "    --port <number> : port to listen for a TCP connection\n"
    "    --cm  : compatibility mode\n\n"
    "Example:\n"
    "nacl-gdb_server --port 4014 --program \"c:\\chrome.exe --no-sandbox\"\n"
    "Note: there's no need to specify --no-sandbox flag.\n"
    "Type Ctrl-C or 'quit' to exit.";

std::string GetLastErrorDescription(int error_code = 0) {
  if (0 == error_code)
    error_code = ::GetLastError();
  char buff[kErrorBuffSize] = {'?', 0};
  ::FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                   0,
                   error_code,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buff,
                   sizeof(buff),
                   0);
  return std::string(buff);
}
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
  bool compatibility_mode = command_line.HasSwitch("-cm");

  debug::TextFileLogger log;
  log.Open("debug_log.txt");
  debug::Logger::SetGlobalLogger(&log);

  debug::DebugAPI debug_api;
  debug::DebugServer debug_server(&debug_api, port);

  if (compatibility_mode)
    debug_server.EnableCompatibilityMode();

  if (!debug_server.Init()) {
    DBG_LOG("ERR101.01", "msg='debug_server.Init failed'");
    return kErrDebugServerInitFailed;
  }

  if (!debug_server.StartProcess(cmd.c_str(), NULL)) {
    std::string sys_err = GetLastErrorDescription();
    DBG_LOG("ERR101.03",
            "msg='gdb_server.StartProcess failed' 'cmd=[%s]' err='%s'",
            cmd.c_str(),
            sys_err.c_str());
    printf("StartProcess failed cmd='[%s]' err='%s'",
            cmd.c_str(),
            sys_err.c_str());
    return kErrStartProcessFailed;
  }

  DBG_LOG("TR101.04", "msg='Debug server started' port=%d cmd='%s'", port, cmd);
  while (true) {
    if (_kbhit()) {
      char cmd[200] = {0};
      gets_s(cmd, sizeof(cmd));
      cmd[sizeof(cmd) - 1] = 0;
      DBG_LOG("TR101.05", "user_command='%s'", cmd);
      if (0 == strcmp(cmd, "quit")) {
        debug_server.Quit();
        break;
      }
    }
    debug_server.DoWork(kWaitForDebugEventMilliseconds);
    if (debug_server.ProcessExited()) {
      printf("Exit?");
      getchar();
      break;
    }
  }
  DBG_LOG("TR101.06", "msg='Debugger stopped'");
  return 0;
}

