// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Google.NaClVsx.DebugSupport;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NaClVsx.Package_UnitTestProject {
  /// <summary>
  ///This is a test class for NaClSymbolProviderTest and is intended
  ///to contain all NaClSymbolProviderTest Unit Tests
  ///</summary>
  [TestClass]
  public class NaClSymbolProviderTest {
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
    [ClassInitialize]
    public static void MyClassInitialize(TestContext testContext) {
      root_ = Environment.GetEnvironmentVariable("NACL_VSX_ROOT");
      if (root_ == null) {
        root_ = "c:\\sample\\dir";
      }
    }

    //Use ClassCleanup to run code after all tests in a class have run
    //[ClassCleanup()]
    //public static void MyClassCleanup()
    //{
    //}
    //
    //Use TestInitialize to run code before running each test
    [TestInitialize]
    public void MyTestInitialize() {
      string status;

      sym_ = new NaClSymbolProvider(null);
      bool ok = sym_.LoadModule(
          Path.Combine(root_, nexePath), baseAddr, out status);
      Assert.IsTrue(ok, "LoadModule failed");
    }

    //Use TestCleanup to run code after each test has run
    //[TestCleanup()]
    //public void MyTestCleanup()
    //{
    //}
    //

    #endregion

    /// <summary>
    ///A test for PositionFromAddress
    ///</summary>
    [TestMethod]
    public void PositionFromAddressTest() {
      // loop.cc(10,0): 0x20380 (32 bytes)
      DocumentPosition pos = sym_.PositionFromAddress(baseAddr + 0x20385);
      Assert.IsNotNull(pos);
      Assert.AreEqual(pos, new DocumentPosition("loop.cc", 9));
    }

    /// <summary>
    ///A test for AddressesFromPosition
    ///</summary>
    [TestMethod]
    public void AddressesFromPositionTest() {
      // loop.cc(10,0): 0x20380 (32 bytes)
      ulong addr = baseAddr + 0x20380;
      DocumentPosition pos = sym_.PositionFromAddress(addr);
      Assert.IsNotNull(pos);

      IEnumerable<ulong> addr2 = sym_.AddressesFromPosition(pos);
      Assert.IsTrue(addr2.Contains(addr));
    }

    /// <summary>
    ///A test for AddressesFromPosition
    ///</summary>
    [TestMethod]
    public void AddressesFromPositionAbsPathTest() {
      // loop.cc(10,0): 0x20380 (32 bytes)
      // should have formal parameter "count" and local variable "i"
      string abspath = @"c:\hg\nvs\src\loop\loop.cc";
      ulong addr = baseAddr + 0x20380;
      var pos = new DocumentPosition(abspath, 9);

      IEnumerable<ulong> addr2 = sym_.AddressesFromPosition(pos);
      Assert.IsTrue(addr2.Contains(addr));

      // This next bit tests to be sure we're remembering the abs path
      DocumentPosition pos2 = sym_.PositionFromAddress(addr);
      Assert.AreEqual(abspath, pos2.Path);
    }


    ///<summary>
    ///A test for GetSymbolsInScope
    ///</summary>
    [TestMethod]
    public void GetSymbolsInScopeTest() {
      // loop.cc(10,0): 0x20380 (32 bytes)
      // should have global variable g_gGlobalData, formal
      // parameter "count" and local variable "i"
      ulong addr = baseAddr + 0x20380;
      IEnumerable<Symbol> symbols = sym_.GetSymbolsInScope(addr);
      Assert.AreEqual(3, symbols.Count());
    }

    [TestMethod]
    public void GetSymbolTypeTest() {
      // loop.cc(10,0): 0x20380 (32 bytes)
      // should have global variable g_gGlobalData, formal
      // parameter "count" and local variable "i"
      ulong addr = baseAddr + 0x20380;
      IEnumerable<Symbol> symbols = sym_.GetSymbolsInScope(addr);

      // first symbol should be "i"
      Symbol s = symbols.First();
      Assert.AreEqual("i", s.Name);
      SymbolType t = sym_.GetSymbolType(s.Key);

      Assert.AreEqual("int", t.Name);
      Assert.IsTrue(4 == t.SizeOf);
      Assert.IsFalse(0 == t.Key);
      Assert.IsFalse(t.Key == s.Key);
    }

    [TestMethod]
    public void GetSymbolValueTest() {
      // loop.cc(10,0): 0x20380 (32 bytes)
      // should have global variable g_gGlobalData, formal
      // parameter "count" and local variable "i"
      ulong addr = baseAddr + 0x20380;
      IEnumerable<Symbol> symbols = sym_.GetSymbolsInScope(addr);

      // first symbol should be "i"
      Symbol s = symbols.First();


      Assert.AreEqual("i", s.Name);
      SymbolType t = sym_.GetSymbolType(s.Key);

      byte[] bytes = BitConverter.GetBytes((Int64) 1234567);
      var arrBytes = new ArraySegment<byte>(bytes);
      string o = sym_.SymbolValueToString(s.Key, arrBytes);
      Assert.AreEqual(o, "1234567");

      bytes = BitConverter.GetBytes((Int64) (-1234567));
      arrBytes = new ArraySegment<byte>(bytes);
      o = sym_.SymbolValueToString(s.Key, arrBytes);
      Assert.AreEqual(o, "-1234567");
    }

    ///<summary>
    /// A test for GetNextLocation
    ///</summary>
    [TestMethod]
    public void GetNextLocationTest() {
      Assert.Inconclusive("Verify the correctness of this test method.");
    }

    /// <summary>
    ///A test for GetFunctionDetails
    ///</summary>
    [TestMethod]
    public void GetFunctionDetailsTest() {
      Assert.Inconclusive("Verify the correctness of this test method.");
    }

    /// <summary>
    ///A test for FunctionFromAddress
    ///</summary>
    [TestMethod]
    public void FunctionFromAddressTest() {
      ulong address = baseAddr + 0x20380;
      Function actual = sym_.FunctionFromAddress(address);
      Assert.AreEqual("print_line", actual.Name);
    }


    /// <summary>
    ///A test for NaClSymbolProvider Constructor
    ///</summary>
    [TestMethod]
    public void NaClSymbolProviderConstructorTest() {
      Assert.Inconclusive("TODO: Implement code to verify target");
    }

    #region Private Implementation

    private static readonly ulong baseAddr = 0x00000ffc00000000;

    private static readonly string codePath =
        @"src\loop\loop.cc";

    private static readonly string nexePath =
        @"src\loop\loop.nexe";

    private static string root_;
    private NaClSymbolProvider sym_;

    #endregion
  }
}
