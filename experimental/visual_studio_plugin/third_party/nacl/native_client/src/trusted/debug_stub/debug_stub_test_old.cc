/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#include <stdio.h>

#include "native_client/src/include/portability.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/shared/platform/nacl_time.h"

#include "native_client/src/trusted/debug_stub/debug_host.h"
#include "native_client/src/trusted/debug_stub/debug_inst.h"
#include "native_client/src/trusted/debug_stub/debug_stub_api.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"
#include "native_client/src/trusted/debug_stub/debug_stub_api.h"
#include "native_client/src/trusted/debug_stub/debug_thread.h"

using namespace nacl_debug_conn;
using namespace nacl_debug_stub;

using std::string;

static void *context[16] = { 
	(void *) 0, (void *) 1, (void *) 2, (void *) 3, 
	(void *) 4, (void *) 5, (void *) 6, (void *) 7,
	(void *) 8, (void *) 9, (void *) 10, (void *) 11,
	(void *) 12, (void *) 13, (void *) 14, (void *) 15
};

volatile int doneTest = 0;



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


NaClThread *ThreadCreate(void (WINAPI *start_fn)(void *), void *state) {
  NaClThread *thread = new NaClThread;
  if (NaClThreadCtor(thread, start_fn, state, 65536))
    return thread;

  delete thread;
  return NULL;
}


void WaitMS(int ms) {
  nacl_abi_timespec tm;
  tm.tv_sec  = ms / 1000;
  tm.tv_nsec = (ms  % 1000)* 1000000;

  NaClNanosleep(&tm, &tm);
}


void WINAPI BogusThread(void *) {
  char *p = 0;
  *p = 0;
}

void WINAPI TargetServer(void *) {
  const char *addr = "0.0.0.0:4014";

  DebugInst *target = new DebugInst("InstTest");
  if (false == target->PrepSocketServer("0.0.0.0:4014")) {
    printf("Inst failed to prep server socket at '%s'.\n", addr);
    return;
  }

  DebugThread *thread = new DebugThread();
  
  thread->start_fn = BogusThread;
  thread->cookie = 0;
  thread->regSize_ = sizeof(intptr_t) * 16;
  thread->registers_ = malloc(thread->regSize_);
  thread->ntp_ = static_cast<NaClThread*>(malloc(sizeof(NaClThread)));

  NaClThreadCtor(thread->ntp_, NaClDebugStubThreadStart, thread, 65536);
  target->AddThread(thread);

  while(!doneTest) {
    if (target->GetPendingClient()) {
      printf("GOT client!\n");
      while (!doneTest) {
        if (target->PacketPump() == DS_ERROR)
          break;

        WaitMS(100);
      }
    }
  } 
  
  delete target;
}




int DebugInstTest() {
  int failed = 1;
  int errors = 0;
  const char *addr = "127.0.0.1:4014";
  char *block = new char[4096];

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
  if (sig != 11) {
    printf("Expecting signal %s(11), got %s(%d).\n",
            NaClDebugSigToStr(11), 
            NaClDebugSigToStr(sig), 
            sig);
  }

  // Read memory at 'context'
  memset(block, 0, sizeof(context));
  res = host->GetMemory((intptr_t) context, block, sizeof(context));
  if (res != DebugHost::DHR_OK) {
    printf("Host failed to get registers, error %s.\n", 
      HostErrorStr(res));
    errors++;
    goto DebugInstFailed;
  }
  if (memcmp(block, context, sizeof(context))) {
    printf("Failed compare on register read.\n");
  }

  // Write it back
  memset(context, 0, sizeof(context));
  res = host->SetMemory((intptr_t) context, block, sizeof(context));
  if (res != DebugHost::DHR_OK) {
    printf("Host failed to set memory, error %s.\n", 
      HostErrorStr(res));
    errors++;
    goto DebugInstFailed;
  }
  if (memcmp(block, context, sizeof(context))) {
    printf("Failed compare on register write.\n");
  }

  // Read the registers
  memset(block, 0, sizeof(context));
  res = host->GetRegisters(block, sizeof(context));
  if (res != DebugHost::DHR_OK) {
    printf("Host failed to get registers, error %s.\n", 
      HostErrorStr(res));
    errors++;
    goto DebugInstFailed;
  }
  if (memcmp(block, context, sizeof(context))) {
    printf("Failed compare on register read.\n");
  }

  // Now set them back
  memset(context, 0, sizeof(context));
  res = host->SetRegisters(block, sizeof(context));
  if (res != DebugHost::DHR_OK) {
    printf("Host failed to set registers, error %s.\n", 
      HostErrorStr(res));
    errors++;
    goto DebugInstFailed;
  }
  if (memcmp(block, context, sizeof(context))) {
    printf("Failed compare on register write.\n");
  }

DebugInstFailed:
  if (host)
    delete host;

  if (block)
    delete[] block;

  return errors;
}



