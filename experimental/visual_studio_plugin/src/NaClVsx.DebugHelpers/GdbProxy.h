// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
#ifndef NACLVSX_DEBUGHELPERS_TRANSPORT_H_
#define NACLVSX_DEBUGHELPERS_TRANSPORT_H_

#include "src/NaClVsx.DebugHelpers/Registers.h"

namespace NaClVsx {
namespace DebugHelpers {

struct GdbProxyImpl;

public ref class GdbProxy {
  public:
    GdbProxy(void);
    ~GdbProxy();

    //
    // Please keep this enum in sync with the one in debug_host.h
    //
    enum class ResultCode {
      DHR_BUSY   = -4,   // Target is busy (running)
      DHR_FAILED = -3,   // Transaction completed with failure
      DHR_LOST = -2,     // Lost connection during transaction
      DHR_TIMEOUT =-1,   // Transaction Timed out
      DHR_PENDING = 0,   // Transaction is pending as expected
      DHR_OK = 1         // Transaction Succeeded
    };
    delegate void AsyncResponse(ResultCode result,
                                       System::String^ msg,
                                       array<System::Byte>^ data);

    static bool CanConnect(System::String^ connectionString);
    static bool CanConnect(int pid) {
      pid;  // unreferenced
      return true;
    }

    void Open(System::String^ connectionString);
    void Close();

    bool IsRunning();
    void SetOutputAsync(AsyncResponse^ reply);
    void SetStopAsync(AsyncResponse^ reply);

    ResultCode GetPath(AsyncResponse^ reply);
    ResultCode GetArch(AsyncResponse^ reply);
    ResultCode GetThreads(AsyncResponse^ reply);

    ResultCode GetLastSig([System::Runtime::InteropServices::Out]int% sig);

    ResultCode GetMemory(System::UInt64 offs, System::Object^ data);
    ResultCode GetMemory(
      System::UInt64 offs, System::Array^ data, System::UInt32 count);
    ResultCode SetMemory(
      System::UInt64 offs, System::Array^ data, System::UInt32 count);

    ResultCode GetRegisters(RegsX86_64^% registers);
    ResultCode SetRegisters(void *data, System::UInt32 size);

    ResultCode RequestBreak();
    ResultCode RequestContinueBackground();
    ResultCode RequestContinue();
    ResultCode RequestStep();

    bool HasBreakpoint(System::UInt64 offs);
    ResultCode AddBreakpoint(System::UInt64 offs);
    ResultCode RemoveBreakpoint(System::UInt64 offs);
//    ResultCode EnableBreakpoint(System::UInt64 offs);
//    ResultCode DisableBreakpoint(System::UInt64 offs);
//    ResultCode SuspendBreakpoint(System::UInt64 offs);
//    ResultCode ResumeBreakpoint(System::UInt64 offs);
    bool       QueryBreakpoint(System::UInt64 offs);

  private:
    System::String^ connectionString_;
    GdbProxyImpl* pimpl_;
  };
}  // namespace DebugHelpers
}  // namespace NaClVsx


#endif  // NACLVSX_DEBUGHELPERS_TRANSPORT_H_

