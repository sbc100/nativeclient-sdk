// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.IO;
using System.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.NaClVsx.DebugSupport;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  /// <summary>
  ///   Contains various utilities that are useful for multiple tests.
  /// </summary>
  class NaClPackageTestUtils {
    #region  constants

    /// <summary>
    ///   This is the location at which I is declared in void main in loop.cc.
    /// </summary>
    public const uint kDeclarationOfIRow = 7;

    /// <summary>
    ///   This is the call site of the foo function in loop.cc.
    /// </summary>
    public const uint kFooCallSiteRow = 47;

    /// <summary>
    ///   This is the line of the closing paren of the main function in loop.cc.
    /// </summary>
    public const uint kMainEndRow = 50;

    /// <summary>
    ///   This is the first line of the body of the main function in loop.cc.
    /// </summary>
    public const uint kMainStartRow = 43;

    /// <summary>
    ///   This is the location of the printf in print_char_type in loop.cc
    /// </summary>
    public const uint kPrintCharTypePrintfRow = 38;

    /// <summary>
    ///   This is the call site of the print_line function in loop.cc.
    /// </summary>
    public const uint kPrintLineCallSiteRow = 23;

    /// <summary>
    ///   This is a line number somewhere in the print_line function in loop.cc.
    ///   We don't care exactly which one.
    /// </summary>
    public const uint kPrintLineRow = 9;

    /// <summary>
    ///   This is the call site of the print_loop function in loop.cc.
    /// </summary>
    public const uint kPrintLoopCallSiteRow = 30;

    #endregion

    /// <summary>
    ///   Creates a new NaClSymbolProvider and parses our loop.nexe sample
    ///   binary.
    /// </summary>
    /// <returns>The pre-loaded symbol provider.</returns>
    public static NaClSymbolProvider LoadLoopNexe() {
      var symbolProvider = new NaClSymbolProvider(null);
      string status;
      var ok = symbolProvider.LoadModule(
          Path.Combine(
              GetVsxRootDir(),
              kNexePath),
          kBaseAddr,
          out status);
      Assert.IsTrue(ok, "LoadModule failed");
      return symbolProvider;
    }

    public static string GetLoopCCPath() {
      return vsxRootDir + kCCPath;
    }

    /// <summary>
    ///   Finds the SDK root, using the system environment.  Will fail to assert
    ///   if root cannot be located.
    /// </summary>
    /// <returns>The location of the VSX root directory.</returns>
    public static string GetVsxRootDir() {
      if (null == vsxRootDir) {
        vsxRootDir = Environment.GetEnvironmentVariable("NACL_VSX_ROOT");
      }
      Assert.IsNotNull(
          vsxRootDir,
          "Could not get NACL_VSX_ROOT from environment.");
      return vsxRootDir;
    }

    public static ulong GetAddressForPosition(
        string path, uint line, ISimpleSymbolProvider symbolProvider) {
      var pos = new DocumentPosition(path, line);
      var addresses = symbolProvider.AddressesFromPosition(pos);
      Assert.IsNotNull(addresses);
      Assert.AreEqual(1, addresses.Count());
      return addresses.First();
    }

    public static PendingBreakpoint GetPendingBreakpoint() {
      var sdbMock = new NaClDebuggerMock();
      sdbMock.Symbols = new NaClSymbolProvider(sdbMock);
      var requestMock = new BreakpointRequestMock();
      var request = new BreakpointRequest(requestMock);

      // The instance to test.
      return new PendingBreakpoint(sdbMock, request);
    }

    #region Private Implementation

    /// <summary>
    ///   The location realtive to the  at which the loop nexe is to be found.
    /// </summary>
    private static readonly string kCCPath = @"src\loop\loop.cc";

    /// <summary>
    ///   The location realtive to the  at which the loop nexe is to be found.
    /// </summary>
    private static readonly string kNexePath = @"src\loop\loop.nexe";

    /// <summary>
    ///   The location of the visual studio plugin code.
    /// </summary>
    private static string vsxRootDir;

    #endregion

    /// <summary>
    ///   The base address at which symbol information should start in nexe code.
    /// </summary>
    public static readonly ulong kBaseAddr = 0x0000000c00000000;
  }
}
