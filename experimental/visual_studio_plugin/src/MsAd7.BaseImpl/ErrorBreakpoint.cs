// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  /// <summary>
  /// For documentation of IDebugErrorBreakpoint2 members, please see the VS 2008 MSDN
  /// documentation.
  /// </summary>
  public class ErrorBreakpoint : IDebugErrorBreakpoint2 {
    public ErrorBreakpoint(ErrorBreakpointResolution resolution) {
      resolution_ = resolution;
      pendingBreakpoint_ = null;
    }

    /// <summary>
    /// This property has a public setter so the PendingBreakpoint can add itself to the error
    /// after BreakpointInfo generates it.
    /// </summary>
    public PendingBreakpoint PendingBreakpoint {
      set { pendingBreakpoint_ = value; }
    }

    #region IDebugErrorBreakpoint2 Members

    public int GetPendingBreakpoint(
        out IDebugPendingBreakpoint2 ppPendingBreakpoint) {
      ppPendingBreakpoint = null;
      var haveBreakpoint = VSConstants.S_FALSE;
      if (pendingBreakpoint_ != null) {
        ppPendingBreakpoint = pendingBreakpoint_;
        haveBreakpoint = VSConstants.S_OK;
      }
      return haveBreakpoint;
    }

    public int GetBreakpointResolution(
        out IDebugErrorBreakpointResolution2 ppErrorResolution) {
      ppErrorResolution = resolution_;
      return VSConstants.S_OK;
    }

    #endregion

    #region Private Implementation

    // The resolution of the error if there is one.
    private readonly ErrorBreakpointResolution resolution_;
    // the PendingBreakpoint that caused this error.
    private PendingBreakpoint pendingBreakpoint_;

    #endregion
  }
}
