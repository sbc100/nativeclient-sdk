using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.Design;
using Google.NaClVsx.DebugSupport;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Project;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using MSBuild = Microsoft.Build.BuildEngine;

namespace Google.NaClVsx.ProjectSupport
{
  class NaClProjectConfig : ProjectConfig
  {
    public NaClProjectConfig(ProjectNode project, string configuration) : base(project, configuration) {
      // Currently this is hardcoded, since we only support 64-bit debugging.
      SetConfigurationProperty("TargetArch", "x86_64");
      SetConfigurationProperty("NaClSDKRoot", "$(NACL_SDK_ROOT)");
    }

    public override int DebugLaunch(uint grfLaunch)
    {
      VsDebugTargetInfo info = new VsDebugTargetInfo();
      info.clsidCustom = new Guid(DebugSupport.Engine.kId);
      info.dlo = DEBUG_LAUNCH_OPERATION.DLO_CreateProcess;

      string host = this.ProjectMgr.GetProjectProperty("DebugHost");
      if (string.IsNullOrEmpty(host)
          || !File.Exists(host))
      {
        throw new FileNotFoundException("The Debug Host property has not been specified. " +
                            "Debugging cannot continue.\n\n" +
                            "To specify a debug host, open the project properties and " +
                            "select Debug from the list on the left of the window.");
      }
      info.bstrExe = host;

      string nexe =
          Path.Combine(
              Path.GetDirectoryName(this.ProjectMgr.BaseURI.Uri.LocalPath),
              GetConfigurationProperty("OutputFullPath", false));

      info.bstrArg = string.Format("{0} {1}",
        nexe,
        GetConfigurationProperty("DebugArgs", false));

      info.bstrCurDir = Path.GetDirectoryName(nexe);

      info.fSendStdoutToOutputWindow = 1;
      info.grfLaunch = grfLaunch;
      info.clsidPortSupplier = typeof (NaClPortSupplier).GUID;
      info.bstrPortName = "127.0.0.1:4014";
      VsShellUtilities.LaunchDebugger(this.ProjectMgr.Site, info);
      return VSConstants.S_OK;
    }

    internal string GetRawConfigurationProperty(string propertyName, bool cacheNeedReset)
    {
        MSBuild.BuildProperty property = GetMsBuildProperty(propertyName, cacheNeedReset);
        if (property == null)
            return null;

        return property.Value;
    }
  }
}
