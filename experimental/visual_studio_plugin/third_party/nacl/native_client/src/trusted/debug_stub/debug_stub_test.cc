/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <string>
#include <vector>

#include <stdio.h>
#include "native_client/src/trusted/debug_stub/debug_target.h"
#include "native_client/src/trusted/debug_stub/debug_host.h"
#include "native_client/src/trusted/debug_stub/debug_thread.h"


using namespace nacl_debug_conn;

using std::string;
using std::vector;


const char *HostErrorStr(int num) {
  switch (num) {
    case DebugHost::DHR_FAILED:
      return "Transaction completed with failure";

    case DebugHost::DHR_LOST:
      return "Lost connection during transaction";

    case DebugHost::DHR_TIMEOUT:
      return "Transaction Timed out";

    case DebugHost::DHR_OK:
      return "Transaction Succeeded";
  }
  return "Unknown";
}


void  OutputResult(DebugHost::DHResult res, void *obj, const char *str) {
  if (res != DebugHost::DHR_OK)
    printf("%s failed: %s\n", obj, HostErrorStr(res));
  else
    printf("%s\n%s\n\n", obj, str);
}


void  OutputRegs(DebugHost::DHResult res, void *obj, void *block, uint32_t len) {
  DebugTargetRegsX86_64_t *regs = static_cast<DebugTargetRegsX86_64_t *>(block);

  if (res == DebugHost::DHR_OK) {
    int loop;
    printf("Got Register block size %d.\n", len);
    for (loop=0; loop < 16; loop++)
      printf("\tR%02d = %x\n", loop, regs->IntRegs[loop]);
     printf("\tRIP = %x\n", regs->IntRegs[16]);
  }
  else
    printf("%s : %s\n", obj, HostErrorStr(res));
}


int DebugInstTest() {
  int failed = 1;
  int errors = 0;
  const char *addr = "127.0.0.1:4014";

  DebugHost::DHResult res;
  DebugHost *host = DebugHost::SocketConnect(addr);
  if (NULL == host) {
    printf("Host failed to connect to server socket at '%s'.\n", addr);
    errors++;
    goto DebugInstFailed;
  }

  int sig = 0;
  res = host->GetLastSig(&sig);
  if (res != DebugHost::DHR_OK) {
    printf("Failed to get expected signal.\n", 
      HostErrorStr(res));
    errors++;
    goto DebugInstFailed;
  }

#if 0
  // Read the registers
  DebugTargetRegsX86_64_t ctx;
  uint32_t len = sizeof(ctx) - sizeof(ctx.pad);
  res = host->GetRegisters(OutputRegs, "Get start regs", &ctx, len);
  if (res != DebugHost::DHR_OK) {
    printf("Host failed to get registers, error %s.\n", 
      HostErrorStr(res));
    errors++;
    goto DebugInstFailed;
  }
#endif

  host->GetArchAsync(OutputResult, "GetArch");
  host->GetPathAsync(OutputResult, "GetPath");
  host->GetThreadsAsync(OutputResult, "GetThreads");

  host->RequestBreak(NULL, NULL);
  host->RequestContinue(NULL, NULL);
  host->RequestBreak(NULL, NULL);

#if 0
  printf("Continuing...\n");
  res = host->RequestContinue();
  printf("Breaking...\n");
  res = host->RequestBreak();
  Sleep(5000);
  res = host->RequestContinue();
#endif

DebugInstFailed:
  if (host)
    delete host;

  return errors;
}



int main(int argc, char* argv[]) {
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);
  int errors = 0;

//  NaClDebugStubInit();   

  errors += DebugInstTest();

//  NaClDebugStubFini();

  if (errors == 0)
  {
    printf("PASS\n");    
    return 0;
  } 

  printf("FAILED\n");
  return -1;
}
