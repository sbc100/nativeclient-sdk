// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_API_MOCK_H_
#define DEBUGGER_CORE_DEBUG_API_MOCK_H_
#include <deque>
#include "debugger/core/debug_api.h"

namespace debug {
/// This class is a mock of DebugAPI, used in unit tests.
///
/// This is pure mock, Windows API is never called.
/// |DebugAPIMock| keeps a queue of function call,
/// so that sequence of API calls can be verified by |CompareCallSequence|.
/// Parameters of the API calls are not tracked / verified.
class DebugAPIMock : public DebugAPI {
 public:
  enum FunctionId {
    kCreateProcess,
    kCloseHandle,
    kReadProcessMemory,
    kWriteProcessMemory,
    kFlushInstructionCache,
    kGetThreadContext,
    kSetThreadContext,
    kWaitForDebugEvent,
    kContinueDebugEvent,
    kTerminateThread,
    kDebugBreakProcess,
    kWow64GetThreadContext,
    kWow64SetThreadContext,
    kIsWoW64Process,
    kDebugActiveProcess,
    kDebugActiveProcessStop,
  };

  DebugAPIMock();

  void ClearCallSequence();
  bool CompareCallSequence(const std::deque<FunctionId>& call_list) const;

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

  bool single_step_enabled_;
  std::deque<FunctionId> called_functions_;
};
}  // namespace debug

#endif  // DEBUGGER_CORE_DEBUG_API_MOCK_H_

