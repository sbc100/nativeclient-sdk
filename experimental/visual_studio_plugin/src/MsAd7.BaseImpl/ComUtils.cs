using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Google.MsAd7.BaseImpl
{
  public class ComUtils
  {
    public static Guid GuidOf(Type t) {
      Guid result = Guid.Empty;

      object[] attrs = t.GetCustomAttributes(typeof (InheritGuidAttribute),
                                             false);
      if(attrs.Length > 0) {
        result = GuidOf(((InheritGuidAttribute) attrs[0]).InheritFrom);
      }

      attrs = t.GetCustomAttributes(typeof (GuidAttribute),false);
      if (attrs.Length > 0) {
        result = new Guid(((GuidAttribute) attrs[0]).Value);
      } 
      return result;
    }

    public static Guid GuidOf(object o) {
      return GuidOf(o.GetType());
    }

    public static void RequireOk(int hr) {
      if (hr != 0) {
        throw new COMException("Error", hr);
      }
    }
  }

  public class InheritGuidAttribute : Attribute {
    public InheritGuidAttribute(Type inheritFrom) {
      InheritFrom = inheritFrom;
    }

    public Type InheritFrom { get; set; }
  }


}
