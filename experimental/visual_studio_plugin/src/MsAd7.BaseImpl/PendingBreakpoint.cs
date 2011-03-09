// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.Interfaces;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  /// <summary>
  /// For documentation of IDebugPendingBreakpoint2 members, please see the VS 2008 MSDN
  /// documentation.
  /// </summary>
  public class PendingBreakpoint : IDebugPendingBreakpoint2
  {
    /// <summary>
    /// This custom constructor ensures the class gets everything it needs to know.
    /// </summary>
    /// <param name="dbg">A debugger for address lookups.</param>
    /// <param name="rq">A request that can be queried for code context information.</param>
    public PendingBreakpoint(ISimpleDebugger dbg, BreakpointRequest rq) {
      dbg_ = dbg;
      rq_ = rq;
      breakpointInfo_ = dbg_.Symbols.GetBreakpointInfo();
      errorBreakpoints_ = null;
    }

    #region Implementation of IDebugPendingBreakpoint2

    public int CanBind(out IEnumDebugErrorBreakpoints2 ppErrorEnum) {
      Debug.WriteLine("PendingBreakpoint.CanBind");
      int rValue = breakpointInfo_.GetBindErrors(rq_.DocPos, out ppErrorEnum);
      if (rValue != VSConstants.S_OK) {
        var typedEnum = ppErrorEnum as ErrorBreakpointEnumerator;
        if (typedEnum != null) {
          typedEnum.PopulateBreakpoint(this);
        }
      }
      return rValue;
    }

    public int Bind() {
      Debug.WriteLine("PendingBreakpoint.Bind");
      IEnumerable<ulong> addresses;
      int rValue = breakpointInfo_.GetBindAddresses(
          rq_.DocPos,
          out addresses,
          out errorBreakpoints_);
      if (rValue != VSConstants.S_OK) {
        var typedEnum = errorBreakpoints_ as ErrorBreakpointEnumerator;
        if (typedEnum != null) {
          typedEnum.PopulateBreakpoint(this);
        }
      }
      if (addresses != null && addresses.Count() > 0) {
        foreach (UInt64 address in addresses) {
          var bp = new BoundBreakpoint(this, address);
          boundBreakpoints_.Add(bp);
          dbg_.AddBreakpoint(address);
        }
      } else {
        rValue = VSConstants.E_FAIL;
      }
      return rValue;
    }

    public int GetState(PENDING_BP_STATE_INFO[] pState) {
      Debug.WriteLine("PendingBreakpoint.GetState");
      if (virtualized_) {
        pState[0].Flags = enum_PENDING_BP_STATE_FLAGS.PBPSF_VIRTUALIZED;
      } else {
        pState[0].Flags = enum_PENDING_BP_STATE_FLAGS.PBPSF_NONE;
      }

      // note: not supporting "deleted" state ATM.
      if (enabled_) {
        pState[0].state = enum_PENDING_BP_STATE.PBPS_ENABLED;
      } else {
        pState[0].state = enum_PENDING_BP_STATE.PBPS_DISABLED;
      }
      return VSConstants.S_OK;
    }

    public int GetBreakpointRequest(out IDebugBreakpointRequest2 ppBPRequest) {
      Debug.WriteLine("PendingBreakpoint.GetBreakpointRequest");
      ppBPRequest = rq_;
      return VSConstants.S_OK;
    }

    public int Virtualize(int fVirtualize) {
      Debug.WriteLine("PendingBreakpoint.Virtualize");
      virtualized_ = fVirtualize != 0;
      return VSConstants.S_OK;
    }

    public int Enable(int fEnable) {
      Debug.WriteLine("PendingBreakpoint.Enable");
      enabled_ = fEnable != 0;
      return VSConstants.S_OK;
    }

    public int SetCondition(BP_CONDITION bpCondition) {
      Debug.WriteLine("PendingBreakpoint.SetCondition");
      throw new NotImplementedException();
    }

    public int SetPassCount(BP_PASSCOUNT bpPassCount) {
      Debug.WriteLine("PendingBreakpoint.SetPassCount");
      throw new NotImplementedException();
    }

    public int EnumBoundBreakpoints(out IEnumDebugBoundBreakpoints2 ppEnum) {
      Debug.WriteLine("PendingBreakpoint.EnumBoundBreakpoints");
      throw new NotImplementedException();
    }

    public int EnumErrorBreakpoints(enum_BP_ERROR_TYPE bpErrorType,
                                    out IEnumDebugErrorBreakpoints2 ppEnum) {
      var matchingBreakpointErrors = new ErrorBreakpointEnumerator();

      uint errorCount;
      if (errorBreakpoints_.GetCount(out errorCount) == VSConstants.S_OK) {
        var breakpointArray = new IDebugErrorBreakpoint2[errorCount];
        uint fetchedCount = 0;
        errorBreakpoints_.Next(errorCount, breakpointArray, ref fetchedCount);
        foreach (IDebugErrorBreakpoint2 point in breakpointArray) {
          IDebugErrorBreakpointResolution2 pointResolution;
          if (point.GetBreakpointResolution(out pointResolution) !=
              VSConstants.S_OK) {
            continue;
          }
          var pointResolutionInfo = new BP_ERROR_RESOLUTION_INFO[1];
          if (pointResolution.GetResolutionInfo(
              enum_BPERESI_FIELDS.BPERESI_TYPE,
              pointResolutionInfo) != VSConstants.S_OK) {
            continue;
          }
          if ((pointResolutionInfo[0].dwType == bpErrorType) ||
              ((int) bpErrorType == -1)) {
            matchingBreakpointErrors.Insert(point);
          }
        }
      }
      ppEnum = matchingBreakpointErrors;
      return VSConstants.S_OK;
    }

    public int Delete() {
      Debug.WriteLine("PendingBreakpoint.Delete");
      foreach (BoundBreakpoint bp in boundBreakpoints_) {
        dbg_.RemoveBreakpoint(bp.Address);
      }
      boundBreakpoints_.Clear();
      enabled_ = false;
      return VSConstants.S_OK;
    }

    #endregion

    #region Private Implementation

    private readonly List<BoundBreakpoint> boundBreakpoints_ =
        new List<BoundBreakpoint>();

    private readonly IBreakpointInfo breakpointInfo_;

    private readonly ISimpleDebugger dbg_;
    private readonly BreakpointRequest rq_;
    private bool enabled_;

    private IEnumDebugErrorBreakpoints2 errorBreakpoints_;
    private bool virtualized_;

    #endregion
  }
}
