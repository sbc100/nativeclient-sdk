using System.IO;
using Google.NaClVsx.ProjectSupport;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.Project;

namespace NaClVsx.Package_UnitTestProject
{
    
    
    /// <summary>
    ///This is a test class for NaClProjectConfigTest and is intended
    ///to contain all NaClProjectConfigTest Unit Tests
    ///</summary>
  [TestClass()]
  public class NaClProjectConfigTest
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
    ///A test for IsServerRunning
    ///</summary>
    [TestMethod()]
    public void IsServerRunningTest() {
      string hostName = "localhost";
      int portNum = 5103;
      bool expected = false; //because we haven't launched a server
      bool actual;
      actual = NaClProjectConfig.IsServerRunning(hostName, portNum);
      Assert.AreEqual(expected, actual);
    }

  }
}
