using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using Google.MsAd7.BaseImpl;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.NaClVsx.DebugSupport
{
  [Guid(kClsId)]
  [ComVisible(true)]
  [ClassInterface(ClassInterfaceType.None)]
  sealed class NaClPortSupplier : PortSupplier
  {
    public const string kClsId =
        "19F7B834-0503-42cd-AEB7-93B69071FA05";


    #region Overrides of PortSupplier

    public NaClPortSupplier()
      : base("Native Client (NaCl)",
             "Connects to Native Client executables (.nexe) via the gdb remote serial protocol.") {
      IDebugPort2 p;
      AddPort(new PortRequest("127.0.0.1:4014"), out p);
    }

    protected override Port CreatePort(IDebugPortRequest2 rq) {
      string portname;
      rq.GetPortName(out portname);
      return new NaClPort(this, rq, portname);
    }

    #endregion
  }
}
