/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "native_client/src/include/nacl_base.h"
#include "native_client/src/shared/platform/nacl_threads.h"

#include "native_client/src/trusted/debug_stub/debug_inst.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"
#include "native_client/src/trusted/debug_stub/debug_target.h"
#include "native_client/src/trusted/debug_stub/debug_thread.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

#include "native_client/src/trusted/service_runtime/arch/x86/sel_rt.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include <winsock2.h>


using nacl_debug_stub::DebugThread;
using nacl_debug_stub::DebugInst;
using nacl_debug_conn::DebugTargetRegsX86_64_t;

static LONG NTAPI ExceptionCatch(PEXCEPTION_POINTERS ep);
static int  ExceptionToSignal(int);

static void CopyUntrustedToDebugThread(const CONTEXT *ctx, DebugThread *thread);
static void CopyDebugThreadToUntrusted(const DebugThread *thread, CONTEXT *ctx);

static void CopyTrustedToDebugThread(const struct NaClThreadContext *ctx, DebugThread *thread);
static void CopyDebugThreadToTrusted(const DebugThread *thread, struct NaClThreadContext *ctx);


#ifdef _WIN64
#define REG64_CNT 17
#define REG32_CNT  7

typedef struct {
  intptr_t  Regs64[REG64_CNT];
  uint32_t  Regs32[REG32_CNT];
} NaClDebugThreadContext;


intptr_t GetRegFromCtx(const CONTEXT *ctx, int32_t num) {
  switch(num) {
    case 0: return ctx->Rax; 
    case 1: return ctx->Rbx; 
    case 2: return ctx->Rcx; 
    case 3: return ctx->Rdx; 
    case 4: return ctx->Rsi; 
    case 5: return ctx->Rdi; 
    case 6: return ctx->Rbp; 
    case 7: return ctx->Rsp; 
    case 8: return ctx->R8; 
    case 9: return ctx->R9;
    case 10:return ctx->R10; 
    case 11:return ctx->R11; 
    case 12:return ctx->R12; 
    case 13:return ctx->R13; 
    case 14:return ctx->R14; 
    case 15:return ctx->R15; 
    case 16:return ctx->Rip; 
    case 17:return ctx->EFlags; 
    case 18:return ctx->SegCs; 
    case 19:return ctx->SegSs; 
    case 20:return ctx->SegDs; 
    case 21:return ctx->SegEs; 
    case 22:return ctx->SegFs; 
    case 23:return ctx->SegGs; 
  }

  return -1;
}

void SetRegInCtx(CONTEXT *ctx, int32_t num, intptr_t val) {
  switch(num) {
    case 0: ctx->Rax = val; break;
    case 1: ctx->Rbx = val; break;
    case 2: ctx->Rcx = val; break;
    case 3: ctx->Rdx = val; break;
    case 4: ctx->Rsi = val; break;
    case 5: ctx->Rdi = val; break;
    case 6: ctx->Rbp = val; break;
    case 7: ctx->Rsp = val; break;
    case 8: ctx->R8 = val; break;
    case 9: ctx->R9 = val; break;
    case 10:ctx->R10 = val; break;
    case 11:ctx->R11 = val; break;
    case 12:ctx->R12 = val; break;
    case 13:ctx->R13 = val; break;
    case 14:ctx->R14 = val; break;
    case 15:ctx->R15 = val; break;
    case 16:ctx->Rip = val; break; 
    case 17:ctx->EFlags = (uint32_t) val; break; 
    case 18:ctx->SegCs = (uint32_t) val; break; 
    case 19:ctx->SegSs = (uint32_t) val; break; 
    case 20:ctx->SegDs = (uint32_t) val; break; 
    case 21:ctx->SegEs = (uint32_t) val; break; 
    case 22:ctx->SegFs = (uint32_t) val; break; 
    case 23:ctx->SegGs = (uint32_t) val; break; 
  }
}

#else

