// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl.DebugProperties {
  public class RegisterSet : DebugPropertyBase, ICloneable {
    public RegisterSet(RegisterSetSchema schema,
                       DebugPropertyBase parent)
        : base(
            parent,
            schema.Name,
            "<typeless>",
            null,
            0,
            enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_NONE,
            null) {
      schema_ = schema;

      foreach (var registerDef in schema.Registers) {
        var register = new Register(this, registerDef.Name);
        registersByIndex_.Add(registerDef.Index, register);
        registersByName_.Add(registerDef.Name, register);
      }
    }

    public ulong this[int index] {
      get { return Convert.ToUInt64(registersByIndex_[index].Value); }
      set { registersByIndex_[index].Value = value; }
    }

    public ulong this[string name]{
      get { return Convert.ToUInt64(registersByName_[name].Value); }
      set { registersByName_[name].Value = value; }
    }

    private RegisterSetSchema schema_;
    private readonly Dictionary<int, Register> registersByIndex_ = new Dictionary<int, Register>();
    private readonly Dictionary<string, Register> registersByName_ = new Dictionary<string, Register>();

    #region Implementation of ICloneable

    /// <summary>
    /// Creates a new object that is a copy of the current instance.
    /// </summary>
    /// <returns>
    /// A new object that is a copy of this instance.
    /// </returns>
    /// <filterpriority>2</filterpriority>
    public object Clone() {
      RegisterSet other = new RegisterSet(schema_, null);
      foreach (KeyValuePair<int, Register> pair in registersByIndex_) {
        other[pair.Key] = Convert.ToUInt64(pair.Value.Value);
      }
      return other;
    }

    #endregion
  }
}
