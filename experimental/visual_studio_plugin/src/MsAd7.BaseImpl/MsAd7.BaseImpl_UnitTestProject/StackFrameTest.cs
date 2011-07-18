using Google.MsAd7.BaseImpl;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Google.MsAd7.BaseImpl.DebugProperties;
using Microsoft.VisualStudio.Debugger.Interop;
using Google.MsAd7.BaseImpl.Interfaces;
using System;

namespace MsAd7.BaseImpl_UnitTestProject
{
    
    
    /// <summary>
    ///This is a test class for StackFrameTest and is intended
    ///to contain all StackFrameTest Unit Tests
    ///</summary>
  [TestClass()]
  public class StackFrameTest
  {


    private TestContext testContextInstance;

    /// <summary>
    ///Gets or sets the test context which provides
    ///information about and functionality for the current test run.
    ///</summary>
    public TestContext TestContext
    {
      get
      {
        return testContextInstance;
      }
      set
      {
        testContextInstance = value;
      }
    }

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
    ///A test for EnumProperties
    ///</summary>
    [TestMethod()]
    public void EnumPropertiesTest()
    {
        /*
      RegisterSet rset = null; // TODO: Initialize to an appropriate value
      IDebugThread2 thread = null; // TODO: Initialize to an appropriate value
      Module module = null; // TODO: Initialize to an appropriate value
      ISimpleDebugger dbg = null; // TODO: Initialize to an appropriate value
      StackFrame target = new StackFrame(rset, thread, module, dbg); // TODO: Initialize to an appropriate value
      enum_DEBUGPROP_INFO_FLAGS dwFields = new enum_DEBUGPROP_INFO_FLAGS(); // TODO: Initialize to an appropriate value
      uint nRadix = 0; // TODO: Initialize to an appropriate value
      Guid guidFilter = new Guid(); // TODO: Initialize to an appropriate value
      Guid guidFilterExpected = new Guid(); // TODO: Initialize to an appropriate value
      uint dwTimeout = 0; // TODO: Initialize to an appropriate value
      uint pcelt = 0; // TODO: Initialize to an appropriate value
      uint pceltExpected = 0; // TODO: Initialize to an appropriate value
      IEnumDebugPropertyInfo2 ppEnum = null; // TODO: Initialize to an appropriate value
      IEnumDebugPropertyInfo2 ppEnumExpected = null; // TODO: Initialize to an appropriate value
      int expected = 0; // TODO: Initialize to an appropriate value
      int actual;
      actual = target.EnumProperties(dwFields, nRadix, ref guidFilter, dwTimeout, out pcelt, out ppEnum);
      Assert.AreEqual(guidFilterExpected, guidFilter);
      Assert.AreEqual(pceltExpected, pcelt);
      Assert.AreEqual(ppEnumExpected, ppEnum);
      Assert.AreEqual(expected, actual);
         
      Assert.Inconclusive("Verify the correctness of this test method.");
         */
    }

    /// <summary>
    ///A test for RefreshProperties
    ///</summary>
    [TestMethod()]
    [DeploymentItem("MsAd7.BaseImpl.dll")]
    public void RefreshPropertiesTest()
    {
      //PrivateObject param0 = null; // TODO: Initialize to an appropriate value
      //StackFrame_Accessor target = new StackFrame_Accessor(param0); // TODO: Initialize to an appropriate value
      //target.RefreshProperties();
      //Assert.Inconclusive("A method that does not return a value cannot be verified.");
    }

    /// <summary>
    ///A test for StackFrame Constructor
    ///</summary>
    [TestMethod()]
    public void StackFrameConstructorTest()
    {
      /*
      RegisterSet rset = null; // TODO: Initialize to an appropriate value
      IDebugThread2 thread = null; // TODO: Initialize to an appropriate value
      Module module = null; // TODO: Initialize to an appropriate value
      ISimpleDebugger dbg = null; // TODO: Initialize to an appropriate value
      StackFrame target = new StackFrame(rset, thread, module, dbg);
      Assert.Inconclusive("TODO: Implement code to verify target");
       */
    }
  }
}
