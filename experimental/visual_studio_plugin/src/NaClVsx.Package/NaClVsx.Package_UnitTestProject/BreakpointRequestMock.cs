// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System.Runtime.InteropServices;
using Google.MsAd7.BaseImpl;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace NaClVsx.Package_UnitTestProject {
  public class BreakpointRequestMock : IDebugBreakpointRequest2 {

    #region IDebugBreakpointRequest2 Members

    public int GetLocationType(enum_BP_LOCATION_TYPE[] pBpLocationType) {
      pBpLocationType[0] = enum_BP_LOCATION_TYPE.BPLT_FILE_LINE;
      return VSConstants.S_OK;
    }

    public int GetRequestInfo(enum_BPREQI_FIELDS dwFields,
                              BP_REQUEST_INFO[] pBPRequestInfo) {
      pBPRequestInfo[0] = new BP_REQUEST_INFO {
          bpLocation = {
              bpLocationType = (uint) enum_BP_LOCATION_TYPE.BPLT_CODE_FILE_LINE,
              unionmember2 = Marshal.GetIUnknownForObject(
                  new DocumentPosition("C:\\test\\path", 123))
          }
      };

      return VSConstants.S_OK;
    }

    #endregion
  }
}
