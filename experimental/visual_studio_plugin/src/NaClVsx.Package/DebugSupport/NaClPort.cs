using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Text;
using Google.MsAd7.BaseImpl;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.NaClVsx.DebugSupport
{
  public class NaClPort : Port
  {
    public NaClPort(IDebugPortSupplier2 supplier, IDebugPortRequest2 request, string connectionString) 
        : base(supplier, request, System.Guid.NewGuid()) {
      connectionString_ = connectionString;
    }

    public string ConnectionString {
      get { return connectionString_; }
    }

    void Refresh()
    {
      // currently we're assuming that there is only one
      // (IP) port in use, and it's specified in the
      // connection string.
      // TODO(Noel): generalize to work with multiple ports
      // once we have some way of enumerating which processes
      // are listening.
      if (processes_.Count > 0) return;
      string[] addressComponents = connectionString_.Split(':');
      if (addressComponents.Length < 2)
      {
        throw new ArgumentException("Connection string must be of the format <hostname>:<port>");
      }

      int port = Convert.ToInt32(addressComponents[1]);

      var dbg = new NaClDebugger(0);
      dbg.Open(connectionString_);
      string imagePath = dbg.Path;
      dbg.Close();



      processes_.Add(new NaClDebugProcess(this, connectionString_, port, imagePath));
    }

    #region Overrides of Port

    protected override IEnumerable<DebugProcess> GetProcesses() {
      Refresh();
      return processes_;
    }

    protected override DebugProcess CreateProcessInternal(ProcessStartInfo psi) {
      DebugProcess result = null;
      using (var proc = Process.Start(psi)) {
        if (proc == null) {
          throw new InvalidOperationException(
              string.Format("Can't launch {0}", psi.FileName));
        }

        result = new NaClDebugProcess(
            this, connectionString_, proc.Id, psi.FileName);
      }

      processes_.Add(result);
      return result;
    }

    #endregion

    private readonly string connectionString_;
    private readonly List<DebugProcess> processes_ = new List<DebugProcess>();
  }
}
