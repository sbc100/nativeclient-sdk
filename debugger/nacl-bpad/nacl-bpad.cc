// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "debugger/base/debug_blob.h"
#include "debugger/base/debug_command_line.h"
#include "debugger/chrome_integration_test/rsp_registers.h"
#include "debugger/core/debug_api.h"
#include "debugger/core/debug_execution_engine.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debuggee_thread.h"
#include "debugger/nacl-gdb_server/gdb_registers.h"

#pragma warning(disable : 4996)  // Disable getenv warning.

namespace {
const char* kUsage =
    "usage: nacl-bpad -cmd \"<app_path-and_name> <arg0> <arg1> ...\"\n"
    "example: nacl-bpad -cmd \"c:\\chrome.exe --incognito "
    "http://localhost:5103/hello_world_c/hello_world.html\"\n\n"
    "Runs process tree, waiting for nexe to crash.\n"
    "When it crashes, it stops and prints values of registers,\n"
    "and source file name and line corresponding"
    " to instruction pointer.\n\n"
    "Note: you should have set NACL_SDK_ROOT environment variable.\n"
    "Note: set NEXE_PATH to full path to nexe.\n"
    "Note: make sure you are loading nexe with debug symbols.";

#ifdef _WIN64
const char* kAddressSize = "64";
const char* kAddToLinePrefix = "64";
#else
const char* kAddressSize = "32";
const char* kAddToLinePrefix = "";
#endif

void MakeContinueDecision(const debug::DebugEvent& debug_event,
                          bool is_nacl_app_thread,
                          bool* halt,
                          bool* pass_exception);
std::string GetLastErrorDescription();
void PrintEventDetails(DEBUG_EVENT de, debug::DebuggeeThread* halted_thread);
}  // namespace

int main(int argc, char* argv[]) {
  printf("nacl-bpad %s-bit version 0.4\n", kAddressSize);

  debug::CommandLine cmd_line(argc, argv);
  std::string debuggee_cmd_line = cmd_line.GetStringSwitch("-cmd", "");
  if (debuggee_cmd_line.size() <= 0) {
    printf("%s", kUsage);
    return 1;
  }

  debug::DebugAPI debug_api;
  debug::ExecutionEngine execution_engine(&debug_api);

  bool res = execution_engine.StartProcess(debuggee_cmd_line.c_str(), NULL);
  if (res) {
    printf("Starting process [%s]...\n", debuggee_cmd_line.c_str());
  } else {
    std::string sys_err = GetLastErrorDescription();
    printf("StartProcess failed cmd='%s' err='%s'",
            debuggee_cmd_line.c_str(),
            sys_err.c_str());
    return 2;
  }

  bool starting = true;
  do {
    int pid = 0;
    const int kWaitForDebugEventMsec = 200;
    if (execution_engine.WaitForDebugEventAndDispatchIt(
        kWaitForDebugEventMsec,
        &pid)) {
      starting = false;
      debug::IDebuggeeProcess* halted_process =
          execution_engine.GetProcess(pid);
      if (NULL != halted_process) {
        debug::DebugEvent de = execution_engine.debug_event();
        DEBUG_EVENT wde = de.windows_debug_event();
        debug::DebuggeeThread* halted_thread =
          halted_process->GetHaltedThread();

        if (debug::DebugEvent::kThreadIsAboutToStart ==
            de.nacl_debug_event_code()) {
          printf("NaCl thread started pid=%d tid=%d",
                 wde.dwProcessId,
                 wde.dwThreadId);
          if (NULL != halted_thread)
            printf(" mem_base=0x%p\n",
                   halted_thread->parent_process().nexe_mem_base());
          else
            printf("\n");
        }

        bool halt_debuggee = false;
        bool pass_exception = true;
        MakeContinueDecision(de,
                             halted_thread->IsNaClAppThread(),
                             &halt_debuggee,
                             &pass_exception);
        if (halt_debuggee) {
          // print registers and other stuff
          PrintEventDetails(wde, halted_thread);
          break;
        } else if (pass_exception) {
          halted_process->ContinueAndPassExceptionToDebuggee();
        } else {
          halted_process->Continue();
        }
      }
    }
  } while (execution_engine.HasAliveDebuggee() || starting);
  return 0;
}

