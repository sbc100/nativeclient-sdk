// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_API_H_
#define DEBUGGER_CORE_DEBUG_API_H_
#include <windows.h>
#include <map>

namespace debug {
/// This class is a very thin layer on top of Windows Debug API.

/// Intended to simplify writing unit tests - as a base for DebugAPIMock.
/// Here' a link to Microsoft documentation:
/// http://msdn.microsoft.com/en-us/library/ms679303.aspx
class DebugAPI {
 public:
  DebugAPI() {}
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


  virtual BOOL GetThreadContext(HANDLE hThread, LPCONTEXT lpContext);
  virtual BOOL SetThreadContext(HANDLE hThread, CONTEXT *lpContext);

  virtual BOOL WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent,
                                 DWORD dwMilliseconds);

  virtual BOOL ContinueDebugEvent(DWORD dwProcessId,
                                  DWORD dwThreadId,
                                  DWORD dwContinueStatus);

  virtual BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode);
  virtual BOOL DebugBreakProcess(HANDLE Process);

  virtual BOOL Wow64GetThreadContext(HANDLE hThread, PWOW64_CONTEXT lpContext);
  virtual BOOL Wow64SetThreadContext(HANDLE hThread,
                                     const WOW64_CONTEXT* lpContext);
  virtual BOOL IsWoW64Process(HANDLE hProcess, PBOOL Wow64Process);
  virtual BOOL DebugActiveProcess(DWORD dwProcessId);
  virtual BOOL DebugActiveProcessStop(DWORD dwProcessId);

 private:
  DebugAPI(const DebugAPI&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const DebugAPI&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUG_API_H_

