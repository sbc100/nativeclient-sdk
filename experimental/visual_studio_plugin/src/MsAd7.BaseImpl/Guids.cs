// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
//
// This file contains code licensed under the Microsoft Public License.
// 
using System;

namespace Google.MsAd7.BaseImpl {
  /// <summary>
  /// Well-known GUIDs for ActiveDebugger7, copied from the Microsoft debugger
  /// sample at http://code.msdn.microsoft.com/debugenginesample.
  /// </summary>
  class Guids {
    static public Guid guidFilterRegisters {
      get { return _guidFilterRegisters; }
    }

    static public Guid guidFilterLocals {
      get { return _guidFilterLocals; }
    }

    static public Guid guidFilterAllLocals {
      get { return _guidFilterAllLocals; }
    }

    static public Guid guidFilterArgs {
      get { return _guidFilterArgs; }
    }

    static public Guid guidFilterLocalsPlusArgs {
      get { return _guidFilterLocalsPlusArgs; }
    }

    static public Guid guidFilterAllLocalsPlusArgs {
      get { return _guidFilterAllLocalsPlusArgs; }
    }

    // Language guid for C++. Used when the language for a document context or a stack frame is requested.

    static public Guid guidLanguageCpp {
      get { return _guidLanguageCpp; }
    }

    #region Private Implementation

    static private readonly Guid _guidFilterAllLocals =
        new Guid("196db21f-5f22-45a9-b5a3-32cddb30db06");

    static private readonly Guid _guidFilterAllLocalsPlusArgs =
        new Guid("939729a8-4cb0-4647-9831-7ff465240d5f");

    static private readonly Guid _guidFilterArgs =
        new Guid("804bccea-0475-4ae7-8a46-1862688ab863");

    static private readonly Guid _guidFilterLocals =
        new Guid("b200f725-e725-4c53-b36a-1ec27aef12ef");

    static private readonly Guid _guidFilterLocalsPlusArgs =
        new Guid("e74721bb-10c0-40f5-807f-920d37f95419");

    static private readonly Guid _guidFilterRegisters =
        new Guid("223ae797-bd09-4f28-8241-2763bdc5f713");

    static private readonly Guid _guidLanguageCpp =
        new Guid("3a12d0b7-c26c-11d0-b442-00a0244a1dd2");

    #endregion
  }
}

