using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  class SimpleExpression : IDebugExpression2
  {
    public SimpleExpression(DebugPropertyBase property) {
      property_ = property;
    }

    public DebugPropertyBase Property {
      get { return property_; }
    }

    #region Implementation of IDebugExpression2

    public int EvaluateAsync(enum_EVALFLAGS dwFlags, IDebugEventCallback2 pExprCallback) {
      throw new NotImplementedException();
    }

    public int Abort() {
      throw new NotImplementedException();
    }

    public int EvaluateSync(enum_EVALFLAGS dwFlags, uint dwTimeout, IDebugEventCallback2 pExprCallback, out IDebugProperty2 ppResult) {
      ppResult = property_;
      return VSConstants.S_OK;
    }

    #endregion

    private DebugPropertyBase property_;
  }
}
