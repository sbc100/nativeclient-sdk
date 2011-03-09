// Copyright 2009 The Native Client Authors. All rights reserved.
// 
// Use of this source code is governed by a BSD-style license that can
// 
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.NaClVsx.DebugSupport {
  public class NaClDebugProcess : DebugProcess {
    public NaClDebugProcess(IDebugPort2 port, string connectionString, int pid, string imagePath)
      : base(port, pid, imagePath) {
      connectionString_ = connectionString;
    }

    public NaClPort NaClPort {
     get { return (DebugSupport.NaClPort) this.Port;}
    }

    #region Overrides of DebugProcess

    public override enum_PROCESS_INFO_FLAGS GetProcessStatus() {
      return enum_PROCESS_INFO_FLAGS.PIFLAG_PROCESS_RUNNING;
    }

    protected override ICollection<IDebugProgram2> GetPrograms() {
      Refresh();
      return programs_;
    }

    #endregion

    void Refresh()
    {
      if (programs_.Count > 0) return;
      programs_.Add(new ProgramNode(this));
    }


    List<IDebugProgram2> programs_ = new List<IDebugProgram2>();
    private string connectionString_;
  }
}
