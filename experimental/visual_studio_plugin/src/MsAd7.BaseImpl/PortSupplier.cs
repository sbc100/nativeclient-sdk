using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  public abstract class PortSupplier : IDebugPortSupplier3, IDebugPortSupplierDescription2
  {
    protected PortSupplier(string name, string description) {
      name_ = name;
      description_ = description;
      Debug.WriteLine("PortSupplier ctor");
    }

    public string Name {
      get { return name_; }
    }

    public string Description {
      get { return description_; }
    }

    #region Implementation of IDebugPortSupplier3

    public int GetPortSupplierName(out string pbstrName) {
      pbstrName = name_;
      return VSConstants.S_OK;
    }

    public int GetPortSupplierId(out Guid pguidPortSupplier) {
      pguidPortSupplier = this.GetType().GUID;
      return VSConstants.S_OK;
    }

    public int GetPort(ref Guid guidPort, out IDebugPort2 ppPort) {
      Debug.WriteLine("PortSupplier.GetPort");
      ppPort = ports_[guidPort];
      return VSConstants.S_OK;
    }

    public int EnumPorts(out IEnumDebugPorts2 ppEnum) {
      Debug.WriteLine("PortSupplier.EnumPorts");

      ppEnum = new Ad7Enumerators.PortEnumerator(ports_.Values);
      return VSConstants.S_OK;
    }

    public int CanAddPort() {
      Debug.WriteLine("PortSupplier.CanAddPort");
      return VSConstants.S_OK;
    }

    public int AddPort(IDebugPortRequest2 pRequest, out IDebugPort2 ppPort) {
      Debug.WriteLine("PortSupplier.AddPort");
      var port = CreatePort(pRequest);
      ports_.Add(port.Guid, port);
      ppPort = port;
      return VSConstants.S_OK;
    }


    public int RemovePort(IDebugPort2 pPort) {
      Debug.WriteLine("PortSupplier.RemovePort");
      throw new NotImplementedException();
    }

    public int CanPersistPorts() {
      return VSConstants.S_OK;
    }

    public int EnumPersistedPorts(BSTR_ARRAY PortNames, out IEnumDebugPorts2 ppEnum) {
      Debug.WriteLine("PortSupplier.EnumPersistedPorts");
      throw new NotImplementedException();
    }

    #endregion

    protected abstract Port CreatePort(IDebugPortRequest2 rq);
    Dictionary<Guid, IDebugPort2> ports_ = new Dictionary<Guid, IDebugPort2>();
    private string name_;
    private string description_;

    #region Implementation of IDebugPortSupplierDescription2

    public int GetDescription(enum_PORT_SUPPLIER_DESCRIPTION_FLAGS[] pdwFlags, out string pbstrText) {
      pbstrText = description_;
      return VSConstants.S_OK;
    }

    #endregion
  }
}
