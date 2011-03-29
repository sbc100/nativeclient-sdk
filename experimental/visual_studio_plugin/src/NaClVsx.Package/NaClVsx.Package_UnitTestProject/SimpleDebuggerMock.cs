// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl.Interfaces;

namespace NaClVsx.Package_UnitTestProject {
  public class SimpleDebuggerMock : ISimpleDebugger {
    #region ISimpleDebugger Members

    public ISimpleSymbolProvider Symbols {
      get { return new SymbolProviderMock(); }
    }

    public string Architecture {
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

    #endregion
  }
}
