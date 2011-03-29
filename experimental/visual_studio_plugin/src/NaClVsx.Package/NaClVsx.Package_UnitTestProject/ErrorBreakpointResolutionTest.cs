// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System;
using System.Runtime.InteropServices;
using Google.MsAd7.BaseImpl;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NaClVsx.Package_UnitTestProject {
  /// <summary>
  ///This is a test class for BreakpointErrorResolutionTest and is intended
  ///to contain all BreakpointErrorResolutionTest Unit Tests
  ///</summary>
  [TestClass]
  public class ErrorBreakpointResolutionTest {
    /// <summary>
    ///Gets or sets the test context which provides
    ///information about and functionality for the current test run.
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

    /// <summary>
    ///A test for SetType
    ///</summary>
    [TestMethod]
    public void SetTypeTest() {
      var target = new ErrorBreakpointResolution();
      TestType(target, enum_BP_ERROR_TYPE.BPET_ALL);
      TestType(target, enum_BP_ERROR_TYPE.BPET_GENERAL_ERROR);
      TestType(target, enum_BP_ERROR_TYPE.BPET_GENERAL_WARNING);
      TestType(target, enum_BP_ERROR_TYPE.BPET_NONE);
      TestType(target, enum_BP_ERROR_TYPE.BPET_SEV_GENERAL);
      TestType(target, enum_BP_ERROR_TYPE.BPET_SEV_HIGH);
      TestType(target, enum_BP_ERROR_TYPE.BPET_SEV_LOW);
      TestType(target, enum_BP_ERROR_TYPE.BPET_SEV_MASK);
      TestType(target, enum_BP_ERROR_TYPE.BPET_TYPE_ERROR);
      TestType(target, enum_BP_ERROR_TYPE.BPET_TYPE_MASK);
      TestType(target, enum_BP_ERROR_TYPE.BPET_TYPE_WARNING);
    }

    /// <summary>
    ///A test for SetMessage
    ///</summary>
    [TestMethod]
    public void SetMessageTest() {
      var target = new ErrorBreakpointResolution();
      const string kMessage = "This is a test message";

      var info = new BP_ERROR_RESOLUTION_INFO[1];
      info[0] = new BP_ERROR_RESOLUTION_INFO();

      target.Message = kMessage;
      Assert.AreEqual(
          target.GetResolutionInfo(enum_BPERESI_FIELDS.BPERESI_MESSAGE, info),
          VSConstants.S_OK);
      Assert.IsTrue(
          (info[0].dwFields & enum_BPERESI_FIELDS.BPERESI_MESSAGE) != 0);
      Assert.AreEqual(info[0].bstrMessage, kMessage);
    }

    /// <summary>
    ///A test for SetLocation
    ///</summary>
    [TestMethod]
    public void SetLocationTest() {
      // We have to start by testing some assumptions about int pointers
      IntPtr intPtr = IntPtr.Zero;
      Assert.IsTrue(intPtr.Equals(IntPtr.Zero));
      Assert.IsTrue(intPtr == IntPtr.Zero);

      var target = new ErrorBreakpointResolution();
      const enum_BP_TYPE kBpType = enum_BP_TYPE.BPT_CODE;
      const ulong kAddress = 123456;
      const string kTestPath = "C:\\test\\location";
      var docContext = new DocumentContext(new DocumentPosition(kTestPath, 123));
      target.SetLocation(kBpType, kAddress, docContext);

      var info = new BP_ERROR_RESOLUTION_INFO[1];
      info[0] = new BP_ERROR_RESOLUTION_INFO();

      // the usual tests for GetResolutionInfo
      Assert.AreEqual(
          target.GetResolutionInfo(
              enum_BPERESI_FIELDS.BPERESI_BPRESLOCATION, info),
          VSConstants.S_OK);
      Assert.IsTrue(
          (info[0].dwFields & enum_BPERESI_FIELDS.BPERESI_BPRESLOCATION) != 0);
      Assert.AreEqual(
          (enum_BP_TYPE) info[0].bpResLocation.bpType, enum_BP_TYPE.BPT_CODE);

      // making sure we can extract the location info properly is harder
      IntPtr pointer = info[0].bpResLocation.unionmember1;
      var infoContext = Marshal.GetObjectForIUnknown(pointer) as CodeContext;
      Assert.IsTrue(infoContext != null);
      Assert.AreEqual(infoContext.Address, kAddress);
      IDebugDocumentContext2 infoDocContext;
      Assert.AreEqual(
          infoContext.GetDocumentContext(out infoDocContext), VSConstants.S_OK);
      Assert.AreEqual(infoDocContext, docContext);
      Marshal.Release(pointer);
    }

    /// <summary>
    ///A test for GetResolutionInfo
    ///</summary>
    [TestMethod]
    public void GetResolutionInfoTest() {
      var target = new ErrorBreakpointResolution();
      const enum_BPERESI_FIELDS kDwFields = enum_BPERESI_FIELDS.BPERESI_TYPE |
                                            enum_BPERESI_FIELDS.BPERESI_MESSAGE;
      const string kWarningMessage = "A test warning has occurred";
      const enum_BP_ERROR_TYPE kTestType =
          enum_BP_ERROR_TYPE.BPET_GENERAL_WARNING;
      target.Type = kTestType;
      target.Message = kWarningMessage;

      var info = new BP_ERROR_RESOLUTION_INFO[1];
      Assert.AreEqual(
          target.GetResolutionInfo(kDwFields, info), VSConstants.S_OK);
      Assert.AreEqual(info[0].dwFields, kDwFields);
      Assert.AreEqual(info[0].bstrMessage, kWarningMessage);
      Assert.AreEqual(info[0].dwType, kTestType);
    }

    /// <summary>
    ///A test for GetBreakpointType
    ///</summary>
    [TestMethod]
    public void GetBreakpointTypeTest() {
      var target = new ErrorBreakpointResolution();
          // TODO: Initialize to an appropriate value
      var pBpType = new enum_BP_TYPE[1];
          // TODO: Initialize to an appropriate value
      pBpType[0] = new enum_BP_TYPE();

      Assert.AreEqual(target.GetBreakpointType(pBpType), VSConstants.S_OK);
      Assert.AreEqual(pBpType[0], enum_BP_TYPE.BPT_CODE);
    }

    #region Private Implementation

    private static void TestType(ErrorBreakpointResolution target,
                                 enum_BP_ERROR_TYPE errorType) {
      var info = new BP_ERROR_RESOLUTION_INFO[1];
      info[0] = new BP_ERROR_RESOLUTION_INFO();

      target.Type = errorType;
      Assert.AreEqual(
          target.GetResolutionInfo(enum_BPERESI_FIELDS.BPERESI_TYPE, info),
          VSConstants.S_OK);
      Assert.IsTrue((info[0].dwFields & enum_BPERESI_FIELDS.BPERESI_TYPE) != 0);
      Assert.AreEqual(info[0].dwType, errorType);
    }

    #endregion
  }
}
