// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;
  using System.IO;
  using System.Text;
  using System.Windows.Forms;
  using System.Diagnostics;

  using EnvDTE;
  using EnvDTE80;

  /// <summary>
  /// This class handles the details of finding a nexe and attaching to it.
  /// </summary>
  public class PluginDebuggerGDB : PluginDebuggerBase
  {
    /// <summary>
    /// Path to the actual plug-in assembly.
    /// </summary>
    private string pluginAssembly_;

    /// <summary>
    /// Directory of the plug-in project we are debugging.
    /// </summary>
    private string pluginProjectDirectory_;

    /// <summary>
    /// Path to the NaCl IRT.
    /// </summary>
    private string irtPath_;

    /// <summary>
    /// Path to the project's nmf file.
    /// </summary>
    private string manifestPath_;

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
    /// Constructs the PluginDebuggerHelper.
    /// </summary>
    /// <param name="dte">Automation object from Visual Studio.</param>
    /// <param name="properties">PropertyManager pointing to a valid project/platform.</param>
    public PluginDebuggerGDB(DTE2 dte, PropertyManager properties)
        : base(dte, properties)
    {
      string arch = "i686";
      if (Environment.Is64BitOperatingSystem)
      {
        arch = "x86_64";
      }

      if (properties.TargetArchitecture != arch)
      {
        MessageBox.Show(string.Format("Debugging of {0} NaCl modules is not possible on this system ({1}).", 
                                      properties.TargetArchitecture, arch));
      }

      // check chrome version
      string chrome_path = properties.LocalDebuggerCommand;
      FileVersionInfo version_info = FileVersionInfo.GetVersionInfo(chrome_path);
      string file_version = version_info.FileVersion;
      if (file_version != null)
      {
        string major_version = file_version.Split('.')[0];
        int major_version_int = 0;
        try
        {
          major_version_int = Convert.ToInt32(major_version);
        }
        catch
        {
        }
        if (major_version_int < 22)
        {
          MessageBox.Show("Chrome 22 or above required for NaCl debugging (your version is "
                          + major_version + ")");
          return;
        }
      }

      // We look for the IRT in several ways, mimicing what chrome itself
      // does in chrome/app/client_util.cc:MakeMainDllLoader.

      // First look for the IRT alongside chrome.exe
      string irt_basename = "nacl_irt_" + arch + ".nexe";
      irtPath_ = Path.Combine(Path.GetDirectoryName(chrome_path), irt_basename);
      if (!File.Exists(irtPath_))
      {
        // Next look for a folder alongside chrome.exe with the same name
        // as the version embedded in chrome.exe.
        if (file_version == null)
        {
          if (!File.Exists(irtPath_))
          {
            MessageBox.Show("NaCl IRT not found in chrome install.\nLooking for: " + irtPath_);
            irtPath_ = null;
          }
        }
        else
        {
          irtPath_ = Path.Combine(Path.GetDirectoryName(chrome_path), 
                                  file_version, irt_basename);
          if (!File.Exists(irtPath_))
          {
            MessageBox.Show("NaCl IRT not found in chrome install.\nLooking for: " + irtPath_);
            irtPath_ = null;
          }
        }
      }

      manifestPath_ = properties.ManifestPath;
      pluginAssembly_ = properties.PluginAssembly;
      pluginProjectDirectory_ = properties.ProjectDirectory;
      gdbPath_ = Path.Combine(
          properties.SDKRootDirectory,
          "toolchain",
          string.Concat("win_x86_", properties.ToolchainName),
          @"bin\x86_64-nacl-gdb.exe");

      PluginFoundEvent += new EventHandler<PluginFoundEventArgs>(Attach);
    }

    /// <summary>
    /// Disposes the object. If disposing is false then this has been called by garbage collection,
    /// and we shouldn't reference managed objects.
    /// </summary>
    /// <param name="disposing">True if user call to Dispose, false if garbage collection.</param>
    protected override void Dispose(bool disposing)
    {
      if (!Disposed)
      {
        base.Dispose(disposing);

        if (disposing)
        {
          CleanUpGDBProcess();
        }

        // This is repeated functionality from CleanUpGDBProcess but will
        // only touch unmanaged resources as required by disposing=false.
        if (!string.IsNullOrEmpty(gdbInitFileName_) && File.Exists(gdbInitFileName_))
        {
          File.Delete(gdbInitFileName_);
          gdbInitFileName_ = null;
        }

        Disposed = true;
      }
    }

    /// <summary>
    /// Called to check if a process is a valid nacl module to attach to.
    /// </summary>
    /// <param name="proc">Contains information about the process in question.</param>
    /// <param name="mainChromeFlags">Flags on the main Chrome process.</param>
    /// <returns>True if we should attach to the process.</returns>
    protected override bool IsPluginProcess(ProcessInfo proc, string mainChromeFlags)
    {
      // Ensure the main chrome process has the nacl debug flag, otherwise we shouldn't
      // try to attach to anything.
      if (!mainChromeFlags.Contains(Strings.NaClDebugFlag))
      {
        return false;
      }

      StringComparison ignoreCase = StringComparison.InvariantCultureIgnoreCase;
      return proc.Name.Equals(Strings.NaClProcessName, ignoreCase) &&
             proc.CommandLine.Contains(Strings.NaClLoaderFlag, ignoreCase);
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
    /// Attaches the NaCl GDB debugger to the NaCl plug-in process.  Handles loading symbols
    /// and breakpoints from Visual Studio.
    /// </summary>
    /// <param name="src">The parameter is not used.</param>
    /// <param name="args">
    /// Contains the process ID to attach to, unused since debug stub is already attached.
    /// </param>
    private void Attach(object src, PluginFoundEventArgs args)
    {
      // Clean up any pre-existing GDB process (can happen if user reloads page).
      CleanUpGDBProcess();

      // Create the initialization file to read in on GDB start.
      gdbInitFileName_ = Path.GetTempFileName();
      StringBuilder contents = new StringBuilder();

      if (!string.IsNullOrEmpty(manifestPath_))
      {
        string manifestEscaped = manifestPath_.Replace("\\", "\\\\");
        contents.AppendFormat("nacl-manifest {0}\n", manifestEscaped);
      }
      else
      {
        string pluginAssemblyEscaped = pluginAssembly_.Replace("\\", "\\\\");
        contents.AppendFormat("file \"{0}\"\n", pluginAssemblyEscaped);
      }

      // irtPath_ could be null if the irt nexe was not found in the chrome
      // install.
      if (irtPath_ != null)
      {
        string irtPathEscaped = irtPath_.Replace("\\", "\\\\");
        contents.AppendFormat("nacl-irt \"{0}\"\n", irtPathEscaped);
      }

      contents.AppendFormat("target remote localhost:{0}\n", 4014);

      // Insert breakpoints from Visual Studio project.
      if (Dte.Debugger.Breakpoints != null)
      {
        foreach (Breakpoint bp in Dte.Debugger.Breakpoints)
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
            Utility.WebServerWriteLine(
                Dte,
                string.Format(Strings.UnsupportedBreakpointTypeFormat, bp.LocationType.ToString()));
          }
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
  }
}
