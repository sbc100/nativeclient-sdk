using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Google.MsAd7.BaseImpl.Interfaces;

namespace Google.NaClVsx.DebugSupport.DWARF
{
  public static class ExtensionMethods
  {
    public static T PeekOrDefault<T>(this Stack<T> s) where T : class{
      if (s.Count == 0) {
        return null;
      }
      return s.Peek();
    }

    public static T PeekOrDefault<T>(this Stack<T> s, T defaultValue)
    {
      if (s.Count == 0)
      {
        return defaultValue;
      }
      return s.Peek();
    }

    public static TV GetValueOrDefault<TK,TV>(this IDictionary<TK,TV> d, TK key, TV defaultValue) {
      TV result;
      if (!d.TryGetValue(key, out result)) {
        result = defaultValue;
      }
      return result;
    }

    public static ulong GetU64(this ISimpleDebugger dbg, ulong address) {
      byte[] data = new byte[8];
      dbg.GetMemory(address, data, 8);
      return BitConverter.ToUInt64(data, 0);
    }

    public static uint GetU32(this ISimpleDebugger dbg, ulong address)
    {
      byte[] data = new byte[4];
      dbg.GetMemory(address, data, 4);
      return BitConverter.ToUInt32(data, 0);
    }

  }
}
