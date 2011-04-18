// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Diagnostics;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl.DebugProperties {
  class Variable : DebugPropertyBase {
    public Variable(DebugPropertyBase parent,
                    Symbol sym,
                    enum_DBG_ATTRIB_FLAGS attributes,
                    ISimpleDebugger dbg)
        : base(parent, sym.Name, sym.TypeOf.Name, null, sym.Offset, attributes, dbg) {
      symbol_ = sym;
    }

    protected override void RefreshValue(ref object value)
    {
      var result = new byte[symbol_.TypeOf.SizeOf];
      Debug.WriteLine("RefreshValue, variable=" + symbol_.Name +
                      " Address: " + String.Format("{0,4:X}", Address));
      Dbg.GetMemory(Address, result, (uint) result.Length);
      value = result;
    }

    public override string FormatValue()
    {
      return Dbg.Symbols.SymbolValueToString(
          symbol_.Key, new ArraySegment<byte>((byte[]) Value));
    }

    private Symbol symbol_;
  }
}
