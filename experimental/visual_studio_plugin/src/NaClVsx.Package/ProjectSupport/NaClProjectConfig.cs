using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
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
    // Although the NaClProjectConfig class is not implemented as a singleton,
    // it is what launches the debugger with a given nexe, and therefore there
    // should (at least in the short term) be a current Nexe that we are
    // debugging.  In order to provide the most recent Nexe launched and still
    // have the flexibility (in the future) to have multiple nexes, as well as
    // to keep track of all the nexes we have tried to debug in a session, 
    // the variable |NexeList| is an ArrayList of strings.
    // Whenever the |DebugLaunch| method is invoked, it adds the current nexe
    // that is being launched (though sel_ldr) to the end of the list.
    private static ArrayList NexeList = new ArrayList();
    public static string GetLastNexe() {
      // Current assumption -- NexeList contains only 1 nexe, but just in
      // case we will grab the last one added to NexeList.
      // Print an error if NexeList contanis more than 1.
      if (NexeList.Count != 1) {
        Debug.WriteLine("WARNING: NexeList Count is " + 
                        NaClProjectConfig.NexeList.Count);
        foreach (string a_nexe in NexeList)
        {
          Debug.WriteLine("NEXE: " + a_nexe);
        }
      }
      if (NexeList.Count == 0) {
        Debug.WriteLine("ERROR: NexeList.Count is 0");
        return "";
      }
      return (string) NexeList[NexeList.Count - 1];
    }

    public NaClProjectConfig(ProjectNode project, string configuration) : base(project, configuration) {
      // Currently this is hardcoded, since we only support 64-bit debugging.
      SetConfigurationProperty("TargetArch", "x86_64");
      SetConfigurationProperty("NaClSDKRoot", "$(NACL_SDK_ROOT)");
    }

    /// <summary>
    /// DebugLaunch populates the VsDebugTargetInfo object, and then hands it
    /// to VsShellUtilities to launch the debugger.
    /// </summary>
    /// <param name="grfLaunch">
    /// An enumeration of launch parameters, interpreted as a
    /// __VSDBGLAUNCHFLAGS Enumeration.
    /// </param>
    /// <returns>
    /// VSConstants.S_OK if all goes well.  Otherwise VSConstants.E_UNEXPECTED
    /// </returns>
    public override int DebugLaunch(uint grfLaunch) {
      VsDebugTargetInfo info = new VsDebugTargetInfo();
      info.clsidCustom = new Guid(Engine.kId);
      info.dlo = DEBUG_LAUNCH_OPERATION.DLO_CreateProcess;

      string host = ProjectMgr.GetProjectProperty("DebugHost");
      if (string.IsNullOrEmpty(host)
       || !File.Exists(host)) {
        throw new FileNotFoundException(
            "The Debug Host property has not been specified. " +
            "Debugging cannot continue.\n\n" +
            "To specify a debug host, open the project properties and " +
            "select Debug from the list on the left of the window.");
      }
      info.bstrExe = host;

      if (null == ProjectMgr) {
        return VSConstants.E_UNEXPECTED;
      }

      string nexe =
          Path.Combine(
              Path.GetDirectoryName(ProjectMgr.BaseURI.Uri.LocalPath),
              GetConfigurationProperty("OutputFullPath", false));

      string safeNexeString = string.Format("\"{0}\"", nexe);
      NexeList.Add(nexe);

      if (host.Contains("sel_ldr")) {
        // sel_ldr needs a -g to enable debugger
        info.bstrArg = string.Format(
            "-g {0} {1}",
            safeNexeString,
            GetConfigurationProperty("DebugArgs", false));
      } else if (host.Contains("chrome.exe")) {
        // chrome needs --enable-nacl-debug --no-sandbox to enable debugger
        // FIXME:  Instead of the nexe, we will need to call chrome with
        // an argument that is the web page:  i.e. localhost:5013
        // TODO(mmortensen) Rather than hard-code html_page, we need this to
        // be set in the project, either as a property or by creating an html
        // file in the project.
        string html_page = "localhost:5103";
        // TODO(mmortensen) Determine if all these flags are needed.  In the
        // short term, the more important thing is to get the VSX/chrome 
        // interaction to be more stable.
        string chrome_debug_args = "--enable-file-cookies --dom-automation" +
            " --disable-web-resources --disable-preconnect" +
            " --no-first-run --no-default-browser-check --enable-logging" +
            " --safebrowsing-disable-auto-update" +
            " --no-default-browser-check " +
            " --noerrdialogs --metrics-recording-only --enable-logging" +
            " --allow-file-access-from-files" +
            " --disable-tab-closeable-state-watcher --allow-file-access" +
            " --unlimited-quota-for-files --enable-nacl" +
            " --enable-nacl-debug --no-sandbox --log-level=3";
        info.bstrArg = string.Format(
            "{0} {1} {2}",
            chrome_debug_args,
            html_page,
            GetConfigurationProperty("DebugArgs", false));
      } else {
        info.bstrArg = string.Format(
            "{0} {1}",
            safeNexeString,
            GetConfigurationProperty("DebugArgs", false));
      }

      info.bstrCurDir = Path.GetDirectoryName(nexe);
      info.fSendStdoutToOutputWindow = 1;
      info.grfLaunch = grfLaunch;
      info.clsidPortSupplier = typeof (NaClPortSupplier).GUID;
      info.bstrPortName = "127.0.0.1:4014";

      // If we need to set env vars, this is a way to do it
      // Environment.SetEnvironmentVariable("NACL_DEBUG_ENABLE","1");

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
