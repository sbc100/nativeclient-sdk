// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using Microsoft.VisualStudio.Debugger.Interop;

#endregion

namespace MsAd7.BaseImpl_UnitTestProject {
  class DebugThread2Mock : IDebugThread2 {
    #region IDebugThread2 Members

    public int EnumFrameInfo(enum_FRAMEINFO_FLAGS dwFieldSpec,
                             uint nRadix,
                             out IEnumDebugFrameInfo2 ppEnum) {
      throw new NotImplementedException();
    }

    public int GetName(out string pbstrName) {
      throw new NotImplementedException();
    }

    public int SetThreadName(string pszName) {
      throw new NotImplementedException();
    }

    public int GetProgram(out IDebugProgram2 ppProgram) {
      throw new NotImplementedException();
    }

    public int CanSetNextStatement(IDebugStackFrame2 pStackFrame,
                                   IDebugCodeContext2 pCodeContext) {
      throw new NotImplementedException();
    }

    public int SetNextStatement(IDebugStackFrame2 pStackFrame,
                                IDebugCodeContext2 pCodeContext) {
      throw new NotImplementedException();
    }

    public int GetThreadId(out uint pdwThreadId) {
      throw new NotImplementedException();
    }

    public int Suspend(out uint pdwSuspendCount) {
      throw new NotImplementedException();
    }

    public int Resume(out uint pdwSuspendCount) {
      throw new NotImplementedException();
    }

    public int GetThreadProperties(enum_THREADPROPERTY_FIELDS dwFields,
                                   THREADPROPERTIES[] ptp) {
      throw new NotImplementedException();
    }

    public int GetLogicalThread(IDebugStackFrame2 pStackFrame,
                                out IDebugLogicalThread2 ppLogicalThread) {
      throw new NotImplementedException();
    }

    #endregion
  }
}
