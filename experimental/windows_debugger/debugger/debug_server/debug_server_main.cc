// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <conio.h>
#include "debugger/core/debug_logger.h"
#include "debugger/debug_server/debug_server.h"

int main(int argc, char **argv) {
  debug::TextFileLogger log;
  log.Open("debug_log.txt");
  //log.EnableStdout(true);
  debug::Logger::SetGlobalLogger(&log);

  const char* cmd = "D:\\chromuim_648_12\\src\\build\\Debug\\chrome.exe";
  //TODO(garianov) accept debugee name from command line

  int port = 4014;
  //TODO(garianov) accept port from command line

  debug::DebugServer debug_server;
  if (!debug_server.ListenOnPort(port)) {
    DBG_LOG("ERR20.01", "msg='debug_server.ListenOnPort failed' port=%d", port);
    return 1;
  }

  if (!debug_server.StartProcess(cmd, NULL)) {
    DBG_LOG("ERR20.02", "msg='debug_server.StartProcess failed' cmd='%s'", cmd);
    return 2;
  }

  while (true) {
    if (_kbhit()) {
      char cmd[200] = {0};
      gets_s(cmd, sizeof(cmd));
      cmd[sizeof(cmd) - 1] = 0;
      DBG_LOG("TR20.03", "user_command='%s'", cmd);
      if (0 == strcmp(cmd, "quit")) {
        debug_server.Quit();
        break;
      }
    }
    debug_server.DoWork(20);      
  }

  return 0;
}

