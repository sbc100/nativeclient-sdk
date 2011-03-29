// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  /// <summary>
  /// For documentation of IDebugBreakpointRequest2 members, please see the VS 2008 MSDN
  /// documentation.
  /// </summary>
  public class BreakpointRequest : IDebugBreakpointRequest2
  {
    /// <summary>
    /// Allows our implementation of the Debug Engine to construct an implemenation of this
    /// interface that is specific to our purpose, from the interface whose implementation is
    /// opaque to us.
    /// </summary>
    /// <param name="rq">A breakpoint request whose implementation is opaque to this class.</param>
    public BreakpointRequest(IDebugBreakpointRequest2 rq) {
      var loctype = new enum_BP_LOCATION_TYPE[1];
      rq.GetLocationType(loctype);
      locationType_ = loctype[0];

      var inf = new BP_REQUEST_INFO[1];
      rq.GetRequestInfo(
          enum_BPREQI_FIELDS.BPREQI_ALLFIELDS,
          inf);
      requestInfo_ = inf[0];

      //
      // BP_LOCATIONis a discriminated union in C+, and seems to
      // have been ported to C# rather carelessly. This code follows
      // the instructions in the MSDN docs at
      // http://msdn.microsoft.com/en-us/library/bb162191(v=VS.80).aspx
      //
      if ((locationType_ & enum_BP_LOCATION_TYPE.BPLT_FILE_LINE) != 0) {
        DocPos =
            new DocumentPosition(
                (IDebugDocumentPosition2)
                Marshal.GetObjectForIUnknown(
                    requestInfo_.bpLocation.unionmember2));
      } else {
        throw new ArgumentException("Unsupported breakpoint location type");
      }
    }

    public enum_BP_LOCATION_TYPE LocationType {
      get { return locationType_; }
    }

    public BP_REQUEST_INFO RequestInfo {
      get { return requestInfo_; }
    }

    public DocumentPosition DocPos { get; set; }

    #region Implementation of IDebugBreakpointRequest2

    public int GetLocationType(enum_BP_LOCATION_TYPE[] pBPLocationType) {
      pBPLocationType[0] = locationType_;
      return VSConstants.S_OK;
    }

    public int GetRequestInfo(enum_BPREQI_FIELDS dwFields,
                              BP_REQUEST_INFO[] pBPRequestInfo) {
      pBPRequestInfo[0] = requestInfo_;
      return VSConstants.S_OK;
    }

    #endregion

    #region Private Implementation

    private readonly enum_BP_LOCATION_TYPE locationType_ =
        enum_BP_LOCATION_TYPE.BPLT_NONE;

    private readonly BP_REQUEST_INFO requestInfo_;

    #endregion
  }
}
