// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl.DebugProperties {
  class Register : DebugPropertyBase {
    public Register(DebugPropertyBase parent, string name)
        : base(parent,
               name,
               "int",
               0,
               0,
               enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_DATA
               | enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_STORAGE_REGISTER,
               null) {}

    public override string FormatValue() {
      return string.Format("{0:X16}", Value);
    }
  }
}
