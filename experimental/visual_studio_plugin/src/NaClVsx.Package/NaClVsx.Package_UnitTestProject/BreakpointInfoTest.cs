// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Collections.Generic;
using System.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.NaClVsx.DebugSupport;
using Google.NaClVsx.DebugSupport.DWARF;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  ///<summary>
  ///  This is a test class for BreakpointInfoTest and is intended
  ///  to contain all BreakpointInfoTest Unit Tests
  ///</summary>
  [TestClass]
  public class BreakpointInfoTest {
    ///<summary>
    ///  Gets or sets the test context which provides
    ///  information about and functionality for the current test run.
    ///</summary>
    public TestContext TestContext { get; set; }

    #region Additional test attributes

    #endregion

    /// <summary>
    ///   This test checks to make sure that an error is returned if a bogus line number is
    ///   requested.
    /// </summary>
    [TestMethod]
    public void GetBindErrorsTestCritical() {
      var expectedErrorTypes = new List<enum_BP_ERROR_TYPE>();
      const enum_BP_ERROR_TYPE kExpectedErrorType = enum_BP_ERROR_TYPE.BPET_GENERAL_ERROR;
      expectedErrorTypes.Add(kExpectedErrorType);
      TestGetBindErrors(100000000, VSConstants.S_FALSE, expectedErrorTypes);
    }

    /// <summary>
    ///   A test for the case where there are no bind errors.
    /// </summary>
    [TestMethod]
    public void GetBindErrorsTestNoErrors() {
      TestGetBindErrors(NaClPackageTestUtils.kMainStartRow,
                        VSConstants.S_OK,
                        new List<enum_BP_ERROR_TYPE>());
    }

    /// <summary>
    ///   This test checks what happens when a breakpoint is requested on a line that does not
    ///   contain any code.  The expected outcome is to have a breakpoint on the next line with
    ///   code.
    /// </summary>
    [TestMethod]
    public void GetBindErrorsTestNonCritical() {
      var expectedErrorTypes = new List<enum_BP_ERROR_TYPE>();
      const enum_BP_ERROR_TYPE kExpectedResolutionType =
        enum_BP_ERROR_TYPE.BPET_SEV_LOW | enum_BP_ERROR_TYPE.BPET_TYPE_WARNING;
      expectedErrorTypes.Add(kExpectedResolutionType);
      TestGetBindErrors(NaClPackageTestUtils.kBlankLineInPrintLoop,
                        BreakpointInfo.kBindErrorWarning,
                        expectedErrorTypes);
    }

    ///<summary>
    ///  A test for GetBindAddresses at the end of a function.
    ///</summary>
    [TestMethod]
    public void GetBindAddressesTestEndOfMain() {
      TestGetBindAddresses(NaClPackageTestUtils.kMainEndRow, 1);
    }

    ///<summary>
    ///  A test for GetBindAddresses in the body of a function.
    ///</summary>
    [TestMethod]
    public void GetBindAddressesTestStartOfMain() {
      TestGetBindAddresses(NaClPackageTestUtils.kMainStartRow, 1);
    }

    /// <summary>
    ///   This tests to make sure that Breakpoint Info will work inside of a
    ///   function.
    /// </summary>
    [TestMethod]
    public void GetBreakpointLocationsAtStartOfMainTest() {
      var expectedSymbols = new List<string> {
          "x",
          "y",
          "test_string",
          "argc",
          "argv",
          "g_GlobalData"
      };

      TestLoopCCLocation(
          NaClPackageTestUtils.kMainStartRow,
          "main",
          expectedSymbols);
    }

    /// <summary>
    ///   This tests to make sure that Breakpoint Info will work at the end of a
    ///   scope.
    /// </summary>
    [TestMethod]
    public void GetBreakpointLocationsAtEndOfMainTest() {
      var expectedSymbols = new List<string> {
          "argc",
          "argv",
          "g_GlobalData"
      };

      TestLoopCCLocation(
          NaClPackageTestUtils.kMainEndRow,
          "main",
          expectedSymbols);
    }

    // None of our tests should modify the SymbolProvider, so it should be OK
    // to instantiate it once.  This saves seconds for each instantiation.

    #region Private Implementation

    NaClSymbolProvider symbolProvider_;

    #endregion

    #region Private Implementation

    /// <summary>
    ///   Sets up the SymbolProvider.  This function only has an effect the
    ///   first time it is called and initializes symbolProvider_.  If a test
    ///   modifies the contents of SymbolProvider or of the SymbolDB, then it
    ///   should use its own instances of both.
    /// </summary>
    private void InitializeSymbolProvider() {
      if (null == symbolProvider_) {
        symbolProvider_ = NaClPackageTestUtils.LoadLoopNexe();
        Assert.IsNotNull(symbolProvider_);
      }
    }

    /// <summary>
    ///   Runs BreakpointInfo.GetBindAddresses for the given location in loop.cc
    ///   and verifies that the expected number of addresses was returned.
    ///   While this function could work for locations at which no bind address
    ///   was expected, it does not support calls that would result in an error.
    ///   Those should be tested separately.
    /// </summary>
    /// <param name = "lineToTest">
    ///   The line of code to test.
    /// </param>
    /// <param name = "expectedBindAddressCount">
    ///   The number of addresses to expect (Usually 1)
    /// </param>
    private void TestGetBindAddresses(uint lineToTest,
                                      int expectedBindAddressCount) {
      InitializeSymbolProvider();
      var symbolProviderPrivates = new PrivateObject(symbolProvider_);
      var database = symbolProviderPrivates.GetField("db_") as SymbolDatabase;
      var target = new BreakpointInfo(database, symbolProvider_);
      var position = new DocumentPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          lineToTest);
      IEnumerable<ulong> outAddresses;
      IEnumDebugErrorBreakpoints2 outErrorEnum;
      const int kExpected = VSConstants.S_OK;
      var actual = target.GetBindAddresses(
          position, out outAddresses, out outErrorEnum);
      Assert.AreEqual(expectedBindAddressCount, outAddresses.Count());
      uint errorCount;
      outErrorEnum.GetCount(out errorCount);
      Assert.AreEqual((uint) 0, errorCount);
      Assert.AreEqual(kExpected, actual);
    }

    /// <summary>
    ///   Runs BreakpointInfo.GetBindErrors for the given location in loop.cc and verifies that
    ///   the expected error types were returned.
    /// </summary>
    /// <param name = "lineToTest">The line of code to test.</param>
    /// <param name="expectedReturnCode">What return code to expect from GetBindErrors()</param>
    /// <param name="expectedTypes">What error resolutions are expected to result from this
    ///   call.</param>
    private void TestGetBindErrors(uint lineToTest,
                                   int expectedReturnCode,
                                   List<enum_BP_ERROR_TYPE> expectedTypes)
    {
      InitializeSymbolProvider();
      var symbolProviderPrivates = new PrivateObject(symbolProvider_);
      var database = symbolProviderPrivates.GetField("db_") as SymbolDatabase;
      var target = new BreakpointInfo(database, symbolProvider_);
      var position = new DocumentPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          lineToTest);
      IEnumDebugErrorBreakpoints2 outErrorEnum;
      var actualReturnCode = target.GetBindErrors(
          position, out outErrorEnum);
      Assert.AreEqual(expectedReturnCode, actualReturnCode);
      uint errorCount;
      outErrorEnum.GetCount(out errorCount);
      Assert.AreEqual((uint)expectedTypes.Count(), errorCount);
      if (0 != expectedTypes.Count()) {
        var errorsFetched = 0u;
        var errorBreakpointArray = new IDebugErrorBreakpoint2[errorCount];
        outErrorEnum.Next(errorCount, errorBreakpointArray, ref errorsFetched);
        Assert.AreEqual(errorsFetched, errorCount);
        Assert.AreEqual(expectedTypes.Count(), errorBreakpointArray.Length);
        for (var i = 0; i < expectedTypes.Count(); ++i) {
          IDebugErrorBreakpointResolution2 actualBreakpointResolution;
          errorBreakpointArray[i].GetBreakpointResolution(out actualBreakpointResolution);
          Assert.AreEqual(expectedTypes[i],
                          ((ErrorBreakpointResolution) actualBreakpointResolution).Type);
        }
      }
    }

    /// <summary>
    ///   Tests the call to GetBreakpointLocations individually, using a
    ///   private object.  For this test, we depend on SymbolProvider's test
    ///   suite passing.  If it does, then we can validate a source code
    ///   location by using the SymbolProvider to get it's address, and then
    ///   to get its position.  If the position we end up with is the same one
    ///   we started at, then the test passes.
    /// </summary>
    /// <param name = "lineToTest">
    ///   The line at which to check for breakpoint info.
    /// </param>
    /// <param name = "expectedFunctionName">
    ///   The function in which the breakpoint is expected to occur.</param>
    /// <param name = "expectedSymbols">
    ///   A container of Symbols that should be expected to exist at the location of the breakpoint.
    /// </param>
    private void TestLoopCCLocation(uint lineToTest,
                                    string expectedFunctionName,
                                    IEnumerable<string> expectedSymbols) {
      InitializeSymbolProvider();
      var symbolProviderPrivates = new PrivateObject(symbolProvider_);
      var database = symbolProviderPrivates.GetField("db_") as SymbolDatabase;
      Assert.IsNotNull(database);
      var breakpointInfo = new BreakpointInfo(database, symbolProvider_);
      var breakpointInfoPrivates = new PrivateObject(breakpointInfo);

      var files = database.SourceFilesByFilename["loop.cc"];
      var msvcPosition = new DocumentPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          lineToTest);
      // Normally GetBindAddresses handles the conversion from the MSVC Line
      // to the DWARF line position, but we're bypassing the public interface,
      // so we have to do it ourselves.
      var sourceLocations =
          (IEnumerable<SymbolDatabase.SourceLocation>)
          breakpointInfoPrivates.Invoke(
              "GetBreakpointLocations",
              files.First(),
              NaClSymbolProvider.GetDwarfLineIndex(msvcPosition.BeginPos.dwLine));
      Assert.AreEqual(1, sourceLocations.Count());

      var breakpointLocation = sourceLocations.First();
      var breakpointAddress = breakpointLocation.StartAddress +
                              symbolProvider_.BaseAddress;
      var function =
          symbolProvider_.FunctionFromAddress(
              breakpointLocation.StartAddress +
              symbolProvider_.BaseAddress);
      var breakpointPosition =
          symbolProvider_.PositionFromAddress(breakpointAddress);
      Assert.AreEqual(expectedFunctionName, function.Name);
      Assert.AreEqual(
          msvcPosition.BeginPos.dwLine, breakpointPosition.BeginPos.dwLine);

      var symbols = symbolProvider_.GetSymbolsInScope(breakpointAddress);
      Assert.AreEqual(expectedSymbols.Count(), symbols.Count());

      foreach (var symbol in symbols) {
        Assert.IsTrue(expectedSymbols.Contains(symbol.Name));
      }
    }

    #endregion
  }
}
