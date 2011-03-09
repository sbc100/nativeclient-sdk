using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  class ProgramNode : IDebugProgramNode2
  {
    public int GetProgramName(out string pbstrProgramName) {
      Debug.WriteLine("ProgramNode.GetProgramName");

      pbstrProgramName = progName_;
      return VSConstants.S_OK;
    }

    public int GetHostName(enum_GETHOSTNAME_TYPE dwHostNameType, out string pbstrHostName) {
      Debug.WriteLine("ProgramNode.GetHostName");

      pbstrHostName = hostName_;
      return VSConstants.S_OK;
    }

    public int GetHostPid(AD_PROCESS_ID[] pHostProcessId) {
      Debug.WriteLine("ProgramNode.GetHostPid");

      pHostProcessId[0] = hostPid_;
      return VSConstants.S_OK;
    }

    public int GetHostMachineName_V7(out string pbstrHostMachineName) {
      Debug.WriteLine("ProgramNode.GetHostMachineName_V7");

      pbstrHostMachineName = hostMachineName_;
      return VSConstants.S_OK;
    }

    public int Attach_V7(IDebugProgram2 pMDMProgram, IDebugEventCallback2 pCallback, uint dwReason) {
      Debug.WriteLine("ProgramNode.Attach_V7");

      throw new NotImplementedException("ProgramNode.Attach_V7");
    }

    public int GetEngineInfo(out string pbstrEngine, out Guid pguidEngine) {
      Debug.WriteLine("ProgramNode.GetEngineInfo");

      throw new NotImplementedException("ProgramNode.Attach_V7");
    }

    public int DetachDebugger_V7() {
      Debug.WriteLine("ProgramNode.DetachDebugger_V7");

      throw new NotImplementedException("ProgramNode.DetachDebugger_V7");
    }

    private string progName_ = "ProgramNode.ProgramName";
    private string hostName_ = "ProgramNode.HostName";
    private AD_PROCESS_ID hostPid_ = new AD_PROCESS_ID();
    private string hostMachineName_ = "ProgramNode.HostMachineName";
  }
}
