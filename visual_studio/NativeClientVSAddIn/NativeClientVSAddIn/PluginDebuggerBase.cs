// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;
  using System.Collections.Generic;
  using System.Windows.Forms;

  using EnvDTE;
  using EnvDTE80;

  /// <summary>
  /// This is a base class for encapsulating functionality related to attaching a debugger
  /// to a nacl/pepper plug-in.  This base class mostly contains functionality related to finding
  /// the plug-in.
  /// </summary>
  public class PluginDebuggerBase : IDisposable
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
    /// </summary>
    /// <param name="dte">Automation object from Visual Studio.</param>
    /// <param name="properties">PropertyManager set to a valid project/platform.</param>
    protected PluginDebuggerBase(DTE2 dte, PropertyManager properties)
    {
      if (dte == null)
      {
        throw new ArgumentNullException("dte");
      }

      if (properties == null)
      {
        throw new ArgumentNullException("properties");
      }

      Dte = dte;

      // Every second, check for a new instance of the plug-in to attach to.
      // Note that although the timer itself runs on a separate thread, the event
      // is fired from the main UI thread during message processing, thus we do not
      // need to worry about threading issues.
      pluginFinderTimer_ = new Timer();
      pluginFinderTimer_.Tick += new EventHandler(FindAndAttachToPlugin);
      pluginFinderForbiddenPids_ = new List<uint>();
      processSearcher_ = new ProcessSearcher();

      pluginFinderTimer_.Interval = InitialPluginCheckFrequency;
      pluginFinderTimer_.Start();
    }

    /// <summary>
    /// Finalizer. Should clean up unmanaged resources. Should not be overriden in derived classes.
    /// </summary>
    ~PluginDebuggerBase()
    {
      Dispose(false);
    }

    /// <summary>
    /// An event indicating a target plug-in was found on the system.
    /// </summary>
    public event EventHandler<PluginFoundEventArgs> PluginFoundEvent;

    /// <summary>
    /// Gets or sets a value indicating whether this object has been disposed of already.
    /// </summary>
    protected bool Disposed { get; set; }

    /// <summary>
    /// Gets or sets the main visual studio object.
    /// </summary>
    protected DTE2 Dte { get; set; }

    /// <summary>
    /// Disposes the object when called by user code (not directly by garbage collector).
    /// </summary>
    public void Dispose()
    {
      Dispose(true);
      GC.SuppressFinalize(this);
    }

    /// <summary>
    /// This is called periodically by the Visual Studio UI thread to look for our plug-in process
    /// and attach the debugger to it.  The call is triggered by the pluginFinderTimer_ object.
    /// </summary>
    /// <param name="unused">The parameter is not used.</param>
    /// <param name="unused1">The parameter is not used.</param>
    public void FindAndAttachToPlugin(object unused, EventArgs unused1)
    {
      StringComparison ignoreCase = StringComparison.InvariantCultureIgnoreCase;

      // This function is called by the main Visual Studio event loop and we may have put the event
      // on the queue just before disposing it meaning this could be called after we've disposed.
      if (Disposed)
      {
        return;
      }

      // Set the main chrome process that was started by visual studio.  If it's not chrome
      // or not found then we have no business attaching to any plug-ins so return.
      if (debuggedChromeMainProcess_ == null)
      {
        foreach (Process proc in Dte.Debugger.DebuggedProcesses)
        {
          if (proc.Name.EndsWith(Strings.ChromeProcessName, ignoreCase))
          {
            debuggedChromeMainProcess_ = System.Diagnostics.Process.GetProcessById(proc.ProcessID);
            break;
          }
        }

        return;
      }

      if (debuggedChromeMainProcess_.HasExited)
      {
        // Might happen if we're shutting down debugging.
        return;
      }

      // Get the list of all descendants of the main chrome process.
      uint mainChromeProcId = (uint)debuggedChromeMainProcess_.Id;
      List<ProcessInfo> chromeDescendants = processSearcher_.GetDescendants(mainChromeProcId);
      if (chromeDescendants.Count == 0)
      {
        // Might happen if we're shutting down debugging.
        return;
      }

      ProcessInfo mainProcInfo = chromeDescendants.Find(p => p.ID == mainChromeProcId);
      if (mainProcInfo == null)
      {
        // Might happen if we're shutting down debugging.
        return;
      }

      string mainChromeFlags = mainProcInfo.CommandLine;

      // From the list of descendants, find the plug-in by it's command line arguments and
      // process name as well as not being attached to already.
      List<ProcessInfo> plugins = chromeDescendants.FindAll(p =>
          IsPluginProcess(p, mainChromeFlags) && !pluginFinderForbiddenPids_.Contains(p.ID));

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
    /// Disposes the object. If disposing is false then this has been called by garbage collection,
    /// and we shouldn't reference managed objects.
    /// </summary>
    /// <param name="disposing">True if user call to Dispose, false if garbase collection.</param>
    protected virtual void Dispose(bool disposing)
    {
      if (!Disposed && disposing)
      {
        pluginFinderTimer_.Stop();
      }
      
      Disposed = true;
    }

    /// <summary>
    /// Called to check if a process is a valid plugin to attach to.
    /// </summary>
    /// <param name="proc">Contains information about the process in question.</param>
    /// <param name="mainChromeFlags">Flags on the main Chrome process.</param>
    /// <returns>True if we should attach to the process.</returns>
    protected virtual bool IsPluginProcess(ProcessInfo proc, string mainChromeFlags)
    {
      throw new InvalidOperationException();
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
