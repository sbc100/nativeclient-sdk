using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.Shell;

namespace Google.NaClVsx.Installation
{
  [AttributeUsage(AttributeTargets.Class, AllowMultiple = true)]
  public sealed class PortSupplierRegistrationAttribute : RegistrationAttribute
  {
    public PortSupplierRegistrationAttribute(Type portSupplierType) {
      portSupplierType_ = portSupplierType;
    }

    public Type PortSupplierType {
      get { return portSupplierType_; }
      set { portSupplierType_ = value; }
    }

    public string Name {
      get { return name_; }
      set { name_ = value; }
    }

    public bool DisallowUserPorts {
      get { return disallowUserPorts_; }
      set { disallowUserPorts_ = value; }
    }

    #region Overrides of RegistrationAttribute

    public override void Register(RegistrationContext context) {
      Key key = context.CreateKey(KeyName());
      key.SetValue("CLSID", ClsIdString());
      key.SetValue("DisallowUserEnteredPorts", disallowUserPorts_ ? 1 : 0);
      key.SetValue("Name", name_);
    }

    public override void Unregister(RegistrationContext context) {
      context.RemoveKey(KeyName());
    }

    #endregion

    string ClsIdString() {
      return PortSupplierType.GUID.ToString("B");
    }

    string KeyName()
    {
      return string.Format("AD7Metrics\\PortSupplier\\{0}", ClsIdString());
    }

    private Type portSupplierType_;
    private string name_ = "[Port provider name goes here]";
    private bool disallowUserPorts_ = false;
  }
}
