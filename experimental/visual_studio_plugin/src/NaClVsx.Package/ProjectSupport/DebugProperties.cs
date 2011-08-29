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
    [DisplayName("Hosting Process (chrome or sel_ldr)")]
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

    [Category("Launch")]
    [DisplayName("IRT (for sel_ldr)")]
    [ProjectProperty("IrtNexe", true)]
    public string IrtNexe
    {
      get { return irtNexe_; }
      set { irtNexe_ = value; IsDirty = true; }
    }


    [Category("ChromeUrl")]
    [DisplayName("Html for web application")]
    [ProjectProperty("HtmlPage", true)]
    public string HtmlPage
    {
      get { return htmlPage_; }
      set { htmlPage_ = value; IsDirty = true; }
    }
    [Category("ChromeUrl")]
    [DisplayName("Web server hostname")]
    [ProjectProperty("LaunchHostname", true)]
    public string LaunchHost
    {
      get { return launchHost_; }
      set { launchHost_ = value; IsDirty = true; }
    }
    [Category("ChromeUrl")]
    [DisplayName("Web server port")]
    [ProjectProperty("LaunchPort", true)]
    public string LaunchPort
    {
      get { return launchPort_; }
      set { launchPort_ = value; IsDirty = true; }
    }

    [Category("WebServer")]
    [DisplayName("Httpd Server")]
    [ProjectProperty("WebServer", true)]
    public string WebServerProgram
    {
      get { return webServerProgram_; }
      set { webServerProgram_ = value; IsDirty = true; }
    }

    [Category("WebServer")]
    [DisplayName("Httpd Server Arguments")]
    [ProjectProperty("WebServerArgs", true)]
    public string WebServerArgs
    {
      get { return webServerArgs_; }
      set { webServerArgs_ = value; IsDirty = true; }
    }

    private string hostProgram_;
    private string args_;
    private string htmlPage_;
    private string irtNexe_;
    private string launchHost_;
    private string launchPort_;
    private string webServerProgram_;
    private string webServerArgs_;
  }
}
