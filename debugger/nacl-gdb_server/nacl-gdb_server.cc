// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <conio.h>
#include "debugger/base/debug_command_line.h"
#include "debugger/core/debug_logger.h"
#include "debugger/debug_server/debug_server.h"

int main(int argc, char **argv) {
  debug::CommandLine command_line(argc, argv);

  debug::TextFileLogger log;
  log.Open("debug_log.txt");
  debug::Logger::SetGlobalLogger(&log);

  std::string cmd = command_line.GetStringSwitch("debuggee", "");
  if (0 == cmd.size()) {
    printf("Error: program to debug shall be specified with \"--debuggee\" switch.");
    return 1;
  }
  int port = command_line.GetIntSwitch("port", 4014);

  debug::DebugAPI debug_api;
  debug::DebugServer debug_server(&debug_api);
  if (!debug_server.ListenOnPort(port)) {
    DBG_LOG("ERR101.01", "msg='debug_server.ListenOnPort failed' port=%d", port);
    return 1;
  }

  if (!debug_server.StartProcess(cmd.c_str(), NULL)) {
    DBG_LOG("ERR101.02", "msg='debug_server.StartProcess failed' cmd='%s'", cmd);
    return 2;
  }

  DBG_LOG("TR101.03", "msg='Debug server started' port=%d cmd='%s'", port, cmd);
  while (true) {
    if (_kbhit()) {
      char cmd[200] = {0};
      gets_s(cmd, sizeof(cmd));
      cmd[sizeof(cmd) - 1] = 0;
      DBG_LOG("TR101.04", "user_command='%s'", cmd);
      if (0 == strcmp(cmd, "quit")) {
        debug_server.Quit();
        break;
      }
    }
    debug_server.DoWork(20);      
    if (debug_server.ProcessExited()) {
      printf("Exit?");
      getchar();
      break;
    }
  }
  DBG_LOG("TR101.05", "msg='Debugger stopped'");
  return 0;
}
