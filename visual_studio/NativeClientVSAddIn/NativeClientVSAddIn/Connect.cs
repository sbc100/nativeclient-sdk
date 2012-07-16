// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;

  using EnvDTE;
  using EnvDTE80;
  using Extensibility;

  /// <summary>The object for implementing an Add-in.</summary>
  /// <seealso class='IDTExtensibility2' />
  public class Connect : IDTExtensibility2
  {
    /// <summary>
    /// Receives events related to starting/stopping debugging.
    /// </summary>
    private DebuggerEvents debuggerEvents_;

    /// <summary>
    /// Holds methods related to running the plug-in and debugging.
    /// </summary>
    private PluginDebuggerHelper debuggerHelper_;

    /// <summary>
    /// Implements the OnConnection method of the IDTExtensibility2 interface.
    /// Receives notification that the Add-in is being loaded.
    /// </summary>
    /// <param name="application">Root object of the host application.</param>
    /// <param name="connectMode">
    /// Describes how the Add-in is being loaded (e.g. command line or UI). This is unused since
    /// the add-in functions the same regardless of how it was loaded.
    /// </param>
    /// <param name="addInInst">Object representing this Add-in.</param>
    /// <param name="custom">Unused, but could contain host specific data for the add-in.</param>
    /// <seealso class='IDTExtensibility2' />
    public void OnConnection(
        object application,
        ext_ConnectMode connectMode,
        object addInInst,
        ref Array custom)
    {
      DTE2 dte = (DTE2)application;
      debuggerHelper_ = new PluginDebuggerHelper(dte);

      debuggerEvents_ = dte.Events.DebuggerEvents;
      debuggerEvents_.OnEnterDesignMode += DebuggerOnEnterDesignMode;
      debuggerEvents_.OnEnterRunMode += DebuggerOnEnterRunMode;
    }

    /// <summary>
    /// Called when Visual Studio ends a debugging session.
    /// </summary>
    /// <param name="reason">The parameter is not used.</param>
    public void DebuggerOnEnterDesignMode(dbgEventReason reason)
    {
      debuggerHelper_.StopDebugging();
    }

    /// <summary>
    /// Called when Visual Studio starts a debugging session.
    /// </summary>
    /// <param name="reason">The parameter is not used.</param>
    public void DebuggerOnEnterRunMode(dbgEventReason reason)
    {
      // If we are starting debugging (not re-entering from a breakpoint)
      // then load project settings and start the debugger-helper.
      if (reason == dbgEventReason.dbgEventReasonLaunchProgram &&
          debuggerHelper_.LoadProjectSettings())
      {
        debuggerHelper_.StartDebugging();
      }
    }

    /// <summary>
    /// Implements the OnDisconnection method of the IDTExtensibility2
    /// interface. Receives notification that the Add-in is being unloaded.
    /// </summary>
    /// <param name='disconnectMode'>Describes how the Add-in is being unloaded.</param>
    /// <param name='custom'>Array of parameters that are host application specific.</param>
    /// <seealso class='IDTExtensibility2' />
    public void OnDisconnection(ext_DisconnectMode disconnectMode, ref Array custom)
    {
    }

    /// <summary>
    /// Implements the OnAddInsUpdate method of the IDTExtensibility2 interface.
    /// Receives notification when the collection of Add-ins has changed.
    /// </summary>
    /// <param name='custom'>Array of parameters that are host application specific.</param>
    /// <seealso class='IDTExtensibility2' />
    public void OnAddInsUpdate(ref Array custom)
    {
    }

    /// <summary>
    /// Implements the OnStartupComplete method of the IDTExtensibility2 interface.
    /// Receives notification that the host application has completed loading.
    /// </summary>
    /// <param name='custom'>Array of parameters that are host application specific.</param>
    /// <seealso class='IDTExtensibility2' />
    public void OnStartupComplete(ref Array custom)
    {
    }

    /// <summary>
    /// Implements the OnBeginShutdown method of the IDTExtensibility2 interface.
    /// Receives notification that the host application is being unloaded.
    /// </summary>
    /// <param name='custom'>Array of parameters that are host application specific.</param>
    /// <seealso class='IDTExtensibility2' />
    public void OnBeginShutdown(ref Array custom)
    {
    }
  }
}