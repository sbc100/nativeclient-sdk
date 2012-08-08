// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;

  using EnvDTE80;

  /// <summary>
  /// This class handles the details of finding a pepper plugin and attaching to it.
  /// </summary>
  public class PluginDebuggerVS : PluginDebuggerBase
  {
    /// <summary>
    /// Path to the actual plug-in assembly.
    /// </summary>
    private string pluginAssembly_;

    /// <summary>
    /// Constructs the PluginDebuggerHelper.
    /// </summary>
    /// <param name="dte">Automation object from Visual Studio.</param>
    /// <param name="properties">PropertyManager pointing to a valid project/platform.</param>
    public PluginDebuggerVS(DTE2 dte, PropertyManager properties)
        : base(dte, properties)
    {
      pluginAssembly_ = properties.PluginAssembly;
      PluginFoundEvent += new EventHandler<PluginFoundEventArgs>(Attach);
    }

    /// <summary>
    /// Called to check if a process is a valid pepper plugin to attach to.
    /// </summary>
    /// <param name="proc">Contains information about the process in question.</param>
    /// <param name="mainChromeFlags">Flags on the main Chrome process.</param>
    /// <returns>True if we should attach to the process.</returns>
    protected override bool IsPluginProcess(ProcessInfo proc, string mainChromeFlags)
    {
      StringComparison ignoreCase = StringComparison.InvariantCultureIgnoreCase;
      string identifierFlagTarget =
              string.Format(Strings.PepperProcessPluginFlagFormat, pluginAssembly_);
      return proc.Name.Equals(Strings.ChromeProcessName, ignoreCase) &&
             proc.CommandLine.Contains(Strings.ChromeRendererFlag, ignoreCase) &&
             proc.CommandLine.Contains(identifierFlagTarget, ignoreCase);
    }

    /// <summary>
    /// Attaches the Visual Studio debugger to the given process ID.
    /// </summary>
    /// <param name="src">The parameter is not used.</param>
    /// <param name="args">Contains the process ID to attach to.</param>
    private void Attach(object src, PluginFoundEventArgs args)
    {
      foreach (EnvDTE.Process proc in Dte.Debugger.LocalProcesses)
      {
        if (proc.ProcessID == args.ProcessID)
        {
          proc.Attach();
          break;
        }
      }
    }
  }
}
