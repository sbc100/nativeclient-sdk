// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api.h"
#include "debugger/core/debug_logger.h"

namespace debug {
BOOL DebugAPI::CreateProcess(
    LPCTSTR lpApplicationName,
    LPTSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCTSTR lpCurrentDirectory,
    LPSTARTUPINFO lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation) {
  return ::CreateProcess(lpApplicationName,
                         lpCommandLine,
                         lpProcessAttributes,
                         lpThreadAttributes,
                         bInheritHandles,
                         dwCreationFlags,
                         lpEnvironment,
                         lpCurrentDirectory,
                         lpStartupInfo,
                         lpProcessInformation);
}

BOOL DebugAPI::CloseHandle(HANDLE hObject) {
  return ::CloseHandle(hObject);
}

BOOL DebugAPI::ReadProcessMemory(HANDLE hProcess,
                                 LPCVOID lpBaseAddress,
                                 LPVOID lpBuffer,
                                 SIZE_T nSize,
                                 SIZE_T *lpNumberOfBytesRead) {
  int pid = ::GetProcessId(hProcess);
  SIZE_T rd = 0;
  BOOL res = ::ReadProcessMemory(hProcess,
                                 lpBaseAddress,
                                 lpBuffer,
                                 nSize,
                                 &rd);
  if (TRUE != res)
    DBG_LOG("ERR02.01",
            "msg='::ReadProcessMemory(pid=%d addr=%p len=%d) -> error' rd=%d",
            pid,
            lpBaseAddress,
            nSize,
            rd);
  if (NULL != lpNumberOfBytesRead)
    *lpNumberOfBytesRead = rd;
  return res;
}

BOOL DebugAPI::WriteProcessMemory(HANDLE hProcess,
                                  LPVOID lpBaseAddress,
                                  LPCVOID lpBuffer,
                                  SIZE_T nSize,
                                  SIZE_T *lpNumberOfBytesWritten) {
  return ::WriteProcessMemory(hProcess,
                              lpBaseAddress,
                              lpBuffer,
                              nSize,
                              lpNumberOfBytesWritten);
}

BOOL DebugAPI::FlushInstructionCache(HANDLE hProcess,
                                     LPCVOID lpBaseAddress,
                                     SIZE_T dwSize) {
  return ::FlushInstructionCache(hProcess, lpBaseAddress, dwSize);
}

BOOL DebugAPI::GetThreadContext(HANDLE hThread, LPCONTEXT lpContext) {
  return ::GetThreadContext(hThread, lpContext);
}

BOOL DebugAPI::SetThreadContext(HANDLE hThread, CONTEXT *lpContext) {
  return ::SetThreadContext(hThread, lpContext);
}

BOOL DebugAPI::WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent,
                                 DWORD dwMilliseconds) {
  return ::WaitForDebugEvent(lpDebugEvent, dwMilliseconds);
}

BOOL DebugAPI::ContinueDebugEvent(DWORD dwProcessId,
                                  DWORD dwThreadId,
                                  DWORD dwContinueStatus) {
  BOOL res = ::ContinueDebugEvent(dwProcessId, dwThreadId, dwContinueStatus);
  DBG_LOG("TR02.02",
          "msg='::ContinueDebugEvent(dwProcessId=%d dwThreadId=%d"
          " dwContinueStatus=0x%X) -> %s",
          dwProcessId,
          dwThreadId,
          dwContinueStatus,
          res ? "ok" : "error");
  return res;
}

BOOL DebugAPI::TerminateThread(HANDLE hThread, DWORD dwExitCode) {
  return ::TerminateThread(hThread, dwExitCode);
}

BOOL DebugAPI::DebugBreakProcess(HANDLE Process) {
  return ::DebugBreakProcess(Process);
}

BOOL DebugAPI::Wow64GetThreadContext(HANDLE hThread, PWOW64_CONTEXT lpContext) {
  return ::Wow64GetThreadContext(hThread, lpContext);
}

BOOL DebugAPI::Wow64SetThreadContext(HANDLE hThread,
                                     const WOW64_CONTEXT* lpContext) {
  return ::Wow64SetThreadContext(hThread, lpContext);
}

BOOL DebugAPI::IsWoW64Process(HANDLE hProcess, PBOOL Wow64Process) {
  return ::IsWow64Process(hProcess, Wow64Process);
}

BOOL DebugAPI::DebugActiveProcess(DWORD dwProcessId) {
  return ::DebugActiveProcess(dwProcessId);
}

BOOL DebugAPI::DebugActiveProcessStop(DWORD dwProcessId) {
  return ::DebugActiveProcessStop(dwProcessId);
}
}  // namespace debug