intptr_t GetRegFromCtx(const CONTEXT *ctx, int32_t num) {
  switch(num) {
    case 0: return ctx->Eax; 
    case 1: return ctx->Ecx; 
    case 2: return ctx->Edx; 
    case 3: return ctx->Ebx; 
    case 4: return ctx->Ebp; 
    case 5: return ctx->Esp; 
    case 6: return ctx->Esi; 
    case 7: return ctx->Edi; 
    case 8: return ctx->Eip; 
    case 9: return ctx->EFlags; 
    case 10:return ctx->SegCs; 
    case 11:return ctx->SegSs; 
    case 12:return ctx->SegDs; 
    case 13:return ctx->SegEs; 
    case 14:return ctx->SegFs; 
    case 15:return ctx->SegGs; 
  }

  return -1;
}

void SetRegInCtx(CONTEXT *ctx, int32_t num, intptr_t val) {
  switch(num) {
    case 0:  ctx->Eax = val;  break;
    case 1:  ctx->Ecx = val;  break;
    case 2:  ctx->Edx = val;  break;
    case 3:  ctx->Ebx = val;  break;
    case 4:  ctx->Ebp = val;  break;
    case 5:  ctx->Esp = val;  break;
    case 6:  ctx->Esi = val;  break;
    case 7:  ctx->Edi = val;  break;
    case 8:  ctx->Eip = val;  break;
    case 9:  ctx->EFlags = val;  break;
    case 10: ctx->SegCs = val;  break;
    case 11: ctx->SegSs = val;  break;
    case 12: ctx->SegDs = val;  break;
    case 13: ctx->SegEs = val;  break;
    case 14: ctx->SegFs = val;  break;
    case 15: ctx->SegGs = val;  break;
  }
}
#endif

static bool InSysCall(DebugInst* inst, uint64_t eip) {
  if ((eip >= inst->GetOffset()) && (eip < (inst->GetOffset() + 0x100000000)))
    return false;
  return true;
}

#define TRAP_FLAG (1 << 8)
LONG NTAPI ExceptionCatch(PEXCEPTION_POINTERS ep) {
  DebugInst* inst = DebugInst::DebugStub();

  DebugInst::ThreadId_t id;
  DebugThread *thread;
  NaClDebugThreadContext *dtx;
  
  id = static_cast<DebugInst::ThreadId_t>(GetCurrentThreadId());
  thread = nacl_debug_stub::DebugInst::DebugStub()->GetThread(id);
  dtx = static_cast<NaClDebugThreadContext *>(thread->registers_);

  // Turn of step if enabled
  ep->ContextRecord->EFlags &= ~TRAP_FLAG;

  // Update the context
  if (InSysCall(inst, ep->ContextRecord->Rip)) {
    thread->SetFlag(DebugThread::DTF_SYSCALL);
    CopyTrustedToDebugThread(thread->GetUserCtx(), thread);
  }
  else {
    thread->ClearFlag(DebugThread::DTF_SYSCALL);
    CopyUntrustedToDebugThread(ep->ContextRecord, thread);
  }

  // Disable breakpoint if this is first time in.
  if (inst->GetFlags() & DebugInst::DIF_BREAK_START) {
    printf("Removing startup breakpoint.\n");
    inst->ClearFlagMask(DebugInst::DIF_BREAK_START);
    inst->DisableBreakpoint(ep->ContextRecord->Rip);
  }

  // We are done with setup, so signal that the debug stub can access the thread.
  thread->res_ = DebugThread::DTR_WAIT;
  thread->sig_ = ExceptionToSignal(ep->ExceptionRecord->ExceptionCode);
  debug_log_error("Caught exception %d : %s\n", 
                   thread->sig_,
                   NaClDebugSigToStr(thread->sig_));

  // Wait while the debug stub thread talks to the debugger.
  while (DebugThread::DTR_WAIT == thread->res_ ) {
    
    // Sleep until the debugger tells us to go
    Sleep(100);
  }

  printf("Continuing from exception...\n");
  
  // If we are continuing...
  if (DebugThread::DTR_KILL != thread->res_) {
    if (thread->GetFlag(DebugThread::DTF_SYSCALL)) {
      CopyDebugThreadToTrusted(thread, thread->GetUserCtx());
    } 
    else {
      // Update the thread state, then return to where we stopped
      CopyDebugThreadToUntrusted(thread, ep->ContextRecord);

      // If we requested stepping...
      // NOTE: we can only do this in NEXE space, otherwise we would
      // trigger the step inside of the trusted code.
      if (DebugThread::DTR_STEP == thread->res_) {
        ep->ContextRecord->EFlags |= TRAP_FLAG;
      }
    }

    return EXCEPTION_CONTINUE_EXECUTION;
  }


  // Kill this thread by returning
  return EXCEPTION_EXECUTE_HANDLER;
}