namespace {
const char* GetExceptionName(int exc_no) {
#define EX_NAME(x) case x : return #x
  switch (exc_no) {
    EX_NAME(EXCEPTION_ACCESS_VIOLATION);
    EX_NAME(EXCEPTION_DATATYPE_MISALIGNMENT);
    EX_NAME(EXCEPTION_BREAKPOINT);
    EX_NAME(EXCEPTION_SINGLE_STEP);
    EX_NAME(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
    EX_NAME(EXCEPTION_FLT_DENORMAL_OPERAND);
    EX_NAME(EXCEPTION_FLT_DIVIDE_BY_ZERO);
    EX_NAME(EXCEPTION_FLT_INEXACT_RESULT);
    EX_NAME(EXCEPTION_FLT_INVALID_OPERATION);
    EX_NAME(EXCEPTION_FLT_OVERFLOW);
    EX_NAME(EXCEPTION_FLT_STACK_CHECK);
    EX_NAME(EXCEPTION_FLT_UNDERFLOW);
    EX_NAME(EXCEPTION_INT_DIVIDE_BY_ZERO);
    EX_NAME(EXCEPTION_INT_OVERFLOW);
    EX_NAME(EXCEPTION_PRIV_INSTRUCTION);
    EX_NAME(EXCEPTION_IN_PAGE_ERROR);
    EX_NAME(EXCEPTION_ILLEGAL_INSTRUCTION);
    EX_NAME(EXCEPTION_NONCONTINUABLE_EXCEPTION);
    EX_NAME(EXCEPTION_STACK_OVERFLOW);
    EX_NAME(EXCEPTION_INVALID_DISPOSITION);
    EX_NAME(EXCEPTION_GUARD_PAGE);
    EX_NAME(EXCEPTION_INVALID_HANDLE);
  }
#undef EX_NAME
  return "";
}

std::string GetStringEnvVar(const std::string& name,
                            const std::string& default_value) {
  char* value = getenv(name.c_str());
  if (NULL != value)
    return value;
  return default_value;
}

bool GetCodeLine(int addr, std::string* src_file, int* src_line) {
  const char* kLineFileName = "line.txt";
  std::string sdk_root = GetStringEnvVar("NACL_SDK_ROOT", "");
  std::string nexe_path = GetStringEnvVar("NEXE_PATH", "");

  *src_line = 0;
  char addr_str[200];
  _snprintf(addr_str, sizeof(addr_str), "0x%x", addr);
  std::string cmd = sdk_root +
      "\\toolchain\\win_x86\\bin\\nacl" + kAddToLinePrefix + "-addr2line "
      "--exe=" + nexe_path + " " + addr_str + " > " + kLineFileName;

  printf("\n-----calling-----\n%s\n----------\n\n", cmd.c_str());
  system(cmd.c_str());
  int scanned_items = 0;

  // line.txt shall have something like this:
  // /cygdrive/d/src/nacl_sdk2/src/examples/hello_world_c/hello_world.c:217
  FILE* file = fopen(kLineFileName, "rt");
  if (NULL != file) {
    char line[MAX_PATH] = {0};
    fgets(line, sizeof(line) - 1, file);
    line[sizeof(line) - 1] = 0;
    char tmp[MAX_PATH] = {0};
    scanned_items = sscanf(line, "%[^:]:%d", tmp, src_line);  // NOLINT
    *src_file = tmp;
    fclose(file);
  }
  bool res = (2 == scanned_items);
  if (!res)
    printf("Error running: '%s'\n", cmd.c_str());
  return res;
}

void PrintEventDetails(DEBUG_EVENT de, debug::DebuggeeThread* halted_thread) {
  int exc_no = de.u.Exception.ExceptionRecord.ExceptionCode;
  const char* exc_name = GetExceptionName(exc_no);
  void* addr = de.u.Exception.ExceptionRecord.ExceptionAddress;

#ifdef _WIN64
  // On a 64-bit Windows, instruction pointer is absolute, flat address,
  // so we have to translate it to the address relative to the memory base
  // of the loaded nexe.
  addr = static_cast<char*>(addr) -
    reinterpret_cast<size_t>(halted_thread->parent_process().nexe_mem_base());

  // On a 32-bit Windows, instruction pointer is relative to the
  // nexe memory base, so we don't have to adjust it.
#endif

  printf("\nException %s(%d) in nexe at 0x%p\n\n", exc_name, exc_no, addr);
  fflush(stdout);

  std::string src_file;
  int src_line = 0;
  if (GetCodeLine(reinterpret_cast<int>(addr), &src_file, &src_line)) {
    printf("\nException %s in:\n%s:%d\n\n",
           exc_name,
           src_file.c_str(),
           src_line);
  }

  CONTEXT context;
  if (halted_thread->GetContext(&context)) {
    debug::Blob gdb_regs;
    rsp::CONTEXTToGdbRegisters(context, &gdb_regs);

    debug::RegistersSet regs;
#ifdef _WIN64
    regs.InitializeForWin64();
#else
    regs.InitializeForWin32();
#endif
    regs.PrintRegisters(gdb_regs);
  }
}

std::string GetLastErrorDescription() {
  int error_code = ::GetLastError();
  char buff[1000] = {'?', 0};
  ::FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                   0,
                   error_code,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buff,
                   sizeof(buff),
                   0);
  return std::string(buff);
}

// 0. By default, don't stop and pass exception to debuggee
// 1. Don't stop on OUTPUT_DEBUG_STRING_EVENT, don't pass it to debuggee
// 2. Don't stop on NaCl debug events
// 3. Don't pass EXCEPTION_BREAKPOINT to debuggee
// 4. Don't stop on kVS2008_THREAD_INFO exception, pass it to debuggee
// 5. stop on exceptions in nexe threads
void MakeContinueDecision(const debug::DebugEvent& debug_event,
                          bool is_nacl_app_thread,
                          bool* halt,
                          bool* pass_exception) {
  const int kVS2008_THREAD_INFO = 0x406D1388;
  *halt = false;
  *pass_exception = true;

  DEBUG_EVENT wde = debug_event.windows_debug_event();
  if (OUTPUT_DEBUG_STRING_EVENT == wde.dwDebugEventCode) {
    *halt = false;
    // Recent chrome requires it to run under debugger
    // (chromium Build 92793).
    *pass_exception = false;
  }
  if (EXCEPTION_DEBUG_EVENT == wde.dwDebugEventCode) {
    int exception_code = wde.u.Exception.ExceptionRecord.ExceptionCode;
    if (kVS2008_THREAD_INFO != exception_code) {
     if (EXCEPTION_BREAKPOINT == exception_code)
        *pass_exception = false;
      *halt = false;
    }
    *halt = is_nacl_app_thread;
  } else if (EXIT_THREAD_DEBUG_EVENT == wde.dwDebugEventCode) {
      *halt = false;
  }
}

}  // namespace

