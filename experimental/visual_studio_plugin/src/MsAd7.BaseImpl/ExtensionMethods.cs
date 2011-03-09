using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  public static class ExtensionMethods
  {
    public static bool HasFlag(this enum_THREADPROPERTY_FIELDS a, enum_THREADPROPERTY_FIELDS b) {
      return ((uint) a & (uint) b) != 0;
    }

    public static bool HasFlag(this enum_DEBUGPROP_INFO_FLAGS a, enum_DEBUGPROP_INFO_FLAGS b)
    {
      return ((uint)a & (uint)b) != 0;
    }

    public static bool Equals(this TEXT_POSITION a, TEXT_POSITION b) {
      return (a.dwLine == b.dwLine) && (a.dwColumn == b.dwColumn);
    }
  }
}