namespace nacl_debug_stub {
bool NaClDebugStubThreadPause(DebugThread *dtp) {
  DebugInst* inst = DebugInst::DebugStub();

  switch (dtp->sig_) {
    case 0:   // Suspend running thread
    {
      CONTEXT ctx;

      memset(&ctx, 0, sizeof(ctx));
      ctx.ContextFlags = CONTEXT_ALL;

      if (SuspendThread(dtp->GetHandle()) == -1) {
        debug_log_error("Could not suspend thread %d.\n", dtp->GetID());
        return false;
      }   
      if (0 == GetThreadContext(dtp->GetHandle(), &ctx)) {
	      debug_log_error("Unable to get context of thread %d\n",dtp->GetID());
        return false;
      }

      CopyUntrustedToDebugThread(&ctx, dtp);
      if (InSysCall(inst, ctx.Rip)) {
        dtp->SetFlag(DebugThread::DTF_SYSCALL);
        CopyTrustedToDebugThread(dtp->GetUserCtx(), dtp);
      } else {
        dtp->ClearFlag(DebugThread::DTF_SYSCALL);
      }
      dtp->sig_ = -1;
      break;
    }

    case -1:
	    debug_log_warning("Tried to pause suspended thread %d\n", dtp->GetID());
      break;

    default:
	    debug_log_warning("Tried to pause signalled thread %d\n", dtp->GetID());
      break;
  }
  return true;
}

bool NaClDebugStubThreadResume(DebugThread *dtp) {
  DebugInst* inst = DebugInst::DebugStub();

  switch (dtp->sig_) {
    case -1: // Suspended thread
    {
      CONTEXT ctx;

      memset(&ctx, 0, sizeof(ctx));
      ctx.ContextFlags = CONTEXT_INTEGER;

      CopyDebugThreadToUntrusted(dtp, &ctx);

      // Set trap FLAG if requested
      if (DebugThread::DTR_STEP == dtp->res_) {
        ctx.EFlags |= TRAP_FLAG;
      }

      // Only set the context if within the NEXE, otherwise the SYSCALL will restore.
      if (dtp->GetFlag(DebugThread::DTF_SYSCALL)) {
        CopyDebugThreadToTrusted(dtp, dtp->GetUserCtx());
      }
      else {
        if (0 == SetThreadContext(dtp->GetHandle(), &ctx)) {
	        debug_log_error("Unable to get context of thread %d\n", dtp->GetID());
          return false;
        }
      }
      if (ResumeThread(dtp->GetHandle()) == -1) {
        debug_log_error("Could not resume thread %d.\n", dtp->GetID());
        return false;
      }
      dtp->sig_ = 0;
    }
    case 0: // Running thread
  	  debug_log_warning("Tried to resume running thread %d\n", dtp->GetID());
      break;

    default: 
  	  debug_log_warning("Tried to resume signalled thread %d\n", dtp->GetID());
      break;
  }
  return true;
}

} /* End of nacl_debug_stub Namespace */

