using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  public class ExpressionContext : IDebugExpressionContext2
  {
    #region Implementation of IDebugExpressionContext2

    public int GetName(out string pbstrName) {
      Debug.WriteLine("ExpressionContext.GetName");
      throw new NotImplementedException();
    }

    public int ParseText(string pszCode, enum_PARSEFLAGS dwFlags, uint nRadix, out IDebugExpression2 ppExpr, out string pbstrError, out uint pichError) {
      Debug.WriteLine("ExpressionContext.ParseText");
      throw new NotImplementedException();
    }

    #endregion
  }
}
