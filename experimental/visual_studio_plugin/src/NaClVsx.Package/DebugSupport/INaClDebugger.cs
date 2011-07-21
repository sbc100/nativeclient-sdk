// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Runtime.CompilerServices;
using Google.MsAd7.BaseImpl.Interfaces;

#endregion

namespace Google.NaClVsx.DebugSupport {
  public interface INaClDebugger : ISimpleDebugger {
    [MethodImpl(MethodImplOptions.Synchronized)]
    bool HasBreakpoint(ulong addr);

    [MethodImpl(MethodImplOptions.Synchronized)]
    void AddTempBreakpoint(ulong addr);

    [MethodImpl(MethodImplOptions.Synchronized)]
    void RemoveTempBreakpoints();

    [MethodImpl(MethodImplOptions.Synchronized)]
    void Open(string connectionString);

    [MethodImpl(MethodImplOptions.Synchronized)]
    void Close();

    event SimpleDebuggerTypes.MessageHandler Opened;
  }
}
