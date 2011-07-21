// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Collections.Generic;

#endregion

namespace Google.NaClVsx.DebugSupport.DWARF {
  public static class ExtensionMethods {
    public static T PeekOrDefault<T>(this Stack<T> s) where T : class {
      if (s.Count == 0) {
        return null;
      }
      return s.Peek();
    }

    public static T PeekOrDefault<T>(this Stack<T> s, T defaultValue) {
      if (s.Count == 0) {
        return defaultValue;
      }
      return s.Peek();
    }

    public static TV GetValueOrDefault<TK, TV>(this IDictionary<TK, TV> d,
                                               TK key,
                                               TV defaultValue) {
      TV result;
      if (!d.TryGetValue(key, out result)) {
        result = defaultValue;
      }
      return result;
    }
  }
}
