// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System.Collections.Generic;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl.Interfaces {
  public interface IBreakpointInfo {
    // This interface exists to implement a subset of the functionality that will be
    // asked of the SymbolProvider.  Specifically information related to breakpoints.

    // Populates the |ppErrorEnum| with any warnings and errors that would occur if
    // the debugger were to attempt to bind a breakpoint at |pos|.
    int GetBindErrors(DocumentPosition pos,
                      out IEnumDebugErrorBreakpoints2 ppErrorEnum);

    // Returns the addresses where the debugger could break at or near |pos|.
    int GetBindAddresses(DocumentPosition pos,
                         out IEnumerable<ulong> addresses,
                         out IEnumDebugErrorBreakpoints2 ppErrorEnum);
  }
}