const char *NaClDebugSigToStr(int sig) {
  switch (sig) {
    case 1: return "Hangup (POSIX)";
    case 2: return "Terminal interrupt (ANSI)";
    case 3: return "Terminal quit (POSIX)";
    case 4: return "Illegal instruction (ANSI)";
    case 5: return "Trace trap (POSIX)";
    case 6: return "IOT Trap (4.2 BSD)";
    case 7: return "BUS error (4.2 BSD)";
    case 8: return "Floating point exception (ANSI)";
    case 9: return "Kill(can't be caught or ignored) (POSIX)";
    case 10: return "User defined signal 1 (POSIX)";
    case 11: return "Invalid memory segment access (ANSI)";
    case 12: return "User defined signal 2 (POSIX)";
    case 13: return "Write on a pipe with no reader, Broken pipe (POSIX)";
    case 14: return "Alarm clock (POSIX)";
    case 15: return "Termination (ANSI)";
    case 16: return "Stack fault";
    case 17: return "Child process has stopped or exited, changed (POSIX)";
    case 18: return "Continue executing, if stopped (POSIX)";
    case 19: return "Stop executing(can't be caught or ignored (POSIX)";
    case 20: return "Terminal stop signal (POSIX)";
    case 21: return "Background process trying to read, from TTY (POSIX)";
    case 22: return "Background process trying to write, to TTY (POSIX)";
    case 23: return "Urgent condition on socket (4.2 BSD)";
    case 24: return "CPU limit exceeded (4.2 BSD)";
    case 25: return "File size limit exceeded (4.2 BSD)";
    case 26: return "Virtual alarm clock (4.2 BSD)";
    case 27: return "Profiling alarm clock (4.2 BSD)";
    case 28: return "Window size change (4.3 BSD, Sun)";
    case 29: return "I/O now possible (4.2 BSD)";
    case 30: return "Power failure restart (System V)";
  }

  return "Unknown signal";
}


uint32_t nacl_debug_stub::NaClDebugStubThreadContextSize() {
  return sizeof(intptr_t) * REG64_CNT + sizeof(uint32_t) * REG32_CNT;
};

void WINAPI NaClDebugStubThreadStart(void *cookie) {
  struct NaClAppThread  *natp = static_cast<NaClAppThread *>(cookie);
  DebugThread::ThreadId_t id;
  
  id = static_cast<DebugThread::ThreadId_t>(GetCurrentThreadId());

  
  DebugInst *inst = DebugInst::DebugStub();
  DebugThread *dtp;
  HANDLE  hdl = OpenThread(THREAD_ALL_ACCESS, 0, (DWORD) id);
  
  dtp = new DebugThread(id, hdl, static_cast<intptr_t>(natp->user.prog_ctr), cookie);
  dtp->SetNaClAppThread(natp);

  inst->AddThread(dtp);
  
  /* Set the exception vector for this thread */
  AddVectoredExceptionHandler( 1, ExceptionCatch );
  
  inst->Launch(cookie);
}

uint32_t NaClDebugStubReprotect(void *ptr, uint32_t len, uint32_t newflags) {
  DWORD oldflags;
  if (!VirtualProtect(ptr, len, newflags, &oldflags)) {
		printf("Failed with %d\n", GetLastError());
    return -1;
  }

  FlushInstructionCache(GetCurrentProcess(), ptr, len);
  return oldflags;
}


void NaClGetFilePath(const char *file, char *out, uint32_t max) {
    HANDLE hFile;
    DWORD dwRet;

    hFile = CreateFileA(file,                 // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL, // normal file
                       NULL);                 // no attr. template

    if( hFile == INVALID_HANDLE_VALUE)
    {
        printf("Could not open file (error %d\n)", GetLastError());
        return;
    }

    dwRet = GetFinalPathNameByHandleA( hFile, out, max, VOLUME_NAME_DOS ); //FILE_NAME_NORMALIZED 
    CloseHandle(hFile);
}

int ExceptionToSignal(int ex) {
  switch(ex) {
    case EXCEPTION_GUARD_PAGE:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_ACCESS_VIOLATION: 
    case EXCEPTION_IN_PAGE_ERROR:
      return 11;

    case EXCEPTION_BREAKPOINT: 
    case EXCEPTION_SINGLE_STEP: 
      return 5;   
    
    case EXCEPTION_FLT_DENORMAL_OPERAND: 
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:   
    case EXCEPTION_FLT_INEXACT_RESULT:     
    case EXCEPTION_FLT_INVALID_OPERATION:  
    case EXCEPTION_FLT_OVERFLOW:  
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
      return 8;

    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_PRIV_INSTRUCTION:
      return 4;

    case EXCEPTION_STACK_OVERFLOW:
      return 16;

    case CONTROL_C_EXIT:
      return 3;

    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
    case EXCEPTION_INVALID_DISPOSITION:
    case EXCEPTION_INVALID_HANDLE:
      return 4;

  }
  return 4;
}

