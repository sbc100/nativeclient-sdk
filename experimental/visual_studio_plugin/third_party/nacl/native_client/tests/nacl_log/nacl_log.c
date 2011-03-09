/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
   A simple test that the NaClLog functionality is working.
*/

#include <stdio.h>
#include <nacl/nacl_log.h>

void hello_world() {
  NaClLog(LOG_INFO, "Hello, World!\n");
}

int main(int argc, char* argv[]) {
  NaClLogModuleInit();
  hello_world();
  NaClLogModuleFini();
  return 0;
}
