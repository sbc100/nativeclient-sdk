// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using Google.NaClVsx.DebugSupport;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NaClVsx.DebugHelpers;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  ///<summary>
  ///  This is a test class for ThreadTest and is intended
  ///  to contain all ThreadTest Unit Tests
  ///</summary>
  [TestClass]
  public class ThreadTest {
    ///<summary>
    ///  Gets or sets the test context which provides
    ///  information about and functionality for the current test run.
    ///</summary>
    public TestContext TestContext { get; set; }

    /// <summary>
    ///   A test for EnumFrameInfo.  This test is bulky because it's the only
    ///   test in this class at the moment.  A lot of the code here can be
    ///   reused and should be factored when more tests are added.
    /// </summary>
    [TestMethod]
    public void EnumFrameInfoTest() {
      // In order for this test to mean anything we need a real symbol
      // provider.
      var symbolProvider = NaClPackageTestUtils.LoadLoopNexe();
      Assert.IsNotNull(
          symbolProvider,
          "NaClSymbolProviderTest could not initialize.");
      var debuggerMock = new NaClDebuggerMock();
      var symbolProviderPrivates = new PrivateObject(symbolProvider);
      symbolProviderPrivates.SetField("dbg_", debuggerMock);
      debuggerMock.Symbols = symbolProvider;
      // We can also use a real NaClDebugProcess which can be equipped with
      // some mocks.
      IDebugPort2 debugPort2 = new DebugPort2Mock();
      var process = new NaClDebugProcess(
          debugPort2, "connectionString", 123, "imagePath");
      var program = new ProgramNode(process);
      var programPrivates = new PrivateObject(program);
      programPrivates.SetField("dbg_", debuggerMock);

      var name = "loop";
      uint tid = 1;
      var target = new Thread(program, name, tid);
      var dwFieldSpec = new enum_FRAMEINFO_FLAGS();
      uint nRadix = 0;
      IEnumDebugFrameInfo2 ppDebugFrames;
      // Now it's time to prime the mocks.
      var loopCCPath = NaClPackageTestUtils.GetLoopCCPath();
      var mockRegisters = new RegsX86_64();

      // We need the following addresses to create a simulated callstack.
      var printLineAddress = NaClPackageTestUtils.GetAddressForPosition(
          loopCCPath, NaClPackageTestUtils.kPrintLineRow, symbolProvider);
      var printLoopAddress = NaClPackageTestUtils.GetAddressForPosition(
          loopCCPath,
          NaClPackageTestUtils.kPrintLineCallSiteRow,
          symbolProvider);
      var fooAddress = NaClPackageTestUtils.GetAddressForPosition(
          loopCCPath,
          NaClPackageTestUtils.kPrintLoopCallSiteRow,
          symbolProvider);
      var mainAddress = NaClPackageTestUtils.GetAddressForPosition(
          loopCCPath, NaClPackageTestUtils.kFooCallSiteRow, symbolProvider);
      // A million is a nice round number which makes it easy to calculate the
      // expected register values.  We can use it for the register values we
      // mock.
      var dummyValidAddress = 1000000 + symbolProvider.BaseAddress;

      mockRegisters.Rip = printLineAddress;
      mockRegisters.Rbp = dummyValidAddress;
      // As the stack is being analyzed, the debugger will traverse up the
      // call chain.  The mock will thus return code locations from the inside
      // out to simulate this procedure.
      // Note that the symbolprovider has been loaded with real call frame
      // information, and that the non-dummy addresses have been parsed out of
      // the loop nexe as well.
      debuggerMock.RecordCall("GetRegisters", (uint) 1, mockRegisters);
      debuggerMock.RecordCall("GetU64", (ulong) 1000008, printLoopAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 1000000, dummyValidAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 1000008, fooAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 1000000, dummyValidAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 1000008, mainAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 1000000, dummyValidAddress);
      // Having the same mainAddress returned twice simulates the infinite loop
      // that was fixed in RefreshFrameInfo()
      // http://code.google.com/p/nativeclient/issues/detail?id=2012
      debuggerMock.RecordCall("GetU64", (ulong) 1000008, mainAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 1000000, dummyValidAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 999984, dummyValidAddress);
      debuggerMock.RecordCall("GetU64", (ulong) 999992, dummyValidAddress);
      var returned = target.EnumFrameInfo(
          dwFieldSpec, nRadix, out ppDebugFrames);
      Assert.AreEqual(VSConstants.S_OK, returned);
      uint frameCount;
      ppDebugFrames.GetCount(out frameCount);
      Assert.AreEqual((uint) 4, frameCount);
    }
  }
}