void CopyUntrustedToDebugThread(const CONTEXT *ctx, DebugThread *thread) {
  int a;
  NaClDebugThreadContext *nctx = static_cast<NaClDebugThreadContext *>(thread->registers_);
  for (a = 0; a < REG64_CNT; a++)
    nctx->Regs64[a] = GetRegFromCtx(ctx, a);
  for (a = 0; a < REG32_CNT; a++)
    nctx->Regs32[a] = (uint32_t) GetRegFromCtx(ctx, a + REG64_CNT);
}

void CopyDebugThreadToUntrusted(const DebugThread *thread, CONTEXT *ctx) {
  int a;
  NaClDebugThreadContext *nctx = static_cast<NaClDebugThreadContext *>(thread->registers_);
  for (a = 0; a < REG64_CNT; a++)
    SetRegInCtx(ctx, a, nctx->Regs64[a]);
  for (a = 0; a < REG32_CNT; a++)
    SetRegInCtx(ctx, a + REG64_CNT, nctx->Regs32[a]);
    
}

nacl_reg_t GetSyscallReturnAddress(DebugThread *thread) {
  uintptr_t sp_user = NaClGetThreadCtxSp(
    static_cast<NaClThreadContext*>(thread->GetContextPtr()));
  uintptr_t sp_sys = NaClUserToSysStackAddr(thread->GetNaClAppThread()->nap, sp_user);
  nacl_reg_t user_ret = *(uintptr_t *) (sp_sys + NACL_USERRET_FIX);
  return user_ret;
}

void CopyTrustedToDebugThread(const struct NaClThreadContext *ctx, DebugThread *thread) {
  DebugTargetRegsX86_64_t *nctx = static_cast<DebugTargetRegsX86_64_t *>(thread->registers_);

  nctx->Rax = ctx->rax;
  nctx->Rbx = ctx->rbx;
  nctx->Rcx = ctx->rcx;
  nctx->Rdx = ctx->rdx;
  nctx->Rbp = ctx->rbp;
  nctx->Rsi = ctx->rsi;
  nctx->Rdi = ctx->rdi;
  nctx->Rsp = ctx->rsp;
  nctx->R8 = ctx->r8;
  nctx->R9 = ctx->r9;
  nctx->R10 = ctx->r10;
  nctx->R11 = ctx->r11;
  nctx->R12 = ctx->r12;
  nctx->R13 = ctx->r13;
  nctx->R14 = ctx->r14;
  nctx->R15 = ctx->r15;

  nctx->Rip = GetSyscallReturnAddress(thread);
}

void CopyDebugThreadToTrusted(const DebugThread *thread, struct NaClThreadContext *ctx) {
  DebugTargetRegsX86_64_t *nctx = static_cast<DebugTargetRegsX86_64_t *>(thread->registers_);

  ctx->rax = nctx->Rax;
  ctx->rbx = nctx->Rbx;
  ctx->rcx = nctx->Rcx;
  ctx->rdx = nctx->Rdx;
  ctx->rbp = nctx->Rbp;
  ctx->rsi = nctx->Rsi;
  ctx->rdi = nctx->Rdi;
  ctx->rsp = nctx->Rsp;
  ctx->r8 = nctx->R8;
  ctx->r9 = nctx->R9;
  ctx->r10 = nctx->R10;
  ctx->r11 = nctx->R11;
  ctx->r12 = nctx->R12;
  ctx->r13 = nctx->R13;
  ctx->r14 = nctx->R14;
  ctx->r15 = nctx->R15;
  ctx->prog_ctr = nctx->Rip;
}

