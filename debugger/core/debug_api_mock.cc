// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api_mock.h"

#define DAPI_PROLOG(function) called_functions_.push_back(k##function)
#define CASE_NAME(x) case debug::DebugAPIMock::x: return #x

namespace {
const char* GetNameOfDebugApiCall(debug::DebugAPIMock::FunctionId call_id) {
  switch (call_id) {
    CASE_NAME(kCreateProcess);
    CASE_NAME(kCloseHandle);
    CASE_NAME(kReadProcessMemory);
    CASE_NAME(kWriteProcessMemory);
    CASE_NAME(kFlushInstructionCache);
    CASE_NAME(kGetThreadContext);
    CASE_NAME(kSetThreadContext);
    CASE_NAME(kWaitForDebugEvent);
    CASE_NAME(kContinueDebugEvent);
    CASE_NAME(kTerminateThread);
    CASE_NAME(kDebugBreakProcess);
    CASE_NAME(kWow64GetThreadContext);
    CASE_NAME(kWow64SetThreadContext);
    CASE_NAME(kIsWoW64Process);
    CASE_NAME(kDebugActiveProcess);
    CASE_NAME(kDebugActiveProcessStop);
  }
  return "N/A";
}
}

namespace debug {
DebugAPIMock::DebugAPIMock()
    : single_step_enabled_(false) {
}

void DebugAPIMock::ClearCallSequence() {
  called_functions_.clear();
}

bool DebugAPIMock::CompareCallSequence(
    const std::deque<FunctionId>& call_list) const {
  bool equal = true;

  size_t num = called_functions_.size();
  if (call_list.size() != num) {
    equal = false;
  }
  if (equal) {
    for (size_t i = 0; i < num; i++) {
      if (called_functions_[i] != call_list[i]) {
        equal = false;
        break;
      }
    }
  }
  if (!equal) {  // Print out expected and actual call list.
    printf("=== expected call list:\n");
    for (size_t i = 0; i < call_list.size(); i++) {
      const char* call_name = GetNameOfDebugApiCall(call_list[i]);
      printf("\t%s\n", call_name);
    }
    printf("=== actual call list:\n");
    for (size_t i = 0; i < called_functions_.size(); i++) {
      const char* call_name = GetNameOfDebugApiCall(called_functions_[i]);
      printf("\t%s\n", call_name);
    }
  }
  return equal;
}

BOOL DebugAPIMock::CreateProcess(LPCTSTR lpApplicationName,
                                 LPTSTR lpCommandLine,
                                 LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                 LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                 BOOL bInheritHandles,
                                 DWORD dwCreationFlags,
                                 LPVOID lpEnvironment,
                                 LPCTSTR lpCurrentDirectory,
                                 LPSTARTUPINFO lpStartupInfo,
                                 LPPROCESS_INFORMATION lpProcessInformation) {
  DAPI_PROLOG(CreateProcess);
  return TRUE;
}

BOOL DebugAPIMock::CloseHandle(HANDLE hObject) {
  DAPI_PROLOG(CloseHandle);
  return TRUE;
}

BOOL DebugAPIMock::ReadProcessMemory(HANDLE hProcess,
                                     LPCVOID lpBaseAddress,
                                     LPVOID lpBuffer,
                                     SIZE_T nSize,
                                     SIZE_T *lpNumberOfBytesRead) {
  DAPI_PROLOG(ReadProcessMemory);
  return TRUE;
}

BOOL DebugAPIMock::WriteProcessMemory(HANDLE hProcess,
                                  LPVOID lpBaseAddress,
                                  LPCVOID lpBuffer,
                                  SIZE_T nSize,
                                  SIZE_T *lpNumberOfBytesWritten) {
  DAPI_PROLOG(WriteProcessMemory);
  return TRUE;
}

BOOL DebugAPIMock::FlushInstructionCache(HANDLE hProcess,
                                     LPCVOID lpBaseAddress,
                                     SIZE_T dwSize) {
  DAPI_PROLOG(FlushInstructionCache);
  return TRUE;
}

BOOL DebugAPIMock::GetThreadContext(HANDLE hThread, LPCONTEXT lpContext) {
  DAPI_PROLOG(GetThreadContext);
  return TRUE;
}

BOOL DebugAPIMock::SetThreadContext(HANDLE hThread, CONTEXT *lpContext) {
  DAPI_PROLOG(SetThreadContext);
  single_step_enabled_ = (lpContext->EFlags & (1 << 8)) > 0;
  return TRUE;
}

BOOL DebugAPIMock::WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent,
                                 DWORD dwMilliseconds) {
  DAPI_PROLOG(WaitForDebugEvent);
  if (events_.size() == 0)
    return FALSE;

  *lpDebugEvent = events_[0];
  events_.pop_front();
  return TRUE;
}

BOOL DebugAPIMock::ContinueDebugEvent(DWORD dwProcessId,
                                  DWORD dwThreadId,
                                  DWORD dwContinueStatus) {
  DAPI_PROLOG(ContinueDebugEvent);
  return TRUE;
}

BOOL DebugAPIMock::TerminateThread(HANDLE hThread, DWORD dwExitCode) {
  DAPI_PROLOG(TerminateThread);
  return TRUE;
}

BOOL DebugAPIMock::DebugBreakProcess(HANDLE Process) {
  DAPI_PROLOG(DebugBreakProcess);
  return TRUE;
}

BOOL DebugAPIMock::Wow64GetThreadContext(HANDLE hThread,
                                         PWOW64_CONTEXT lpContext) {
  DAPI_PROLOG(Wow64GetThreadContext);
  return TRUE;
}

BOOL DebugAPIMock::Wow64SetThreadContext(HANDLE hThread,
                                     const WOW64_CONTEXT* lpContext) {
  DAPI_PROLOG(Wow64SetThreadContext);
  return TRUE;
}

BOOL DebugAPIMock::IsWoW64Process(HANDLE hProcess, PBOOL Wow64Process) {
  DAPI_PROLOG(IsWoW64Process);
  return TRUE;
}

BOOL DebugAPIMock::DebugActiveProcess(DWORD dwProcessId) {
  DAPI_PROLOG(DebugActiveProcess);
  return TRUE;
}

BOOL DebugAPIMock::DebugActiveProcessStop(DWORD dwProcessId) {
  DAPI_PROLOG(DebugActiveProcessStop);
  return TRUE;
}
}  // namespace debug

