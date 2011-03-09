// Copyright 2009 The Native Client Authors. All rights reserved.
// 
// Use of this source code is governed by a BSD-style license that can
// 
// be found in the LICENSE file.
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  public class PortRequest : IDebugPortRequest2 {
    public PortRequest(string name) {
      name_ = name;
    }

    public string Name {
      get { return name_; }
      set { name_ = value; }
    }

    #region Private Implementation

    private string name_;

    #endregion

    #region Implementation of IDebugPortRequest2

    public int GetPortName(out string pbstrPortName) {
      pbstrPortName = name_;
      return VSConstants.S_OK;
    }

    #endregion
  }
}