#if 0
#include <stdio.h> 
#include <windows.h> // for EXCEPTION_ACCESS_VIOLATION 
#include <excpt.h> 

int ExceptionCatch(unsigned int code, struct _EXCEPTION_POINTERS *ep) { 
   puts("in filter."); 
   if (code == EXCEPTION_ACCESS_VIOLATION) { 
   } 
   return EXCEPTION_EXECUTE_HANDLER;
}

volatile int exit_spin = 0;
void Spin(void *arg) {
  int a=0;

  __try { 
    while(!exit_spin) {
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      a++;
      Sleep(0);
    }
   } __except(ExceptionCatch(GetExceptionCode(), GetExceptionInformation())) { 
      printf("Caught Exception.\n");
      return;
   } 
}

#define DEBUG_BREAK 0xCC
void insert_break(void *ptr)
{
	unsigned char *p = (unsigned char *) ptr; //_ReturnAddress();
	DWORD oldflags;
	
	if (!VirtualProtect(p, 64, PAGE_EXECUTE_READWRITE ,&oldflags))
	{
		printf("Failed with %d\n", GetLastError());
	}
	else
	{
		*p = DEBUG_BREAK;
		VirtualProtect(p, 64, oldflags, 0);
		FlushInstructionCache(GetCurrentProcess(), p, 64);
	}
}


int ExceptionTest() {
  char *p = 0;
  DWORD id;

  HANDLE thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) Spin, 0, 0, &id);
  printf("Created thread %d, %d\n", thread, id);

  Sleep(1000);

  int cnt = SuspendThread(thread);
  if (cnt == -1) {
      printf("Failed to suspend.\n");
  } else {
    CONTEXT ctx;
    memset(&ctx, 0, sizeof(ctx));
    Sleep(1000);
    ctx.ContextFlags = CONTEXT_CONTROL;
    if (0 == GetThreadContext(thread, &ctx))
      printf("Failed to Get Context with error %d.\n", GetLastError());
    else {
      printf("EIP=%llX\n", ctx.Rip);
      insert_break((void *) ctx.Rip);
    }
    //SetThreadContext(thread, &ctx);
    ResumeThread(thread);
  }


#if 0
   __try { 
      *p = 1;
   } __except(ExceptionCatch(GetExceptionCode(), GetExceptionInformation())) { 
      return 0;
   } 
#endif
   Sleep(1000);
   exit_spin = 1;
   return 0;
}

#endif


int BasicDebuggingTest() {
//   DebugHandle handle = NaClDebugStubCreate("TestDebug");
  return 0;
}


int main(int argc, char* argv[]) {
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);
  int errors = 0;

#if NACL_BUILD_SUBARCH == 64

  NaClLogModuleInit();
  NaClDebugStubInit();   

#if 1
  DebugHost *host = DebugHost::SocketConnect("127.0.0.1:4014");
  char path[256];
  
  host->GetPath(path, sizeof(path));

  delete host;
  return 0;
#else
  NaClThread *serverThread = ThreadCreate(TargetServer, 0);
  if (NULL == serverThread) {
    printf("Failed to create server thread.\n");
    errors++;
  } 
  else {
    // Wait for server to spin up
    WaitMS(1000);

    // Verify DebugInst
    errors += DebugInstTest();

    // Verify Basic Debugging
    errors += BasicDebuggingTest();

    doneTest = 1;
    WaitMS(1000);
    delete serverThread;
  }


  NaClDebugStubFini();
  NaClLogModuleFini();

  if (errors == 0)
  {
    printf("PASS\n");    
    return 0;
  } 

  printf("FAILED\n");
  return -1;
#endif
#else
  return 0;
#endif
}
