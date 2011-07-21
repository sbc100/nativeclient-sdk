// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using Google.MsAd7.BaseImpl.TestUtils;
using Microsoft.VisualStudio.Debugger.Interop;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  class DebugPort2Mock : MockBase, IDebugPort2 {
    #region IDebugPort2 Members

    public int GetPortName(out string pbstrName) {
      throw new NotImplementedException();
    }

    public int GetPortId(out Guid pguidPort) {
      throw new NotImplementedException();
    }

    public int GetPortRequest(out IDebugPortRequest2 ppRequest) {
      throw new NotImplementedException();
    }

    public int GetPortSupplier(out IDebugPortSupplier2 ppSupplier) {
      throw new NotImplementedException();
    }

    public int GetProcess(AD_PROCESS_ID ProcessId, out IDebugProcess2 ppProcess) {
      throw new NotImplementedException();
    }

    public int EnumProcesses(out IEnumDebugProcesses2 ppEnum) {
      throw new NotImplementedException();
    }

    #endregion
  }
}
