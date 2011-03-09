using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing.Design;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms.Design;
using Microsoft.VisualStudio.Project;

namespace Google.NaClVsx.ProjectSupport
{
  [ComVisible(true), Guid("370263ED-D4DE-4d28-8323-45FD3C600789")]
  class DebugProperties : AutoSettingsPage
  {
    public DebugProperties() {
      Name = "Debug";
    }

    [Category("Launch")]
    [DisplayName("Hosting Process (Service Runtime)")]
    [ProjectProperty("DebugHost", true)]
    [Editor(typeof(FileNameEditor), typeof(UITypeEditor))]
    public string HostProgram
    {
      get { return hostProgram_; }
      set { hostProgram_ = value;
        IsDirty = true;}
    }

    [Category("Launch")]
    [DisplayName("Arguments")]
    [ProjectProperty("DebugArgs", true)]
    public string Args
    {
      get { return args_; }
      set { args_ = value; IsDirty = true; }
    }

    private string hostProgram_;
    private string args_;
  }
}
