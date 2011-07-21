// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using Google.MsAd7.BaseImpl;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  ///<summary>
  ///  This is a test class for BreakpointErrorTest and is intended
  ///  to contain all BreakpointErrorTest Unit Tests
  ///</summary>
  [TestClass]
  public class ErrorBreakpointTest {
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
    ///  A test for setting and getting the PendingBreakpoint
    ///</summary>
    [TestMethod]
    public void PendingBreakpointTest() {
      // These are prerequisite classes for the test.
      var resolution = new ErrorBreakpointResolution();
      var target = new ErrorBreakpoint(resolution);

      var expected =
          NaClPackageTestUtils.GetPendingBreakpoint();
      target.PendingBreakpoint = expected;
      IDebugPendingBreakpoint2 output;
      Assert.AreEqual(target.GetPendingBreakpoint(out output), VSConstants.S_OK);
      Assert.AreEqual(output, expected);
    }

    ///<summary>
    ///  A test for GetBreakpointResolution
    ///</summary>
    [TestMethod]
    public void GetBreakpointResolutionTest() {
      var resolution = new ErrorBreakpointResolution();
      var target = new ErrorBreakpoint(resolution);

      IDebugErrorBreakpointResolution2 ppErrorResolution;
      IDebugErrorBreakpointResolution2 ppErrorResolutionExpected = resolution;
      const int kExpected = VSConstants.S_OK;
      var actual = target.GetBreakpointResolution(out ppErrorResolution);
      Assert.AreEqual(ppErrorResolutionExpected, ppErrorResolution);
      Assert.AreEqual(kExpected, actual);
    }
  }
}
