// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  /// <summary>
  /// For documentation of IDebugErrorBreakpointResolution2 members, please see the VS 2008 MSDN
  /// documentation.
  /// </summary>
  public class ErrorBreakpointResolution : IDebugErrorBreakpointResolution2
  {
    public ErrorBreakpointResolution() {
      resolutionInfo_.dwFields = 0;
      resolutionInfo_.bpResLocation.unionmember1 = IntPtr.Zero;
    }

    /// <summary>
    /// Can be used to set the location of the error point that results from error resolution, or
    /// other resolution information.
    /// </summary>
    /// <param name="bpType">The type of breakpoint being resolved.</param>
    /// <param name="address">The address of the code referenced by the resolved breakpoint.</param>
    /// <param name="docContext">The location of the breakpoint in the code, after resolution.</param>
    public void SetLocation(enum_BP_TYPE bpType,
                            ulong address,
                            DocumentContext docContext) {
      var codeContext =
          new CodeContext(
              string.Format("{0}:{1}", docContext.Path, docContext.Line),
              address,
              1,
              docContext);

      resolutionInfo_.bpResLocation.bpType = (uint) bpType;

      if (resolutionInfo_.bpResLocation.unionmember1 != IntPtr.Zero) {
        Marshal.Release(resolutionInfo_.bpResLocation.unionmember1);
        resolutionInfo_.bpResLocation.unionmember1 = IntPtr.Zero;
      }
      resolutionInfo_.bpResLocation.unionmember1 =
          Marshal.GetIUnknownForObject(codeContext);

      resolutionInfo_.dwFields = resolutionInfo_.dwFields |
                                 enum_BPERESI_FIELDS.BPERESI_BPRESLOCATION;
    }

    /// <summary>
    /// Set the error message field.
    /// </summary>
    /// <value>The message to display to the user.</value>
    public string Message {
      set {
        resolutionInfo_.bstrMessage = value;
        resolutionInfo_.dwFields = resolutionInfo_.dwFields |
                                   enum_BPERESI_FIELDS.BPERESI_MESSAGE;
      }
    }

    /// <summary>
    /// Sets the type field.
    /// </summary>
    /// <value>The type and severity of the problem that was encountered.</value>
    public enum_BP_ERROR_TYPE Type {
      set {
        resolutionInfo_.dwType = value;
        resolutionInfo_.dwFields = resolutionInfo_.dwFields |
                                   enum_BPERESI_FIELDS.BPERESI_TYPE;
      }
      get { return resolutionInfo_.dwType; }
    }

    #region Implementation of IDebugBreakpointResolution2

    public int GetBreakpointType(enum_BP_TYPE[] pBPType) {
      Debug.WriteLine("ErrorBreakpointResolution.GetBreakpointType");
      pBPType[0] = enum_BP_TYPE.BPT_CODE;
      return VSConstants.S_OK;
    }

    public int GetResolutionInfo(enum_BPERESI_FIELDS dwFields,
                                 BP_ERROR_RESOLUTION_INFO[] pBpResolutionInfo) {
      int rValue = VSConstants.S_FALSE;
      if (resolutionInfo_.dwFields != 0 && pBpResolutionInfo != null) {
        if (((int) dwFields & (int) enum_BPERESI_FIELDS.BPERESI_ALLFIELDS) ==
            (int) enum_BPRESI_FIELDS.BPRESI_ALLFIELDS) {
          pBpResolutionInfo[0] = resolutionInfo_;
          rValue = VSConstants.S_OK;
        } else {
          pBpResolutionInfo[0].dwFields = dwFields & resolutionInfo_.dwFields;

          int locMask = (int) pBpResolutionInfo[0].dwFields &
                        (int) enum_BPERESI_FIELDS.BPERESI_BPRESLOCATION;
          int msgMask = (int) pBpResolutionInfo[0].dwFields &
                        (int) enum_BPERESI_FIELDS.BPERESI_MESSAGE;
          int progMask = (int) pBpResolutionInfo[0].dwFields &
                         (int) enum_BPERESI_FIELDS.BPERESI_PROGRAM;
          int threadMask = (int) pBpResolutionInfo[0].dwFields &
                           (int) enum_BPERESI_FIELDS.BPERESI_THREAD;
          int typeMask = (int) pBpResolutionInfo[0].dwFields &
                         (int) enum_BPERESI_FIELDS.BPERESI_TYPE;

          if (locMask != 0) {
            pBpResolutionInfo[0].bpResLocation = resolutionInfo_.bpResLocation;
          }
          if (msgMask != 0) {
            pBpResolutionInfo[0].bstrMessage = resolutionInfo_.bstrMessage;
          }
          if (progMask != 0) {
            pBpResolutionInfo[0].pProgram = resolutionInfo_.pProgram;
          }
          if (threadMask != 0) {
            pBpResolutionInfo[0].pThread = resolutionInfo_.pThread;
          }
          if (typeMask != 0) {
            pBpResolutionInfo[0].dwType = resolutionInfo_.dwType;
          }
          rValue = VSConstants.S_OK;
        }
      }
      return rValue;
    }

    #endregion

    #region Private Implementation

    private BP_ERROR_RESOLUTION_INFO resolutionInfo_;

    #endregion
  }
}
