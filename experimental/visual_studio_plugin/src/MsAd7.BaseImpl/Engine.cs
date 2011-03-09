using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  class Engine : IDebugEngine2
  {
    public int EnumPrograms(out IEnumDebugPrograms2 ppEnum) {
      Debug.WriteLine("Engine.EnumPrograms");

      throw new NotImplementedException();
    }

    public int Attach(IDebugProgram2[] rgpPrograms, IDebugProgramNode2[] rgpProgramNodes, uint celtPrograms, IDebugEventCallback2 pCallback, enum_ATTACH_REASON dwReason) {
      Debug.WriteLine("Engine.Attach");

      throw new NotImplementedException();
    }

    public int CreatePendingBreakpoint(IDebugBreakpointRequest2 pBPRequest, out IDebugPendingBreakpoint2 ppPendingBP) {
      Debug.WriteLine("Engine.CreatePendingBreakpoint");

      throw new NotImplementedException();
    }

    public int SetException(EXCEPTION_INFO[] pException) {
      Debug.WriteLine("Engine.SetException");

      throw new NotImplementedException();
    }

    public int RemoveSetException(EXCEPTION_INFO[] pException) {
      Debug.WriteLine("Engine.RemoveSetException");

      throw new NotImplementedException();
    }

    public int RemoveAllSetExceptions(ref Guid guidType) {
      Debug.WriteLine("Engine.RemoveAllSetExceptions");

      throw new NotImplementedException();
    }

    public int GetEngineId(out Guid pguidEngine) {
      Debug.WriteLine("Engine.GetEngineId");

      throw new NotImplementedException();
    }

    public int DestroyProgram(IDebugProgram2 pProgram) {
      Debug.WriteLine("Engine.DestroyProgram");

      throw new NotImplementedException();
    }

    public int ContinueFromSynchronousEvent(IDebugEvent2 pEvent) {
      Debug.WriteLine("Engine.ContinueFromSynchronousEvent");

      throw new NotImplementedException();
    }

    public int SetLocale(ushort wLangID) {
      Debug.WriteLine("Engine.SetLocale");

      throw new NotImplementedException();
    }

    public int SetRegistryRoot(string pszRegistryRoot) {
      Debug.WriteLine("Engine.SetRegistryRoot");

      throw new NotImplementedException();
    }

    public int SetMetric(string pszMetric, object varValue) {
      Debug.WriteLine("Engine.SetMetric");

      throw new NotImplementedException();
    }

    public int CauseBreak() {
      Debug.WriteLine("Engine.CauseBreak");

      throw new NotImplementedException();
    }
  }
}
