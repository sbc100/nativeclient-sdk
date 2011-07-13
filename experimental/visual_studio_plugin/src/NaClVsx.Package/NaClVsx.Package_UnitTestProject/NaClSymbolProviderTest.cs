// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System;
using System.Collections.Generic;
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

    //Use TestInitialize to run code before running each test
    [TestInitialize]
    public void MyTestInitialize() {
      sym_ = NaClPackageTestUtils.LoadLoopNexe();
      Assert.IsNotNull(sym_, "NaClSymbolProviderTest could not initialize.");
    }

    #endregion

    /// <summary>
    ///A test for PositionFromAddress
    ///</summary>
    [TestMethod]
    public void PositionFromAddressTest() {
      const uint kLoopCCLine = 9;
      var address = GetAddressForPosition(GetLoopCCPath(), kLoopCCLine);
      DocumentPosition returnedPosition =
          sym_.PositionFromAddress(address);
      Assert.IsNotNull(returnedPosition);
      Assert.AreEqual(returnedPosition.BeginPos.dwLine, kLoopCCLine);
      Assert.AreEqual(GetLoopCCPath(), returnedPosition.Path);
    }

    /// <summary>
    /// A test for AddressesFromPosition.  We can't hardcode addresses so we
    /// just test that an address is being returned.
    ///</summary>
    [TestMethod]
    public void AddressesFromPositionTest() {
      var address = GetAddressForPosition(GetLoopCCPath(), 9);
      Assert.IsTrue(address > sym_.BaseAddress);
    }

    /// <summary>
    /// A test for GetSymbolsInScope.  We should test a function that is not at
    /// the beginning of the file, to make sure the debugger can disambiguate.
    /// We should also test the beginning and end of its scope to make sure that
    /// there are no off-by-one errors.
    /// </summary>
    [TestMethod]
    public void GetSymbolsInScopeTest() {
      // This is the beginning of the main function. (Note that the lines are
      // 0-indexed in this context, so we're really looking at line 44. If the
      // function being tested works, we should get 5 symbols: argc, argv,
      // g_GlobalData, x, and y.
      var pos = new DocumentPosition(GetLoopCCPath(), 43);
      var expectedNames = new List<string> {
          "x",
          "y",
          "argc",
          "argv",
          "g_GlobalData"
      };
      TestForSymbolsInScopeAt(pos, expectedNames);
      
      // This is the line of the closing paren, at which the local variables
      // should be out of scope.
      pos = new DocumentPosition(GetLoopCCPath(), 50);
      expectedNames.Remove("x");
      expectedNames.Remove("y");
      TestForSymbolsInScopeAt(pos, expectedNames);
    }

    [TestMethod]
    public void GetSymbolTypeTest() {
      ulong addr = GetAddressForPosition(GetLoopCCPath(), 7);
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
      // loop.cc(10,0):
      // should have global variable g_gGlobalData, formal
      // parameter "count" and local variable "i"
      ulong addr = GetAddressForPosition(GetLoopCCPath(), 9);
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

    [TestMethod]
    public void GetSymbolCharValueTest() {
      ulong addr = GetAddressForPosition(GetLoopCCPath(), 38);
      IEnumerable<Symbol> symbols = sym_.GetSymbolsInScope(addr);
      // Should have 2 vars in this scope:
      // global variable g_gGlobalData, and local variable "c".
      Assert.AreEqual(2, symbols.Count());

      // First symbol should be for 'char c'
      Symbol s = symbols.First();

      // Make sure we can convert a 1 byte char
      byte[] bytes = new byte[1];
      char char_value = 'I';
      bytes[0] = (byte)char_value; // ASCII value for 'I'
      var arrBytes = new ArraySegment<byte>(bytes);
      string str_obj = sym_.SymbolValueToString(s.Key, arrBytes);
      Assert.AreEqual("73", str_obj);
      // NOTE: since the input was a byte value of 73 for 'I'
      // the output of this is a string with that value "73"
      // The VSX debugger env formats this as a 'char' because
      // of the data type of the variable, so we
      // convert the string value to a number (e.g. "73" -> 73)
      // and then check the char value and also format the
      // char value in a string and check that too
      Int16 result_ascii_val = Convert.ToInt16(str_obj);
      char result_char = Convert.ToChar(result_ascii_val);
      Assert.AreEqual('I', result_char);
      Assert.AreEqual("I", String.Format(
          "{0:c}", Convert.ToChar(result_char)));
    }

    ///<summary>
    /// A test for GetNextLocation
    ///</summary>
    [TestMethod]
    public void GetNextLocationTest() {
      //Assert.Inconclusive("Verify the correctness of this test method.");
    }

    /// <summary>
    ///A test for GetFunctionDetails
    ///</summary>
    [TestMethod]
    public void GetFunctionDetailsTest() {
      //Assert.Inconclusive("Verify the correctness of this test method.");
    }

    /// <summary>
    ///A test for FunctionFromAddress
    ///</summary>
    [TestMethod]
    public void FunctionFromAddressTest() {
      var address = GetAddressForPosition(GetLoopCCPath(), 9);
      Function actual = sym_.FunctionFromAddress(address);
      Assert.AreEqual("print_line", actual.Name);
    }


    /// <summary>
    ///A test for NaClSymbolProvider Constructor
    ///</summary>
    [TestMethod]
    public void NaClSymbolProviderConstructorTest() {
      //Assert.Inconclusive("TODO: Implement code to verify target");
    }

    #region Private Implementation

    private static string root_;
    private NaClSymbolProvider sym_;

    private ulong GetAddressForPosition(string path, uint line) {
      var pos = new DocumentPosition(path, line);
      var addresses = sym_.AddressesFromPosition(pos);
      Assert.IsNotNull(addresses);
      Assert.AreEqual(1, addresses.Count());
      return addresses.First();
    }

    private static string GetLoopCCPath() {
      return root_ + @"\src\loop\loop.cc";
    }

    private void TestForSymbolsInScopeAt(DocumentPosition position,
      IEnumerable<string> expectedSymbols) {
        var addresses = sym_.AddressesFromPosition(position);
        var address = addresses.First();
        
        var symbols = sym_.GetSymbolsInScope(address);
        Assert.AreEqual(expectedSymbols.Count(), symbols.Count());

        foreach (var symbol in symbols)
        {
          Assert.IsTrue(expectedSymbols.Contains(symbol.Name));
        }
    }

    #endregion
  }
}
