// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Google.NaClVsx.DebugSupport;
using Google.NaClVsx.DebugSupport.DWARF;

namespace NaClVsx.Package_UnitTestProject {
  public class SymbolProviderMock : ISimpleSymbolProvider {
    #region ISimpleSymbolProvider Members

    public IEnumerable<ulong> AddressesFromPosition(DocumentPosition pos) {
      throw new NotImplementedException();
    }

    public DocumentPosition PositionFromAddress(ulong address) {
      throw new NotImplementedException();
    }

    public IBreakpointInfo GetBreakpointInfo() {
      return new BreakpointInfo(new SymbolDatabase(), this);
    }

    public ulong GetBaseAddress() {
      throw new NotImplementedException();
    }

    public IEnumerable<Symbol> GetSymbolsInScope(ulong instructionAddress) {
      throw new NotImplementedException();
    }

    public IEnumerable<ulong> GetAddressesInScope(ulong programCounter) {
      throw new NotImplementedException();
    }

    public string SymbolValueToString(ulong key, ArraySegment<byte> arrBytes) {
      throw new NotImplementedException();
    }

    public Function FunctionFromAddress(ulong address) {
      throw new NotImplementedException();
    }

    public FunctionDetails GetFunctionDetails(Function fn) {
      throw new NotImplementedException();
    }

    public bool LoadModule(string path, ulong loadOffset, out string status) {
      throw new NotImplementedException();
    }

    public ulong GetNextLocation(ulong addr) {
      throw new NotImplementedException();
    }

    #endregion
  }
}
