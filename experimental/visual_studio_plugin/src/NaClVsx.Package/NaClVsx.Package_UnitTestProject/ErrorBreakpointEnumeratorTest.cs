// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  ///<summary>
  ///  This is a test class for BreakpointErrorEnumTest and is intended
  ///  to contain all BreakpointErrorEnumTest Unit Tests
  ///</summary>
  [TestClass]
  public class ErrorBreakpointEnumeratorTest {
    ///<summary>
    ///  Gets or sets the test context which provides
    ///  information about and functionality for the current test run.
    ///</summary>
    public TestContext TestContext { get; set; }

    #region Additional test attributes

    //
    //You can use the following additional attributes as you write your tests:
    //
    //Use ClassInitialize to run code before running the first test in the class
    //[ClassInitialize()]
    //public static void MyClassInitialize(TestContext testContext)
    //{
    //}
    //
    //Use ClassCleanup to run code after all tests in a class have run
    //[ClassCleanup()]
    //public static void MyClassCleanup()
    //{
    //}
    //
    //Use TestInitialize to run code before running each test
    //[TestInitialize()]
    //public void MyTestInitialize()
    //{
    //}
    //
    //Use TestCleanup to run code after each test has run
    //[TestCleanup()]
    //public void MyTestCleanup()
    //{
    //}
    //

    #endregion

    ///<summary>
    ///  A test for Skip
    ///</summary>
    [TestMethod]
    public void SkipTest() {
      var target = new ErrorBreakpointEnumerator();
      const uint kSkipSize = 33;
      // Skip should fail until there's something in the enum.
      Assert.AreNotEqual(target.Skip(kSkipSize), VSConstants.S_OK);
      PopulateErrorEnum(target, 123);
      // Now it should not fail.
      Assert.AreEqual(target.Skip(kSkipSize), VSConstants.S_OK);
      uint nextCount = 0;
      // We now check to make sure we skipped to the correct place.
      var errorPointer = new ErrorBreakpoint[1];
      Assert.AreEqual(
          target.Next(1, errorPointer, ref nextCount), VSConstants.S_OK);
      Assert.AreEqual(
          GetIntMessage(kSkipSize),
          GetMessageFromBreakpointError(errorPointer[0]));
    }

    ///<summary>
    ///  A test for Reset
    ///</summary>
    [TestMethod]
    public void ResetTest() {
      var target = new ErrorBreakpointEnumerator();

      uint count = 0;
      const uint kArraySize = 20;
      var errors = new ErrorBreakpoint[kArraySize];
      // Now make sure we can iterate when data is available.
      PopulateErrorEnum(target, 123);
      Assert.AreEqual(
          target.Next(kArraySize, errors, ref count), VSConstants.S_OK);
      Assert.IsTrue(count == kArraySize);
      // The NextTest method verifies that errors would now be filled with entries
      // 0 through 19 and that the next call to next would return entry 20.
      target.Reset();
      Assert.AreEqual(target.Next(1, errors, ref count), VSConstants.S_OK);
      Assert.AreEqual(
          GetMessageFromBreakpointError(errors[0]), GetIntMessage(0));
    }

    ///<summary>
    ///  A test for PopulateBreakpoint
    ///</summary>
    [TestMethod]
    public void PopulateBreakpointTest() {
      var target = new ErrorBreakpointEnumerator();
      var breakpoint =
          NaClPackageTestUtils.GetPendingBreakpoint();
      // If PopulateBreakpoint works for a few entries it should work for any number
      const int kSampleSize = 3;
      uint returnedCount = 0;
      var breakpointErrors = new IDebugErrorBreakpoint2[kSampleSize];
      IDebugPendingBreakpoint2 returnedBreakpoint;
      PopulateErrorEnum(target, kSampleSize);
      // First, we verify that we cannot get pendingBreakpoints out of the BreakpointErrors before
      // we call PopulateBreakpoint.
      Assert.AreEqual(
          target.Next(kSampleSize, breakpointErrors, ref returnedCount),
          VSConstants.S_OK);
      for (var i = 0; i < kSampleSize; ++i) {
        Assert.AreNotEqual(
            breakpointErrors[i].GetPendingBreakpoint(out returnedBreakpoint),
            VSConstants.S_OK);
      }
      // Now we call it and make sure that PendingBreakpoint was populated on all
      // BreakpointErrors.
      target.PopulateBreakpoint(breakpoint);
      target.Reset();
      Assert.AreEqual(
          target.Next(kSampleSize, breakpointErrors, ref returnedCount),
          VSConstants.S_OK);
      for (var i = 0; i < kSampleSize; ++i) {
        Assert.AreEqual(
            breakpointErrors[i].GetPendingBreakpoint(out returnedBreakpoint),
            VSConstants.S_OK);
        Assert.AreEqual(returnedBreakpoint, breakpoint);
      }
    }

    ///<summary>
    ///  A test for Next
    ///</summary>
    [TestMethod]
    public void NextTest() {
      var target = new ErrorBreakpointEnumerator();
      // We should not be able to call next successfully until breakpoints have been added.
      uint count = 0;
      const uint kArraySize = 20;
      var errors = new ErrorBreakpoint[kArraySize];
      Assert.AreNotEqual(
          target.Next(kArraySize, errors, ref count), VSConstants.S_OK);
      // Now make sure we can iterate when data is available.
      PopulateErrorEnum(target, 123);
      Assert.AreEqual(
          target.Next(kArraySize, errors, ref count), VSConstants.S_OK);
      Assert.IsTrue(count == kArraySize);
      // Make sure the right errors are in the array);
      for (var i = 0; i < kArraySize; ++i) {
        var expectedMessage = GetIntMessage(i);
        Assert.AreEqual(
            expectedMessage, GetMessageFromBreakpointError(errors[i]));
      }
      Assert.AreEqual(target.Next(1, errors, ref count), VSConstants.S_OK);
      Assert.IsTrue(count == 1);

      Assert.AreEqual(
          GetMessageFromBreakpointError(errors[0]), GetIntMessage(kArraySize));
    }


    ///<summary>
    ///  A test for Insert
    ///</summary>
    [TestMethod]
    public void InsertTest() {
      var target = new ErrorBreakpointEnumerator();
      IDebugErrorBreakpoint2 breakpointError =
          new ErrorBreakpoint(new ErrorBreakpointResolution());

      uint count;
      Assert.AreEqual(target.GetCount(out count), VSConstants.S_OK);
      Assert.IsTrue(count == 0);
      target.Insert(breakpointError);
      Assert.AreEqual(target.GetCount(out count), VSConstants.S_OK);
      Assert.IsTrue(count == 1);

      PopulateErrorEnum(target, 1000);
      Assert.AreEqual(target.GetCount(out count), VSConstants.S_OK);
      Assert.IsTrue(count == 1001);
    }

    ///<summary>
    ///  A test for GetCount
    ///</summary>
    [TestMethod]
    public void GetCountTest() {
      var target = new ErrorBreakpointEnumerator();
      PopulateErrorEnum(target, 13233);
      uint count;

      Assert.AreEqual(target.GetCount(out count), VSConstants.S_OK);
      Assert.IsTrue(count == 13233);
    }

    ///<summary>
    ///  A test for Clone
    ///</summary>
    [TestMethod]
    public void CloneTest() {
      var target = new ErrorBreakpointEnumerator();
      IEnumDebugErrorBreakpoints2 clone = null;
      uint kEnumSize = 23;
      uint returnedEnumSize = 0;
      var targetErrors = new ErrorBreakpoint[kEnumSize];
      var cloneErrors = new ErrorBreakpoint[kEnumSize];
      PopulateErrorEnum(target, kEnumSize);
      Assert.AreEqual(target.Clone(out clone), VSConstants.S_OK);
      Assert.AreEqual(clone.GetCount(out returnedEnumSize), VSConstants.S_OK);
      Assert.AreEqual(returnedEnumSize, kEnumSize);
      Assert.AreEqual(
          target.Next(kEnumSize, targetErrors, ref returnedEnumSize),
          VSConstants.S_OK);
      Assert.AreEqual(returnedEnumSize, kEnumSize);
      Assert.AreEqual(
          clone.Next(kEnumSize, cloneErrors, ref returnedEnumSize),
          VSConstants.S_OK);
      Assert.AreEqual(returnedEnumSize, kEnumSize);
      for (uint i = 0; i < kEnumSize; ++i) {
        Assert.AreEqual(
            GetMessageFromBreakpointError(targetErrors[i]),
            GetMessageFromBreakpointError(cloneErrors[i]));
      }
    }

    #region Private Implementation

    private static string GetIntMessage<T>(T number) {
      return string.Format("This was breakpoint: {0}", number);
    }

    private static string GetMessageFromBreakpointError(ErrorBreakpoint error) {
      IDebugErrorBreakpointResolution2 resolution;
      var resolutionInfo = new BP_ERROR_RESOLUTION_INFO[1];

      Assert.AreEqual(
          error.GetBreakpointResolution(out resolution), VSConstants.S_OK);
      Assert.AreEqual(
          resolution.GetResolutionInfo(
              enum_BPERESI_FIELDS.BPERESI_MESSAGE, resolutionInfo),
          VSConstants.S_OK);
      return resolutionInfo[0].bstrMessage;
    }

    private static void PopulateErrorEnum(ErrorBreakpointEnumerator target,
                                          uint numberOfEntriesToAdd) {
      for (uint i = 0; i < numberOfEntriesToAdd; ++i) {
        var errorResolution = new ErrorBreakpointResolution();
        errorResolution.Message = GetIntMessage(i);
        var newError = new ErrorBreakpoint(errorResolution);
        target.Insert(newError);
      }
    }

    #endregion
  }
}
