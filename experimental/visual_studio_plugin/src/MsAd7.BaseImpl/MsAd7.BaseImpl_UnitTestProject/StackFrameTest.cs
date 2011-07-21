// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.DebugProperties;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace MsAd7.BaseImpl_UnitTestProject {
  ///<summary>
  ///  This is a test class for StackFrameTest and is intended
  ///  to contain all StackFrameTest Unit Tests
  ///</summary>
  [TestClass]
  public class StackFrameTest {
    ///<summary>
    ///  A test for EnumProperties
    ///</summary>
    [TestMethod]
    public void EnumPropertiesTest() {
      var debuggerMock = new SimpleDebuggerMock();
      var target = testStackFrameConstructor(debuggerMock);
      var symbolProviderMock = debuggerMock.Symbols as SymbolProviderMock;
      PrimeMockForRefreshProperties(symbolProviderMock);
      //This guid was taken from a run with a debugger.
      var guidFilter = new Guid(
          -414768709, 4288, 16629, 128, 127, 146, 13, 55, 249, 84, 25);
      IEnumDebugPropertyInfo2 output;
      uint pcelt;
      target.EnumProperties(
          enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_NAME,
          16,
          ref guidFilter,
          10000,
          out pcelt,
          out output);
      uint propertiesFound;
      output.GetCount(out propertiesFound);
      Assert.AreEqual((uint) 4, pcelt);
    }

    ///<summary>
    ///  A test for RefreshProperties
    ///</summary>
    [TestMethod]
    [DeploymentItem("MsAd7.BaseImpl.dll")]
    public void RefreshPropertiesTest() {
      var debuggerMock = new SimpleDebuggerMock();
      var target = testStackFrameConstructor(debuggerMock);
      var symbolProviderMock = debuggerMock.Symbols as SymbolProviderMock;
      PrimeMockForRefreshProperties(symbolProviderMock);
      var privates = new PrivateObject(target);
      privates.Invoke("RefreshProperties");

      var properties = privates.GetField("properties_")
                       as Dictionary<string, DebugPropertyBase>;
      Assert.AreEqual(3, properties.Count);
    }

    /// <summary>
    ///   The actual test is bundled into a private function so the other unit
    ///   tests can use the functionality as well.
    /// </summary>
    [TestMethod]
    public void StackFrameConstructorTest() {
      var debuggerMock = new SimpleDebuggerMock();
      testStackFrameConstructor(debuggerMock);
    }

    #region Private Implementation

    private const ulong kIpRegisterContent = 123456;

    #endregion

    #region Private Implementation

    /// <summary>
    ///   This test tests RefreshProperties.
    ///   The actual test is bundled into a private function so the other unit
    ///   tests can use the functionality as well.
    /// </summary>
    /// <param name = "symbolProviderMock"></param>
    /// <param name = "stackFrame"></param>
    private static void PrimeMockForRefreshProperties(
        SymbolProviderMock symbolProviderMock) {
      var symbol1 = new Symbol {Name = "Symbol_1"};
      var symbol2 = new Symbol {Name = "Symbol_2"};
      var symbol3 = new Symbol {Name = "Symbol_3"};
      var symbols = new List<Symbol> {symbol1, symbol2, symbol3};
      symbolProviderMock.RecordCall(
          "GetSymbolsInScope", kIpRegisterContent, symbols);
    }

    /// <summary>
    ///   This is a test for the construction of our representation of a
    ///   StackFrame. It is bundled into a private function so the other unit
    ///   tests can use the functionality as well.
    /// </summary>
    /// <param name = "debuggerMock">This is passed in as a parameter so that a
    ///   test function can use this constructor test and the run other tests
    ///   using the same mock.</param>
    /// <returns>The newly allocated StackFrame instance</returns>
    private StackFrame testStackFrameConstructor(
        SimpleDebuggerMock debuggerMock) {
      // Set up the classes we need.
      var docPosition = new DocumentPosition("/test/path", 314);
      var debugPropertyBase = new DebugPropertyBase(
          null,
          "DebugPropertyName",
          "UsefulType",
          null,
          123456,
          enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_NONE,
          debuggerMock);

      var registerSet = new RegisterSet(
          RegisterSetSchema.DwarfAmd64Integer,
          debugPropertyBase);
      registerSet["RIP"] = kIpRegisterContent;
      var debugThreadMock = new DebugThread2Mock();
      var module = new Module();
      // Prime the mocks.
      var symbolProviderMock = debuggerMock.Symbols as SymbolProviderMock;
      symbolProviderMock.RecordCall(
          "PositionFromAddress",
          kIpRegisterContent,
          docPosition);
      symbolProviderMock.RecordCall(
          "AddressesFromPosition",
          docPosition,
          new List<ulong> {kIpRegisterContent});
      symbolProviderMock.RecordCall(
          "PositionFromAddress",
          kIpRegisterContent,
          docPosition);
      symbolProviderMock.RecordCall(
          "FunctionFromAddress",
          kIpRegisterContent,
          new Function());
      // Run the test.
      var stackFrame = new StackFrame(
          registerSet, debugThreadMock, module, debuggerMock);
      Assert.IsNotNull(stackFrame);
      return stackFrame;
    }

    #endregion
  }
}
