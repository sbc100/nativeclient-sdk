// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using System.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.DebugProperties;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Google.NaClVsx.DebugSupport;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  ///<summary>
  ///  This is a test class for NaClSymbolProviderTest and is intended
  ///  to contain all NaClSymbolProviderTest Unit Tests
  ///</summary>
  [TestClass]
  public class NaClSymbolProviderTest {
    ///<summary>
    ///  Gets or sets the test context which provides
    ///  information about and functionality for the current test run.
    ///</summary>
    public TestContext TestContext { get; set; }

    #region Additional test attributes

    //Use TestInitialize to run code before running each test
    [TestInitialize]
    public void MyTestInitialize() {
      sym_ = NaClPackageTestUtils.LoadLoopNexe();
      Assert.IsNotNull(sym_, "NaClSymbolProviderTest could not initialize.");
    }

    #endregion

    ///<summary>
    ///  A test for PositionFromAddress
    ///</summary>
    [TestMethod]
    public void PositionFromAddressTest() {
      const uint kLoopCCLine = NaClPackageTestUtils.kPrintLineRow;
      var address = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(), kLoopCCLine, sym_);
      var returnedPosition =
          sym_.PositionFromAddress(address);
      Assert.IsNotNull(returnedPosition);
      Assert.AreEqual(returnedPosition.BeginPos.dwLine, kLoopCCLine);
      Assert.AreEqual(
          NaClPackageTestUtils.GetLoopCCPath(),
          returnedPosition.Path);
    }

    ///<summary>
    ///  A test for AddressesFromPosition.  We can't hardcode addresses so we
    ///  just test that an address is being returned.
    ///</summary>
    [TestMethod]
    public void AddressesFromPositionTest() {
      const uint kLoopCCLine = NaClPackageTestUtils.kPrintLineRow;
      var address = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(), kLoopCCLine, sym_);
      Assert.IsTrue(address > sym_.BaseAddress);
    }

    /// <summary>
    ///   A test for GetSymbolsInScope.  We should test a function that is not
    ///   at the beginning of the file, to make sure the debugger can
    ///   disambiguate.  We should also test the beginning and end of its scope
    ///   to make sure that there are no off-by-one errors.
    /// </summary>
    [TestMethod]
    public void GetSymbolsInScopeTest() {
      // This is the beginning of the main function. (Note that the lines are
      // 0-indexed in this context, so we're really looking at line 44. If the
      // function being tested works, we should get 5 symbols: argc, argv,
      // g_GlobalData, x, and y.
      var pos = new DocumentPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kMainStartRow);
      var expectedNames = new List<string> {
          "x",
          "y",
          "test_string",          
          "argc",
          "argv",
          "g_GlobalData"
      };
      TestForSymbolsInScopeAt(pos, expectedNames);

      // This is the line of the closing paren, at which the local variables
      // should be out of scope.
      pos = new DocumentPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kMainEndRow);
      expectedNames.Remove("x");
      expectedNames.Remove("y");
      expectedNames.Remove("test_string");
      TestForSymbolsInScopeAt(pos, expectedNames);
    }

    [TestMethod]
    public void GetSymbolTypeIntTest() {
      var addr = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kDeclarationOfIRow,
          sym_);
      var symbols = sym_.GetSymbolsInScope(addr);

      // first symbol should be "i"
      var s = symbols.First();
      Assert.AreEqual("i", s.Name);
      var t = sym_.GetSymbolType(s.Key);

      Assert.AreEqual("int", t.Name);
      Assert.IsTrue(4 == t.SizeOf);
      Assert.IsFalse(0 == t.Key);
      Assert.IsFalse(t.Key == s.Key);
    }

    [TestMethod]
    public void GetSymbolTypeStringTest()
    {
      var addr = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kDeclarationOfTestStringRow,
          sym_);
      var symbols = sym_.GetSymbolsInScope(addr);

      var testStringSymbol = new Symbol {
          Name = null,
          Key = 0
      };
      foreach (var symbol in symbols) {
        if ( "test_string" == symbol.Name ) {
          testStringSymbol = symbol;
        }
      }
      Assert.IsNotNull(testStringSymbol.Name);
      Assert.AreNotEqual(0, testStringSymbol.Key);

      Assert.AreEqual("test_string", testStringSymbol.Name);
      var symbolType = sym_.GetSymbolType(testStringSymbol.Key);

      Assert.AreEqual("string", symbolType.Name);
      Assert.IsTrue(4 == symbolType.SizeOf);
      Assert.IsFalse(0 == symbolType.Key);
      Assert.IsFalse(symbolType.Key == testStringSymbol.Key);
    }

    [TestMethod]
    public void GetSymbolValueTest() {
      // loop.cc(10,0):
      // should have global variable g_gGlobalData, formal
      // parameter "count" and local variable "i"
      var addr = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kPrintLineRow,
          sym_);
      var symbols = sym_.GetSymbolsInScope(addr);

      // first symbol should be "i"
      var symbol = symbols.First();

      Assert.AreEqual("i", symbol.Name);

      var bytes = BitConverter.GetBytes((Int64) 1234567);
      var arrBytes = new ArraySegment<byte>(bytes);
      var symbolValue = sym_.SymbolValueToString(symbol.Key, arrBytes);
      Assert.AreEqual(symbolValue, "1234567");

      bytes = BitConverter.GetBytes((Int64) (-1234567));
      arrBytes = new ArraySegment<byte>(bytes);
      symbolValue = sym_.SymbolValueToString(symbol.Key, arrBytes);
      Assert.AreEqual(symbolValue, "-1234567");
    }

    [TestMethod]
    public void GetSymbolCharValueTest() {
      var addr = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kPrintCharTypePrintfRow,
          sym_);
      var symbols = sym_.GetSymbolsInScope(addr);
      // Should have 2 vars in this scope:
      // global variable g_gGlobalData, and local variable "c".
      Assert.AreEqual(2, symbols.Count());

      // First symbol should be for 'char c'
      var s = symbols.First();

      // Make sure we can convert a 1 byte char
      var bytes = new byte[1];
      var char_value = 'I';
      bytes[0] = (byte) char_value; // ASCII value for 'I'
      var arrBytes = new ArraySegment<byte>(bytes);
      var str_obj = sym_.SymbolValueToString(s.Key, arrBytes);
      Assert.AreEqual("73", str_obj);
      // NOTE: since the input was a byte value of 73 for 'I'
      // the output of this is a string with that value "73"
      // The VSX debugger env formats this as a 'char' because
      // of the data type of the variable, so we
      // convert the string value to a number (e.g. "73" -> 73)
      // and then check the char value and also format the
      // char value in a string and check that too
      var result_ascii_val = Convert.ToInt16(str_obj);
      var result_char = Convert.ToChar(result_ascii_val);
      Assert.AreEqual('I', result_char);
      Assert.AreEqual(
          "I",
          String.Format(
              "{0:c}", Convert.ToChar(result_char)));
    }

    ///<summary>
    ///  A test for GetNextLocation
    ///</summary>
    [TestMethod]
    public void GetNextLocationTest() {
      //Assert.Inconclusive("Verify the correctness of this test method.");
    }

    ///<summary>
    ///  A test for GetFunctionDetails
    ///</summary>
    [TestMethod]
    public void GetFunctionDetailsTest() {
      //Assert.Inconclusive("Verify the correctness of this test method.");
    }

    /// <summary>
    ///   A test for GetPreviousFrameState
    /// </summary>
    [TestMethod]
    public void GetPreviousFrameStateTest() {
      var currentRegisters = new RegisterSet(
          RegisterSetSchema.DwarfAmd64Integer, null);
      // TODO: Initialize to an appropriate value
      var address = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kPrintLineRow,
          sym_);
      var privates = new PrivateObject(sym_);
      // We have to mock the calls into the symbol database to try to retrieve
      // memory from the debugger.
      var debuggerMock = new NaClDebuggerMock();
      debuggerMock.RecordCall("GetU64", (ulong) 123464, (ulong) 256100);
      debuggerMock.RecordCall("GetU64", (ulong) 123456, (ulong) 256200);
      privates.SetField("dbg_", debuggerMock);

      currentRegisters["RIP"] = address;
      currentRegisters["RBP"] = 123456 + sym_.BaseAddress;

      var result = sym_.GetPreviousFrameState(currentRegisters);
      Assert.AreEqual((ulong) 256100, result["RIP"]);
      Assert.AreEqual((ulong) 256200, result["RBP"]);
      Assert.AreEqual((ulong) 51539731024, result["CFA"]);
      // Check the new boundary condition with an RIP value that outside the
      // memory range.
      currentRegisters["RIP"] = 1 + sym_.BaseAddress + sym_.BaseAddress;
      result = sym_.GetPreviousFrameState(currentRegisters);
      Assert.IsNull(result);
      // And with an address that is too low to be in our untrusted memory
      // range.
      currentRegisters["RIP"] = 1;
      result = sym_.GetPreviousFrameState(currentRegisters);
      Assert.IsNull(result);
    }

    ///<summary>
    ///  A test for FunctionFromAddress
    ///</summary>
    [TestMethod]
    public void FunctionFromAddressTest() {
      var address = NaClPackageTestUtils.GetAddressForPosition(
          NaClPackageTestUtils.GetLoopCCPath(),
          NaClPackageTestUtils.kPrintLineRow,
          sym_);
      var actual = sym_.FunctionFromAddress(address);
      Assert.AreEqual("print_line", actual.Name);
    }


    ///<summary>
    ///  A test for NaClSymbolProvider Constructor
    ///</summary>
    [TestMethod]
    public void NaClSymbolProviderConstructorTest() {
      //Assert.Inconclusive("TODO: Implement code to verify target");
    }

    #region Private Implementation

    private NaClSymbolProvider sym_;

    #endregion

    #region Private Implementation

    private void TestForSymbolsInScopeAt(
        DocumentPosition position,
        IEnumerable<string> expectedSymbols) {
      var addresses = sym_.AddressesFromPosition(position);
      var address = addresses.First();

      var symbols = sym_.GetSymbolsInScope(address);
      Assert.AreEqual(expectedSymbols.Count(), symbols.Count());

      foreach (var symbol in symbols) {
        Assert.IsTrue(expectedSymbols.Contains(symbol.Name));
      }
    }

    #endregion
  }
}
