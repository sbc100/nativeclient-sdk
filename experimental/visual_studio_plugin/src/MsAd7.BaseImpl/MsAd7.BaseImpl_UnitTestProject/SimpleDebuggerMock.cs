// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl.Interfaces;

#endregion

namespace MsAd7.BaseImpl_UnitTestProject {
  public class SimpleDebuggerMock : ISimpleDebugger {
    #region ISimpleDebugger Members

    public ISimpleSymbolProvider Symbols {
      get { return symbols_; }
    }

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
      throw new NotImplementedException();
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
      throw new NotImplementedException();
    }

    public uint GetU32(ulong address) {
      throw new NotImplementedException();
    }

    #endregion

    #region Private Implementation

    private readonly SymbolProviderMock symbols_ = new SymbolProviderMock();

    #endregion
  }
}
