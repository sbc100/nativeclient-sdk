// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Google.MsAd7.BaseImpl.TestUtils;

namespace MsAd7.BaseImpl_UnitTestProject {
  public class SymbolProviderMock : MockBase, ISimpleSymbolProvider {
    #region ISimpleSymbolProvider Members

    /// <summary>
    /// Throws KeyNotFoundException if the call cannot be validated by the
    /// mock.
    /// </summary>
    /// <param name="pos"></param>
    /// <returns></returns>
    public IEnumerable<ulong> AddressesFromPosition(DocumentPosition pos) {
      return CheckCall<DocumentPosition, IEnumerable<ulong>>(
          "AddressesFromPosition", pos);
    }

    public DocumentPosition PositionFromAddress(ulong address) {
      return CheckCall<ulong, DocumentPosition>("PositionFromAddress", address);
    }

    public IBreakpointInfo GetBreakpointInfo() {
      throw new NotImplementedException();
    }

    public ulong GetBaseAddress() {
      throw new NotImplementedException();
    }

    public IEnumerable<Symbol> GetSymbolsInScope(ulong instructionAddress) {
      return CheckCall<ulong, IEnumerable<Symbol>>(
          "GetSymbolsInScope", instructionAddress);
    }

    public IEnumerable<ulong> GetAddressesInScope(ulong programCounter) {
      throw new NotImplementedException();
    }

    public string SymbolValueToString(ulong key, ArraySegment<byte> arrBytes) {
      throw new NotImplementedException();
    }

    public Function FunctionFromAddress(ulong address) {
      return CheckCall<ulong, Function>("FunctionFromAddress", address);
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
