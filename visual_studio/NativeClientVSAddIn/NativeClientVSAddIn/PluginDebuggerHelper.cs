// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;
  using System.Collections.Generic;
  using System.IO;
  using System.Linq;
  using System.Text;
  using System.Windows.Forms;

  using EnvDTE;
  using EnvDTE80;
  using Microsoft.VisualStudio.VCProjectEngine;

  /// <summary>
  /// This class contains functions and utilities which are run when the user
  /// presses F5 or otherwise starts debugging from within Visual Studio.
  /// </summary>
  public class PluginDebuggerHelper
  {
    /// <summary>
    /// This is the initial number of milliseconds to wait between
    /// checking for plug-in processes to attach the debugger to.
    /// </summary>
    private const int InitialPluginCheckFrequency = 1000;

    /// <summary>
    /// After a plug-in has been found, we slow the frequency of checking
    /// for new ones. This value is in milliseconds.
    /// </summary>
    private const int RelaxedPluginCheckFrequency = 5000;

    /// <summary>
    /// The web server port to default to if the user does not specify one.
    /// </summary>
    private const int DefaultWebServerPort = 5103;

    /// <summary>
    /// The main visual studio object through which all Visual Studio functions are executed.
    /// </summary>
    private DTE2 dte_;

    /// <summary>
    /// Indicates the PluginDebuggerHelper is configured properly to run.
    /// </summary>
    private bool isProperlyInitialized_ = false;

    /// <summary>
    /// Directory of the plug-in project we are debugging.
    /// </summary>
    private string pluginProjectDirectory_;

    /// <summary>
    /// Directory where the plug-in assembly is placed.
    /// </summary>
    private string pluginOutputDirectory_;

    /// <summary>
    /// Path to the actual plug-in assembly.
    /// </summary>
    private string pluginAssembly_;

    /// <summary>
    /// Path to the NaCl IRT.
    /// </summary>
    private string irtPath_;

    /// <summary>
    /// Path to the project's nmf file.
    /// </summary>
    private string manifestPath_;

    /// <summary>
    /// Root directory of the installed NaCl SDK.
    /// </summary>
    private string sdkRootDirectory_;

    /// <summary>
    /// If debugging a .nexe this is the nacl-gdb process object.
    /// </summary>
    private System.Diagnostics.Process gdbProcess_;

    /// <summary>
    /// Path to NaCl-GDB executable.
    /// </summary>
    private string gdbPath_;

    /// <summary>
    /// Path to the gdb initialization file that we auto-generate from the VS project.
    /// </summary>
    private string gdbInitFileName_;

    /// <summary>
    /// The platform that the start-up project is currently configured with (NaCl or PPAPI).
    /// </summary>
    private ProjectPlatformType projectPlatformType_;

    /// <summary>
    /// When debugging is started this is the web server process object.
    /// </summary>
    private System.Diagnostics.Process webServer_;

    /// <summary>
    /// Visual Studio output window pane that captures output from the web server.
    /// </summary>
    private OutputWindowPane webServerOutputPane_;

    /// <summary>
    /// Path to the web server executable.
    /// </summary>
    private string webServerExecutable_;

    /// <summary>
    /// Arguments to be passed to the web server executable to start it.
    /// </summary>
    private string webServerArguments_;

    /// <summary>
    /// Timer object that periodically calls a function to look for the plug-in process to debug.
    /// </summary>
    private Timer pluginFinderTimer_;

    /// <summary>
    /// List of process IDs which we should not attempt to attach the debugger to. Mainly this
    /// list contains process IDs of processes we have already attached to.
    /// </summary>
    private List<uint> pluginFinderForbiddenPids_;

    /// <summary>
    /// Process searcher class which allows us to query the system for running processes.
    /// </summary>
    private ProcessSearcher processSearcher_;

    /// <summary>
    /// The main process of chrome that was started by Visual Studio during debugging.
    /// </summary>
    private System.Diagnostics.Process debuggedChromeMainProcess_;

    /// <summary>
    /// Constructs the PluginDebuggerHelper.
    /// Object is not usable until LoadProjectSettings() is called.
    /// </summary>
    /// <param name="dte">Automation object from Visual Studio.</param>
    public PluginDebuggerHelper(DTE2 dte)
    {
      if (dte == null)
      {
        throw new ArgumentNullException("dte");
      }
      
      dte_ = dte;

      // Every second, check for a new instance of the plug-in to attach to.
      // Note that although the timer itself runs on a separate thread, the event
      // is fired from the main UI thread during message processing, thus we do not
      // need to worry about threading issues.
      pluginFinderTimer_ = new Timer();
      pluginFinderTimer_.Tick += new EventHandler(FindAndAttachToPlugin);
      pluginFinderForbiddenPids_ = new List<uint>();
      processSearcher_ = new ProcessSearcher();
    }

    /// <summary>
    /// An event indicating a target plug-in was found on the system.
    /// </summary>
    public event EventHandler<PluginFoundEventArgs> PluginFoundEvent;

    /// <summary>
    /// Specifies the type of plug-in being run in this debug session.
    /// </summary>
    private enum ProjectPlatformType
    {
      /// <summary>
      /// Represents all non-pepper/non-nacl platform types.
      /// </summary>
      Other,

      /// <summary>
      /// Indicates project platform is a trusted plug-in (nexe).
      /// </summary>
      NaCl,

      /// <summary>
      /// Indicates project platform is an untrusted plug-in.
      /// </summary>
      Pepper
    }

    /// <summary>
    /// Initializes the PluginDebuggerHelper with the current project settings
    /// If project settings are unsupported for NaCl/Pepper debugging then
    /// the object is not initialized and we return false.
    /// </summary>
    /// <returns>True if the object is successfully initialized, false otherwise.</returns>
    public bool LoadProjectSettings()
    {
      isProperlyInitialized_ = false;

      string platformToolset;

      // We require that there is only a single start-up project.
      // If multiple start-up projects are specified then we use the first and
      // leave a warning message in the Web Server output pane.
      Array startupProjects = dte_.Solution.SolutionBuild.StartupProjects as Array;
      if (startupProjects == null || startupProjects.Length == 0)
      {
        throw new ArgumentOutOfRangeException("startupProjects.Length");
      }
      else if (startupProjects.Length > 1)
      {
        WebServerWriteLine(Strings.WebServerMultiStartProjectWarning);
      }

      // Get the first start-up project object.
      List<Project> projList = dte_.Solution.Projects.OfType<Project>().ToList();
      string startProjectName = startupProjects.GetValue(0) as string;
      Project startProject = projList.Find(proj => proj.UniqueName == startProjectName);

      // Get the current platform type. If not nacl/pepper then fail.
      string activePlatform = startProject.ConfigurationManager.ActiveConfiguration.PlatformName;
      if (string.Compare(activePlatform, Strings.PepperPlatformName, true) == 0)
      {
        projectPlatformType_ = ProjectPlatformType.Pepper;
        PluginFoundEvent += new EventHandler<PluginFoundEventArgs>(AttachVSDebugger);
      }
      else if (string.Compare(activePlatform, Strings.NaClPlatformName, true) == 0)
      {
        projectPlatformType_ = ProjectPlatformType.NaCl;
        PluginFoundEvent += new EventHandler<PluginFoundEventArgs>(AttachNaClGDB);
      }
      else
      {
        projectPlatformType_ = ProjectPlatformType.Other;
        return false;
      }

      // We only support certain project types (e.g. C/C++ projects). Otherwise we fail.
      if (!Utility.IsVisualCProject(startProject))
      {
        return false;
      }

      // Extract necessary information from specific project type.
      VCConfiguration config = Utility.GetActiveVCConfiguration(startProject);
      IVCRulePropertyStorage general = config.Rules.Item("ConfigurationGeneral");
      VCLinkerTool linker = config.Tools.Item("VCLinkerTool");
      VCProject vcproj = (VCProject)startProject.Object;
        
      sdkRootDirectory_ = general.GetEvaluatedPropertyValue("VSNaClSDKRoot");
      platformToolset = general.GetEvaluatedPropertyValue("PlatformToolset");
      pluginOutputDirectory_ = config.Evaluate(config.OutputDirectory);
      pluginAssembly_ = config.Evaluate(linker.OutputFile);
      pluginProjectDirectory_ = vcproj.ProjectDirectory;  // Macros not allowed here.
      
      if (projectPlatformType_ == ProjectPlatformType.NaCl)
      {
        irtPath_ = general.GetEvaluatedPropertyValue("NaClIrtPath");
        manifestPath_ = general.GetEvaluatedPropertyValue("NaClManifestPath");
      }

      if (string.IsNullOrEmpty(sdkRootDirectory_))
      {
        MessageBox.Show(Strings.SDKPathNotSetError);
        return false;
      }

      sdkRootDirectory_ = sdkRootDirectory_.TrimEnd("/\\".ToArray<char>());

      // TODO(tysand): Move this code getting port to where the web server is started.
      int webServerPort;
      if (!int.TryParse(general.GetEvaluatedPropertyValue("NaClWebServerPort"), out webServerPort))
      {
        webServerPort = DefaultWebServerPort;
      }

      webServerExecutable_ = "python.exe";
      webServerArguments_ = string.Format(
          "{0}\\examples\\httpd.py --no_dir_check {1}", sdkRootDirectory_, webServerPort);

      gdbPath_ = Path.Combine(
          sdkRootDirectory_, "toolchain", platformToolset, @"bin\x86_64-nacl-gdb.exe");

      debuggedChromeMainProcess_ = null;

      isProperlyInitialized_ = true;
      return true;
    }

    /// <summary>
    /// This function should be called to start the PluginDebuggerHelper functionality.
    /// </summary>
    public void StartDebugging()
    {
      if (!isProperlyInitialized_)
      {
        throw new Exception(Strings.NotInitializedMessage);
      }

      StartWebServer();
      pluginFinderTimer_.Interval = InitialPluginCheckFrequency;
      pluginFinderTimer_.Start();
    }

    /// <summary>
    /// This function should be called to stop the PluginDebuggerHelper functionality.
    /// </summary>
    public void StopDebugging()
    {
      isProperlyInitialized_ = false;
      pluginFinderTimer_.Stop();
      pluginFinderForbiddenPids_.Clear();

      // Remove all event handlers from the plug-in found event.
      if (PluginFoundEvent != null)
      {
        foreach (Delegate del in PluginFoundEvent.GetInvocationList())
        {
          PluginFoundEvent -= (EventHandler<PluginFoundEventArgs>)del;
        }
      }

      Utility.EnsureProcessKill(ref webServer_);
      WebServerWriteLine(Strings.WebServerStopMessage);
      CleanUpGDBProcess();
    }

    /// <summary>
    /// This function cleans up the started GDB process.
    /// </summary>
    private void CleanUpGDBProcess()
    {
      Utility.EnsureProcessKill(ref gdbProcess_);
      if (!string.IsNullOrEmpty(gdbInitFileName_) && File.Exists(gdbInitFileName_))
      {
        File.Delete(gdbInitFileName_);
        gdbInitFileName_ = null;
      }
    }

    /// <summary>
    /// This is called periodically by the Visual Studio UI thread to look for our plug-in process
    /// and attach the debugger to it.  The call is triggered by the pluginFinderTimer_ object.
    /// </summary>
    /// <param name="unused">The parameter is not used.</param>
    /// <param name="unused1">The parameter is not used.</param>
    private void FindAndAttachToPlugin(object unused, EventArgs unused1)
    {
      StringComparison ignoreCase = StringComparison.InvariantCultureIgnoreCase;

      // Set the main chrome process that was started by visual studio.  If it's not chrome
      // or not found then we have no business attaching to any plug-ins so return.
      if (debuggedChromeMainProcess_ == null)
      {
        foreach (Process proc in dte_.Debugger.DebuggedProcesses)
        {
          if (proc.Name.EndsWith(Strings.ChromeProcessName, ignoreCase))
          {
            debuggedChromeMainProcess_ = System.Diagnostics.Process.GetProcessById(proc.ProcessID);
            break;
          }
        }

        return;
      }

      // Get the list of all descendants of the main chrome process.
      uint mainChromeProcId = (uint)debuggedChromeMainProcess_.Id;
      List<ProcessInfo> chromeDescendants = processSearcher_.GetDescendants(mainChromeProcId);

      // If we didn't start with debug flags then we should not attach.
      string mainChromeFlags = chromeDescendants.Find(p => p.ID == mainChromeProcId).CommandLine;
      if (projectPlatformType_ == ProjectPlatformType.NaCl &&
          !mainChromeFlags.Contains(Strings.NaClDebugFlag))
      {
        return;
      }

      // From the list of descendants, find the plug-in by it's command line arguments and
      // process name as well as not being attached to already.
      List<ProcessInfo> plugins;
      switch (projectPlatformType_)
      {
        case ProjectPlatformType.Pepper:
          string identifierFlagTarget =
              string.Format(Strings.PepperProcessPluginFlagFormat, pluginAssembly_);
          plugins = chromeDescendants.FindAll(p =>
             p.Name.Equals(Strings.ChromeProcessName, ignoreCase) &&
             p.CommandLine.Contains(Strings.ChromeRendererFlag, ignoreCase) &&
             p.CommandLine.Contains(identifierFlagTarget, ignoreCase) &&
             !pluginFinderForbiddenPids_.Contains(p.ID));
          break;
        case ProjectPlatformType.NaCl:
          plugins = chromeDescendants.FindAll(p =>
             p.Name.Equals(Strings.NaClProcessName, ignoreCase) &&
             p.CommandLine.Contains(Strings.NaClLoaderFlag, ignoreCase) &&
             !pluginFinderForbiddenPids_.Contains(p.ID));
          break;
        default:
          return;
      }

      // Attach to all plug-ins that we found.
      foreach (ProcessInfo process in plugins)
      {
        // If we are attaching to a plug-in, add it to the forbidden list to ensure we
        // don't try to attach again later.
        pluginFinderForbiddenPids_.Add(process.ID);
        PluginFoundEvent.Invoke(this, new PluginFoundEventArgs(process.ID));

        // Slow down the frequency of checks for new plugins.
        pluginFinderTimer_.Interval = RelaxedPluginCheckFrequency;
      }
    }

    /// <summary>
    /// Attaches the visual studio debugger to a given process ID.
    /// </summary>
    /// <param name="src">The parameter is not used.</param>
    /// <param name="args">Contains the process ID to attach to.</param>
    private void AttachVSDebugger(object src, PluginFoundEventArgs args)
    {
      foreach (EnvDTE.Process proc in dte_.Debugger.LocalProcesses)
      {
        if (proc.ProcessID == args.ProcessID)
        {
          proc.Attach();
          break;
        }
      }
    }

    /// <summary>
    /// Attaches the NaCl GDB debugger to the NaCl plug-in process.  Handles loading symbols
    /// and breakpoints from Visual Studio.
    /// </summary>
    /// <param name="src">The parameter is not used.</param>
    /// <param name="args">
    /// Contains the process ID to attach to, unused since debug stub is already attached.
    /// </param>
    private void AttachNaClGDB(object src, PluginFoundEventArgs args)
    {
      // Clean up any pre-existing GDB process (can happen if user reloads page).
      CleanUpGDBProcess();

      gdbInitFileName_ = Path.GetTempFileName();
      string pluginAssemblyEscaped = pluginAssembly_.Replace("\\", "\\\\");
      string irtPathEscaped = irtPath_.Replace("\\", "\\\\");

      // Create the initialization file to read in on GDB start.
      StringBuilder contents = new StringBuilder();

      if (!string.IsNullOrEmpty(manifestPath_))
      {
        string manifestEscaped = manifestPath_.Replace("\\", "\\\\");
        contents.AppendFormat("nacl-manifest {0}\n", manifestEscaped);
      }
      else
      {
        contents.AppendFormat("file \"{0}\"\n", pluginAssemblyEscaped);
      }

      contents.AppendFormat("nacl-irt {0}\n", irtPathEscaped);
      contents.AppendFormat("target remote localhost:{0}\n", 4014);

      // Insert breakpoints from Visual Studio project.
      foreach (Breakpoint bp in dte_.Debugger.Breakpoints)
      {
        if (!bp.Enabled)
        {
          continue;
        }

        if (bp.LocationType == dbgBreakpointLocationType.dbgBreakpointLocationTypeFile)
        {
          contents.AppendFormat("b {0}:{1}\n", Path.GetFileName(bp.File), bp.FileLine);
        }
        else if (bp.LocationType == dbgBreakpointLocationType.dbgBreakpointLocationTypeFunction)
        {
          contents.AppendFormat("b {0}\n", bp.FunctionName);
        }
        else
        {
          WebServerWriteLine(
            string.Format(Strings.UnsupportedBreakpointTypeFormat, bp.LocationType.ToString()));
        }
      }

      contents.AppendLine("continue");
      File.WriteAllText(gdbInitFileName_, contents.ToString());

      // Start NaCl-GDB.
      try
      {
        gdbProcess_ = new System.Diagnostics.Process();
        gdbProcess_.StartInfo.UseShellExecute = true;
        gdbProcess_.StartInfo.FileName = gdbPath_;
        gdbProcess_.StartInfo.Arguments = string.Format("-x {0}", gdbInitFileName_);
        gdbProcess_.StartInfo.WorkingDirectory = pluginProjectDirectory_;
        gdbProcess_.Start();
      }
      catch (Exception e)
      {
        MessageBox.Show(
            string.Format("NaCl-GDB Start Failed. {0}. Path: {1}", e.Message, gdbPath_));
      }
    }

    /// <summary>
    /// Spins up the web server process to host our plug-in.
    /// </summary>
    private void StartWebServer()
    {
      // Add a panel to the output window which is used to capture output
      // from the web server hosting the plugin.
      if (webServerOutputPane_ == null)
      {
        webServerOutputPane_ = dte_.ToolWindows.OutputWindow.OutputWindowPanes.Add(
            Strings.WebServerOutputWindowTitle);
      }

      webServerOutputPane_.Clear();
      WebServerWriteLine(Strings.WebServerStartMessage);

      try
      {
        webServer_ = new System.Diagnostics.Process();
        webServer_.StartInfo.CreateNoWindow = true;
        webServer_.StartInfo.UseShellExecute = false;
        webServer_.StartInfo.RedirectStandardOutput = true;
        webServer_.StartInfo.RedirectStandardError = true;
        webServer_.StartInfo.FileName = webServerExecutable_;
        webServer_.StartInfo.Arguments = webServerArguments_;
        webServer_.StartInfo.WorkingDirectory = pluginProjectDirectory_;
        webServer_.OutputDataReceived += WebServerMessageReceive;
        webServer_.ErrorDataReceived += WebServerMessageReceive;
        webServer_.Start();
        webServer_.BeginOutputReadLine();
        webServer_.BeginErrorReadLine();
      }
      catch (Exception e)
      {
        WebServerWriteLine(Strings.WebServerStartFail);
        WebServerWriteLine("Exception: " + e.Message);
      }
    }

    /// <summary>
    /// Receives output from the web server process to display in the Visual Studio UI.
    /// </summary>
    /// <param name="sender">The parameter is not used.</param>
    /// <param name="e">Contains the data to display.</param>
    private void WebServerMessageReceive(object sender, System.Diagnostics.DataReceivedEventArgs e)
    {
      WebServerWriteLine(e.Data);
    }

    /// <summary>
    /// Helper function to write data to the Web Server Output Pane.
    /// </summary>
    /// <param name="message">Message to write.</param>
    private void WebServerWriteLine(string message)
    {
      if (webServerOutputPane_ != null)
      {
        webServerOutputPane_.OutputString(message + "\n");
      }
    }

    /// <summary>
    /// The event arguments when a plug-in is found.
    /// </summary>
    public class PluginFoundEventArgs : EventArgs
    {
      /// <summary>
      /// Construct the PluginFoundEventArgs.
      /// </summary>
      /// <param name="pid">Process ID of the found plug-in.</param>
      public PluginFoundEventArgs(uint pid)
      {
        this.ProcessID = pid;
      }

      /// <summary>
      /// Gets or sets process ID of the found plug-in.
      /// </summary>
      public uint ProcessID { get; set; }
    }
  }
}
