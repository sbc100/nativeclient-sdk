using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
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
    // the variable |nexeList_| is an ArrayList of strings.
    // Whenever the |DebugLaunch| method is invoked, it adds the current nexe
    // that is being launched (though sel_ldr) to the end of the list.
    private static ArrayList nexeList_ = new ArrayList();

    public static string GetLastNexe() {
      // Current assumption -- nexeList_ contains only 1 nexe, but just in
      // case we will grab the last one added to nexeList_.
      // Print an error if nexeList_ contanis more than 1.
      if (nexeList_.Count != 1) {
        Debug.WriteLine("WARNING: nexeList_ Count is " + 
                        NaClProjectConfig.nexeList_.Count);
        foreach (string a_nexe in nexeList_)
        {
          Debug.WriteLine("NEXE: " + a_nexe);
        }
      }
      if (nexeList_.Count == 0) {
        Debug.WriteLine("ERROR: nexeList_.Count is 0");
        return "";
      }
      return (string) nexeList_[nexeList_.Count - 1];
    }

    // NaClProjectConfig launches the server, which continues to run after the
    // method that launched it, DebugLaunch, has finished.  Later, when the
    // user is done debugging, need to be able to terminate it by calling
    // a method.
    private static Process serverProcess_ = null;

    /// <summary>
    /// Create the NaCl Project config, which sets default properties.
    /// </summary>
    /// <param name="project"> Project that is created. </param>
    /// <param name="configuration"> Configuration string, 
    /// such as "Debug" or "Release". </param>
    public NaClProjectConfig(ProjectNode project, string configuration) : base(project, configuration) {
      // Currently this is hardcoded, since we only support 64-bit debugging.
      Debug.WriteLine("In NaClProjectConfig");
      SetConfigurationProperty("TargetArch", "x86_64");
    }

    /// <summary>
    /// Determine if the server is running by connecting to the
    /// specified |hostName| and |portNum|.
    /// </summary>
    /// <param name="hostName"> Name of the host, such as "localhost". </param>
    /// <param name="portNum"> Port number, such as 5103. </param>
    /// <returns> true if we could connect, false otherwise. </returns>
    public static bool IsServerRunning(string hostName, int portNum) {
      bool didConnect = false;
      try {
        var socket = new TcpClient(hostName, portNum);
        didConnect = true;
        Debug.WriteLine("Connected to " + hostName + ":" + portNum);
        socket.Close();
      } catch (Exception e) {
        Debug.WriteLine("Exception {" + e.Message + "} connecting to " +
          hostName + ":" + portNum);
      }
      return didConnect;
    }

    /// <summary>
    /// StartServer will launch a process, which is assumed to be a server
    /// such as httpd.cmd.
    /// </summary>
    /// <param name="serverName"> Name of the executable (full path).</param>
    /// <param name="serverArgs"> Name of the args to the server, can be empty
    /// <param name="workingDir"> Directory where the .nexe, .html, .js, and
    ///   .nmf files are.</param>
    /// string.</param>
    /// TODO: We do not have method to kill or halt the server. I
    /// looked into this, but it's non-trivial to kill the entire process
    /// tree and also depends on the server itself.  For example, httpd.cmd
    /// has had issues with closing when you tell it to.
    public static void StartServer(string serverName, string serverArgs,
                                   string workingDir) {      
      serverProcess_ = new Process();
      serverProcess_.StartInfo.FileName = serverName;      
      serverProcess_.StartInfo.WorkingDirectory = workingDir;
      serverProcess_.StartInfo.Arguments = serverArgs;

      // try/catch will handle the exceptions that can be thrown in C# if the
      // process fails to start for anyh reason (bad path, etc.).
      try {
        serverProcess_.Start();
      } catch (Exception e) {
        Debug.WriteLine("Caught an exception " + e);
        MessageBox.Show(
            "Tried to launch server process {" + serverName + "} with arguments {" +
            serverArgs + "} but encountered exception:\n " + e);
      }
      Debug.WriteLine("Name is " + serverProcess_.ProcessName);
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
      nexeList_.Add(nexe);

      if (host.Contains("sel_ldr")) {
        // sel_ldr needs a -g to enable debugger
        info.bstrArg = string.Format(
            "-g {0} {1}",
            safeNexeString,
            GetConfigurationProperty("DebugArgs", false));
      } else if (host.Contains("chrome.exe")) {
        // chrome needs --enable-nacl-debug --no-sandbox to enable debugger

        string hostName = GetConfigurationProperty("LaunchHostname", false);
        int portNum =
            Convert.ToInt32(GetConfigurationProperty("LaunchPort", false));
        string html_page = hostName + ":" + portNum + "/" + 
          GetConfigurationProperty("HtmlPage", false); 
        bool isServerRunning = IsServerRunning(hostName, portNum);

        string serverProgram = GetConfigurationProperty("WebServer", false);
        string serverArgs = GetConfigurationProperty("WebServerArgs", false);
        if (!isServerRunning && serverProgram.Length > 0) {
          // We know that the server at the host is specified and 
          // is not currently running.  We will start it now.
          // Currently, we are not having the user specify args
          // for the server, but if we need to later that would be the
          // second argument to StartServer.         
          string workingDir = Path.GetDirectoryName(nexe);
          StartServer(serverProgram, serverArgs, workingDir);
        }
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
            " --enable-nacl-debug --no-sandbox --log-level=3" +
            " --incognito";           
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
