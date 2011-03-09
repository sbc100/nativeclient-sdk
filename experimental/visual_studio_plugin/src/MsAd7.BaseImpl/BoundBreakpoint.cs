using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  public class BoundBreakpoint : IDebugBoundBreakpoint2
  {
    public BoundBreakpoint(PendingBreakpoint parent, UInt64 address) {
      parent_ = parent;
      address_ = address;
    }

    public PendingBreakpoint Parent {
      get { return parent_; }
    }

    public ulong Address {
      get { return address_; }
    }

    #region Implementation of IDebugBoundBreakpoint2

    public int GetPendingBreakpoint(out IDebugPendingBreakpoint2 ppPendingBreakpoint) {
      Debug.WriteLine("BoundBreakpoint.GetPendingBreakpoint");
      ppPendingBreakpoint = parent_;
      return VSConstants.S_OK;
    }

    public int GetState(enum_BP_STATE[] pState) {
      Debug.WriteLine("BoundBreakpoint.GetState");
      throw new NotImplementedException();
    }

    public int GetHitCount(out uint pdwHitCount) {
      Debug.WriteLine("BoundBreakpoint.GetHitCount");
      throw new NotImplementedException();
    }

    public int GetBreakpointResolution(out IDebugBreakpointResolution2 ppBPResolution) {
      Debug.WriteLine("BoundBreakpoint.GetBreakpointResolution");
      throw new NotImplementedException();
    }
  
    public int Enable(int fEnable) {
      Debug.WriteLine("BoundBreakpoint.Enable");
      throw new NotImplementedException();
    }

    public int SetHitCount(uint dwHitCount) {
      Debug.WriteLine("BoundBreakpoint.SetHitCount");
      throw new NotImplementedException();
    }

    public int SetCondition(BP_CONDITION bpCondition) {
      Debug.WriteLine("BoundBreakpoint.SetCondition");
      throw new NotImplementedException();
    }

    public int SetPassCount(BP_PASSCOUNT bpPassCount) {
      Debug.WriteLine("BoundBreakpoint.SetPassCount");
      throw new NotImplementedException();
    }

    public int Delete() {
      Debug.WriteLine("BoundBreakpoint.Delete");
      throw new NotImplementedException();
    }

    #endregion

    private readonly PendingBreakpoint parent_ = null;
    private ulong address_;
  }
}
