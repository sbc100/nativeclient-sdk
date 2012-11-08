// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;
  using System.Runtime.InteropServices;

  /// <summary>
  /// Interface for IOleMessageFilter.
  /// </summary>
  [ComImport, Guid("00000016-0000-0000-C000-000000000046"),
  InterfaceTypeAttribute(ComInterfaceType.InterfaceIsIUnknown)]
  public interface IOleMessageFilter
  {
    /// <summary>
    /// Handles calls for the thread.
    /// </summary>
    /// <param name="dwCallType">The parameter is not used.</param>
    /// <param name="hTaskCaller">The parameter is not used.</param>
    /// <param name="dwTickCount">The parameter is not used.</param>
    /// <param name="lpInterfaceInfo">The parameter is not used.</param>
    /// <returns>Code indicating message was handled.</returns>
    [System.Diagnostics.CodeAnalysis.SuppressMessage(
      "Microsoft.StyleCop.CSharp.NamingRules",
      "SA1305:FieldNamesMustNotUseHungarianNotation",
      Justification = "Matching variable name to COM interface")]
    [PreserveSig]
    int HandleInComingCall(
        int dwCallType,
        IntPtr hTaskCaller,
        int dwTickCount,
        IntPtr lpInterfaceInfo);

    /// <summary>
    /// Automatically retries the failed call.
    /// </summary>
    /// <param name="hTaskCallee">The parameter is not used.</param>
    /// <param name="dwTickCount">The parameter is not used.</param>
    /// <param name="dwRejectType">The parameter is not used.</param>
    /// <returns>Code indicating call should be retried.</returns>
    [System.Diagnostics.CodeAnalysis.SuppressMessage(
      "Microsoft.StyleCop.CSharp.NamingRules",
      "SA1305:FieldNamesMustNotUseHungarianNotation",
      Justification = "Matching variable name to COM interface")]
    [PreserveSig]
    int RetryRejectedCall(IntPtr hTaskCallee, int dwTickCount, int dwRejectType);

    /// <summary>
    /// Handles an incoming message by indicating it should be dispatched always.
    /// </summary>
    /// <param name="hTaskCallee">The parameter is not used.</param>
    /// <param name="dwTickCount">The parameter is not used.</param>
    /// <param name="dwPendingType">The parameter is not used.</param>
    /// <returns>Code indicating message should be dispatched.</returns>
    [System.Diagnostics.CodeAnalysis.SuppressMessage(
      "Microsoft.StyleCop.CSharp.NamingRules",
      "SA1305:FieldNamesMustNotUseHungarianNotation",
      Justification = "Matching variable name to COM interface")]
    [PreserveSig]
    int MessagePending(IntPtr hTaskCallee, int dwTickCount, int dwPendingType);
  }

  /// <summary>
  /// This class receives messages from the COM calls to Visual Studio
  /// and auto-retries them if they fail because VS is busy.
  /// </summary>
  public class ComMessageFilter : IOleMessageFilter
  {
    /// <summary>
    /// Note this registers the filter only for the current thread.
    /// </summary>
    public static void Register()
    {
      IOleMessageFilter oldFilter = null;
      CoRegisterMessageFilter(new ComMessageFilter(), out oldFilter);
    }

    /// <summary>
    /// Note this only closes the filter for the current thread.
    /// </summary>
    public static void Revoke()
    {
      IOleMessageFilter oldFilter = null;
      CoRegisterMessageFilter(null, out oldFilter);
    }

    /// <summary>
    /// Handles calls for the thread.
    /// </summary>
    /// <param name="dwCallType">The parameter is not used.</param>
    /// <param name="hTaskCaller">The parameter is not used.</param>
    /// <param name="dwTickCount">The parameter is not used.</param>
    /// <param name="lpInterfaceInfo">The parameter is not used.</param>
    /// <returns>Code indicating message was handled.</returns>
    [System.Diagnostics.CodeAnalysis.SuppressMessage(
      "Microsoft.StyleCop.CSharp.NamingRules",
      "SA1305:FieldNamesMustNotUseHungarianNotation",
      Justification = "Matching variable name to COM interface")]
    int IOleMessageFilter.HandleInComingCall(
        int dwCallType,
        System.IntPtr hTaskCaller,
        int dwTickCount,
        System.IntPtr lpInterfaceInfo)
    {
      return 0;  // SERVERCALL_ISHANDLED.
    }

    /// <summary>
    /// Automatically retries the failed call.
    /// </summary>
    /// <param name="hTaskCallee">The parameter is not used.</param>
    /// <param name="dwTickCount">The parameter is not used.</param>
    /// <param name="dwRejectType">The parameter is not used.</param>
    /// <returns>Code indicating call should be retried.</returns>
    [System.Diagnostics.CodeAnalysis.SuppressMessage(
      "Microsoft.StyleCop.CSharp.NamingRules",
      "SA1305:FieldNamesMustNotUseHungarianNotation",
      Justification = "Matching variable name to COM interface")]
    int IOleMessageFilter.RetryRejectedCall(
        System.IntPtr hTaskCallee,
        int dwTickCount,
        int dwRejectType)
    {
      // If reject type is SERVERCALL_RETRYLATER.
      if (dwRejectType == 2)
      {
        // Immediate retry.
        return 99;
      }

      // Cancel call.
      return -1;
    }

    /// <summary>
    /// Handles an incoming message by indicating it should be dispatched always.
    /// </summary>
    /// <param name="hTaskCallee">The parameter is not used.</param>
    /// <param name="dwTickCount">The parameter is not used.</param>
    /// <param name="dwPendingType">The parameter is not used.</param>
    /// <returns>Code indicating message should be dispatched.</returns>
    [System.Diagnostics.CodeAnalysis.SuppressMessage(
      "Microsoft.StyleCop.CSharp.NamingRules",
      "SA1305:FieldNamesMustNotUseHungarianNotation",
      Justification = "Matching variable name to COM interface")]
    int IOleMessageFilter.MessagePending(
        System.IntPtr hTaskCallee, 
        int dwTickCount,
        int dwPendingType)
    {
      return 2; // PENDINGMSG_WAITDEFPROCESS.
    }

    // Implement the IOleMessageFilter interface.
    [DllImport("Ole32.dll")]
    private static extern int CoRegisterMessageFilter(
        IOleMessageFilter newFilter, out IOleMessageFilter oldFilter);
  }
}
