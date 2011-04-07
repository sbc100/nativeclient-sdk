// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef NACL_SDK_DEBUG_CORE_DEBUG_API_H_
#define NACL_SDK_DEBUG_CORE_DEBUG_API_H_
#include <windows.h>

namespace debug {
class DebugAPI {
 public:
  virtual ~DebugAPI() {}

  virtual BOOL CreateProcess(LPCTSTR lpApplicationName,
                             LPTSTR lpCommandLine,
                             LPSECURITY_ATTRIBUTES lpProcessAttributes,
                             LPSECURITY_ATTRIBUTES lpThreadAttributes,
                             BOOL bInheritHandles,
                             DWORD dwCreationFlags,
                             LPVOID lpEnvironment,
                             LPCTSTR lpCurrentDirectory,
                             LPSTARTUPINFO lpStartupInfo,
                             LPPROCESS_INFORMATION lpProcessInformation);

  virtual BOOL CloseHandle(HANDLE hObject);

  virtual BOOL ReadProcessMemory(HANDLE hProcess,
                                 LPCVOID lpBaseAddress,
                                 LPVOID lpBuffer,
                                 SIZE_T nSize,
                                 SIZE_T *lpNumberOfBytesRead);

  virtual BOOL WriteProcessMemory(HANDLE hProcess,
                                  LPVOID lpBaseAddress,
                                  LPCVOID lpBuffer,
                                  SIZE_T nSize,
                                  SIZE_T *lpNumberOfBytesWritten);

  virtual BOOL FlushInstructionCache(HANDLE hProcess,
                                     LPCVOID lpBaseAddress,
                                     SIZE_T dwSize);


  virtual BOOL GetThreadContext(HANDLE hThread,LPCONTEXT lpContext);
  virtual BOOL SetThreadContext(HANDLE hThread, CONTEXT *lpContext);

  virtual BOOL WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent,
                                 DWORD dwMilliseconds);

  virtual BOOL ContinueDebugEvent(DWORD dwProcessId,
                                  DWORD dwThreadId,
                                  DWORD dwContinueStatus);

  virtual BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode);
  virtual BOOL DebugBreakProcess(HANDLE Process);

  virtual BOOL Wow64GetThreadContext(HANDLE hThread,PWOW64_CONTEXT lpContext);
  virtual BOOL Wow64SetThreadContext(HANDLE hThread,
                                     const WOW64_CONTEXT* lpContext);
  virtual BOOL IsWow64Process(HANDLE hProcess,PBOOL Wow64Process);
  virtual BOOL DebugActiveProcess(DWORD dwProcessId);
  virtual BOOL DebugActiveProcessStop(DWORD dwProcessId);
};
}  // namespace debug
#endif  // NACL_SDK_DEBUG_CORE_DEBUG_API_H_
