#include <stdio.h>
#include <stdlib.h>

#include "debug_conn/debug_host.h"
#include "debug_conn/debug_target.h"
#include "nacl_symbols.h"

using namespace NaCl;
using nacl_debug_conn::DebugHost;
using nacl_debug_conn::DebugTargetRegsX86_64_t;

void printit(void *obj, const char *str) {
  printf("start %s:\n%s\nend %s\n\n", obj, str, obj);
}

void __stdcall FillCharPP(DebugHost::DHResult res, void *obj, const char *str) {
  char **ppstr = (char **) obj;
  *ppstr = strdup(str);
}

void __stdcall StopHit(DebugHost::DHResult res, void *obj) {
  uint32_t *val = (uint32_t *) obj;
  printf("Stop triggered.\n");
  *val = 1;
}

void DumpState(DebugHost *host, ISymbolMap *map) {
  DebugTargetRegsX86_64_t regs;
  uint32_t a,b;
  const char *src;
  uint32_t line;

  printf("\nThread INFO:\n");
  host->GetRegisters(&regs, sizeof(regs) - sizeof(regs.pad));
  for (a=0; a < 4; a++) {
    for (b=0; b < 4; b++) {
      uint32_t idx = b + a* 4;
      printf("\t%s %16X", nacl_debug_conn::RegisterIndexToName(idx), regs.IntRegs[idx]);
    }
    printf("\n");
  }
  src = map->AddrToLocation((uint32_t )regs.Rip, &line);
  printf("[%09llXh] %s(%d)\n\n", regs.Rip, src, line);
};


int main(int argc, const char *argv[])
{
  //char *name = "..\\..\\third_party\\nacl\\native_client\\scons-out\\nacl-x86-64\\staging\\noop.nexe";
  char *name = "..\\loop\\loop.nexe";
  ISymbolMap *map;
  AutoStr str;
  char *path;
  int errors = 0;

  DebugHost *host = DebugHost::SocketConnect("127.0.0.1:4014");

  if (!host) {
    printf("Failed to connect to host.\n");
    return -1;
  }

  host->GetPathAsync(FillCharPP, &path);
  if (NULL == (map = (ISymbolMap *) SymbolMapFactory(&path[0]))) {
    printf("Failed to load symbols.\n");
    return -1;
  }

  map->ModuleXML(&str);
//  printit("get module", str.GetStr());
//  map->VariablesToXML(&str);


  int val;
 
  // Verify we are NOT broken.
  host->GetLastSig(&val);
  printf ("Last signal is: %d\n", val);
  if (val != 0) {
    printf("Should not have been broken already (0).\n");
    errors++;
  }

  // Verify we CAN break.
  host->RequestBreak();
  host->GetLastSig(&val);
  printf ("Last signal is: %d\n", val);
  if (val != 255) {
    printf("Last signal should have been requested stop (255).\n");
    errors++;
  }
  DumpState(host, map);

  // Compute a good breakpoint location (in nexe not service runtime
  uint64_t addr = map->LocationToAddr("loop.cc", 24);
  addr += 0xC00000000;
  printf("Adding breakpoint at: %llx\n", addr);

  // Set the breakpoint and make sure we can blocking break.
  host->AddBreakpoint(addr);
  host->RequestContinue();
  host->GetLastSig(&val);
  printf ("Last signal is: %d\n", val);
  if (val != 5) {
    printf("Last signal should have been breakpoint (5).\n");
    errors++;
  }

  // Verify we STILL break since we didn't take off the breakpoint
  host->RequestContinue();
  host->GetLastSig(&val);
  printf ("Last signal is: %d\n", val);
  if (val != 5) {
    printf("Last signal should have been breakpoint (5).\n");
    errors++;
  }
  DumpState(host, map);

  // Disable the breakpoint and verify we are NOT broken
  host->DisableBreakpoint(addr);
  host->RequestContinueBackground();
  printf("\n DISABLED BREAK, CONTINUE FOR EVER.\n\n");
  Sleep(1000); 
  host->GetLastSig(&val);
  printf ("Last signal is: %d\n", val);
  if (val != 0) {
    printf("Last signal should have been none.\n");
    errors++;
  }
  DumpState(host, map);

  // Incase you want to watch it run...
  printf("Waiting for 5 sec before inserting breakpoint.\n");

  // Set stop packet callback
  uint32_t hit = 0;
  host->SetStopAsync(StopHit, &hit);
  DumpState(host, map);

  Sleep(5000);
  // Add breakpoint interactively and watch it trigger.
  host->EnableBreakpoint(addr);
  
  // Wait a sec for it to trigger
  Sleep(1000);  
  host->GetLastSig(&val);

  // Verify our callback got called
  if (!hit) {
    printf("Failed to hit callback.\n");
    errors++;
  }
  DumpState(host, map);

  // Verify we got a breakpoint.
  printf ("Last signal is: %d\n", val);
  if (val != 5) {
    printf("Last signal should have been breakpoint (5).\n");
    errors++;
  }

  // We should now be able to exit and rerun.
  host->RemoveBreakpoint(addr);
  host->RequestContinueBackground();

  map->Free();
  if (errors) {
    printf("Found %d errors!\n", errors);
    return -1;
  }
  return 0;
}
  
