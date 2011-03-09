// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  namespace Ad7Enumerators {
  /// <summary>
  /// For documentation of IEnumDebugErrorBreakpoint2 members, please see the VS 2008 MSDN
  /// documentation.
  /// </summary>
  public class EnumErrorBreakpoints : IEnumDebugErrorBreakpoints2 {
    public EnumErrorBreakpoints() {
      breakpointErrors_ = new List<IDebugErrorBreakpoint2>();
      iterator_ = 0;
    }

    /// <summary>
    /// This function exists so the PendingBreakpoint can add itself to the errors in this
    /// enumeration after BreakpointInfo generates them.
    /// </summary>
    public int PopulateBreakpoint(PendingBreakpoint breakpoint) {
      int populated = VSConstants.S_FALSE;
      foreach (
          ErrorBreakpoint assignable in
              breakpointErrors_.OfType<ErrorBreakpoint>()) {
        assignable.PendingBreakpoint = breakpoint;
        populated = VSConstants.S_OK;
      }
      return populated;
    }

    public void Insert(IDebugErrorBreakpoint2 breakpointError) {
      breakpointErrors_.Add(breakpointError);
    }

    #region IEnumDebugErrorBreakpoints2 Members

    public int Next(uint celt,
                    IDebugErrorBreakpoint2[] rgelt,
                    ref uint pceltFetched) {
      int inRange = VSConstants.S_FALSE;
      var requestedSize = (int) celt;
      if ((requestedSize > 0) &&
          (iterator_ + requestedSize <= breakpointErrors_.Count)) {
        breakpointErrors_.CopyTo(iterator_, rgelt, 0, requestedSize);
        iterator_ += requestedSize;
        pceltFetched = celt;
        inRange = VSConstants.S_OK;
      }
      return inRange;
    }

    public int Skip(uint celt) {
      int inRange = VSConstants.S_FALSE;
      var requestedSize = (int) celt;
      if ((requestedSize > 0) &&
          (iterator_ + celt) < breakpointErrors_.Count) {
        iterator_ += requestedSize;
        inRange = VSConstants.S_OK;
      }
      return inRange;
    }

    public int Reset() {
      iterator_ = 0;
      return VSConstants.S_OK;
    }

    public int Clone(out IEnumDebugErrorBreakpoints2 ppEnum) {
      int success = VSConstants.S_FALSE;
      var clone = new EnumErrorBreakpoints();
      ppEnum = clone;
      foreach (IDebugErrorBreakpoint2 debugErrorBreakpoint2 in breakpointErrors_
          ) {
        clone.Insert(debugErrorBreakpoint2);
      }
      uint cloneCount;
      clone.GetCount(out cloneCount);
      if (breakpointErrors_.Count == cloneCount) {
        success = VSConstants.S_OK;
      }
      return success;
    }

    public int GetCount(out uint pcelt) {
      int rValue = VSConstants.S_FALSE;
      pcelt = 0;
      if (breakpointErrors_ != null) {
        pcelt = (uint) breakpointErrors_.Count;
        rValue = VSConstants.S_OK;
      }
      return rValue;
    }

    #endregion

    #region Private Implementation

    readonly List<IDebugErrorBreakpoint2> breakpointErrors_;
    int iterator_;

    #endregion
  }
}