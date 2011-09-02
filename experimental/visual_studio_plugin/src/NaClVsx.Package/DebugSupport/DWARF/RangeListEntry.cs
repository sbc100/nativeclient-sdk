// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NaClVsx;

namespace Google.NaClVsx.DebugSupport.DWARF
{
  /// <summary>
  /// Represents a Range List entry.
  /// </summary>
  public class RangeListEntry {
    public ulong Offset;
    public ulong BaseAddress;
    public ulong LowPC;
    public ulong HighPC;
  }
}
