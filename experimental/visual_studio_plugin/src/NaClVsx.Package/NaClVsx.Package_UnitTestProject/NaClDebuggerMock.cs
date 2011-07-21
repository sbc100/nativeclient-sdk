// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.MsAd7.BaseImpl.TestUtils;
using Google.NaClVsx.DebugSupport;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  public class NaClDebuggerMock : MockBase, INaClDebugger {
    #region INaClDebugger Members

    public ISimpleSymbolProvider Symbols { get; set; }

    public string Architecture {
      get { throw new NotImplementedException(); }
    }

    public ulong BaseAddress {
      get { throw new NotImplementedException(); }
    }

    public event SimpleDebuggerTypes.EventHandler Stopped;
    public event SimpleDebuggerTypes.EventHandler StepFinished;
    public event SimpleDebuggerTypes.EventHandler Continuing;
    public event SimpleDebuggerTypes.MessageHandler Output;
    public event SimpleDebuggerTypes.ModuleLoadHandler ModuleLoaded;

    public void Break() {
      throw new NotImplementedException();
    }

    public void Step(uint id) {
      throw new NotImplementedException();
    }

    public void Continue() {
      throw new NotImplementedException();
    }

    public void AddBreakpoint(ulong addr) {
      throw new NotImplementedException();
    }

    public void RemoveBreakpoint(ulong addr) {
      throw new NotImplementedException();
    }

    public IEnumerable<uint> GetThreads() {
      throw new NotImplementedException();
    }

    public object GetRegisters(uint id) {
      return CheckCall<uint, object>("GetRegisters", id);
    }

    public void GetMemory(ulong sourceAddress,
                          Array destination,
                          uint countInBytes) {
      throw new NotImplementedException();
    }

    public void SetMemory(ulong destAddress, Array src, uint countInBytes) {
      throw new NotImplementedException();
    }

    public ulong GetU64(ulong address) {
      return CheckCall<ulong, ulong>("GetU64", address);
    }

    public uint GetU32(ulong address) {
      throw new NotImplementedException();
    }

    public bool HasBreakpoint(ulong addr) {
      throw new NotImplementedException();
    }

    public void AddTempBreakpoint(ulong addr) {
      throw new NotImplementedException();
    }

    public void RemoveTempBreakpoints() {
      throw new NotImplementedException();
    }

    public void Open(string connectionString) {
      throw new NotImplementedException();
    }

    public void Close() {
      throw new NotImplementedException();
    }

    public event SimpleDebuggerTypes.MessageHandler Opened;

    #endregion
  }
}
