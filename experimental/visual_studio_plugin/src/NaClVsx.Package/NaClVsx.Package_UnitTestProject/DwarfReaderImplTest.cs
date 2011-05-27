using System.Collections.Generic;
using Google.NaClVsx.DebugSupport.DWARF;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NaClVsx.Package_UnitTestProject
{
  /// <summary>
  ///This is a test class for DwarfReaderImplTest and is intended
  ///to contain all DwarfReaderImplTest Unit Tests
  ///</summary>
  [TestClass()]
  public class DwarfReaderImplTest
  {


    /// <summary>
    ///Gets or sets the test context which provides
    ///information about and functionality for the current test run.
    ///</summary>
    public TestContext TestContext
    {
      get
      {
        return testContextInstance_;
      }
      set
      {
        testContextInstance_ = value;
      }
    }

    #region Additional test attributes
    // 
    //You can use the following additional attributes as you write your tests:
    //
    //Use ClassInitialize to run code before running the first test in the
    //class
    //[ClassInitialize()]
    //public static void MyClassInitialize(TestContext testContext) {
    //}
    //
    //Use ClassCleanup to run code after all tests in a class have run
    //[ClassCleanup()]
    //public static void MyClassCleanup()
    //{
    //}
    //
    //Use TestInitialize to run code before running each test
    [TestInitialize()]
    public void MyTestInitialize()
    {
      testDb_ = new SymbolDatabase();
      target_ = null;
      attributeCount_ = 0;
    }
    
    //Use TestCleanup to run code after each test has run
    //[TestCleanup()]
    //public void MyTestCleanup()
    //{
    //}
    //
    #endregion

    /// <summary>
    /// A test that exercises the entire interface.  There are quite a few
    /// functions in the interface that don't return a value or have effects
    /// that can only be verified by calling other functions.  The asserts
    /// to validate these functions are in the test functions that are being
    /// called here.
    /// </summary>
    [TestMethod()]
    public void InterfaceExerciseTest() {
      DwarfReaderImplConstructorTest();
      const string kAttributeData = "sample_data";

      StartCompilationUnitTest();
      StartDIETest();
      ProcessAttributeTest(kDieOffset, kDieOffset, DwarfAttribute.DW_AT_name,
                           kAttributeData);
      target_.EndDIE(kDieOffset);
      DefineDirTest();
      DefineFileTest();
      AddLineTest();
      AddLocListEntryTest();
      target_.EndCompilationUnit();
      BeginCfiEntryTest();
      AddCfiRuleTest();
      EndCfiEntryTest();
    }


    /// <summary>
    /// A test for StartDIE
    /// </summary>
    [TestMethod()]
    public void StartDIETest() {
      DwarfReaderImplConstructorTest();
      const ulong kParent = 0;
      const DwarfTag kTag = DwarfTag.DW_TAG_variable;
      target_.StartDIE(kParent, kDieOffset, kTag);
      // Check that the DIE was added to the symbol database
      var dbEntry = testDb_.Entries[kDieOffset];
      Assert.IsNotNull(dbEntry);
      Assert.AreEqual(dbEntry.ParentKey, kParent);
      Assert.AreEqual(dbEntry.Tag, kTag);
    }

    // <summary>
    // A test for StartCompilationUnit.
    // </summary>
    [TestMethod()]
    public void StartCompilationUnitTest() {
      DwarfReaderImplConstructorTest();
      var privates = GetPrivates();
      ushort compIndex = (ushort) privates.GetField("compilationUnitIndex_");
      Assert.AreEqual(0, compIndex);
      target_.StartCompilationUnit();
      compIndex = (ushort) privates.GetField("compilationUnitIndex_");
      Assert.AreEqual(1, compIndex);
    }



    /// <summary>
    ///A test for ProcessAttribute
    ///</summary>
    [TestMethod()]
    public void ProcessAttributeTest() {
      DwarfReaderImplConstructorTest();
      const DwarfAttribute kNameAttribute = DwarfAttribute.DW_AT_name;
      const string kNameAttributeData = "variable_name";

      const DwarfAttribute kDeclarationAttribute = 
        DwarfAttribute.DW_AT_declaration;
      const byte kDeclarationAttributeData = 1;

      const DwarfAttribute kTypeAttribute = DwarfAttribute.DW_AT_type;
      // This should actually point to another DIE but structure doesn't
      // matter for this test.
      const string kTypeAttributeData = "bogus";

      StartDIETest();
      
      ProcessAttributeTest(kDieOffset, kDieOffset, kNameAttribute,
                           kNameAttributeData);
      ProcessAttributeTest(kDieOffset, kDieOffset, kDeclarationAttribute,
                           kDeclarationAttributeData);
      ProcessAttributeTest(kDieOffset, kDieOffset, kTypeAttribute,
                           kTypeAttributeData);
    }

    /// <summary>
    ///A test for MakeFileKey
    ///</summary>
    [TestMethod()]
    [DeploymentItem("NaClVsx.Package.dll")]
    public void MakeFileKeyTest() {
      DwarfReaderImplConstructorTest();
      var privates = GetPrivates();
      const uint kFileNum = 4;
      privates.Invoke("StartCompilationUnit");
      ulong actual1 = (ulong) privates.Invoke("MakeFileKey", kFileNum);
      privates.Invoke("StartCompilationUnit");
      ulong actual2 = (ulong)privates.Invoke("MakeFileKey", kFileNum);
      Assert.AreNotEqual(actual1, actual2);
    }

    /// <summary>
    /// A test for EndDIE
    /// </summary>
    [TestMethod()]
    public void EndDIETest() {
      DwarfReaderImplConstructorTest();
      const ulong kLowData = 123456;
      const ulong kHighData = 123654;
      Assert.AreEqual(0, testDb_.ScopeTransitions.Count);
      StartDIETest();
      ProcessAttributeTest(kDieOffset, kDieOffset,
                           DwarfAttribute.DW_AT_low_pc, kLowData);
      ProcessAttributeTest(kDieOffset, kDieOffset,
                           DwarfAttribute.DW_AT_high_pc, kHighData);
      target_.EndDIE(kDieOffset);
      Assert.AreEqual(2, testDb_.ScopeTransitions.Count);
    }

    /// <summary>
    /// A test for EndCompilationUnit
    /// </summary>
    [TestMethod()]
    public void EndCompilationUnitTest() {
      DwarfReaderImplConstructorTest();
      // No point in having a test here since the function does nothing.
    }

    /// <summary>
    /// A test for EndCfiEntry
    /// </summary>
    [TestMethod()]
    public void EndCfiEntryTest()
    {
      DwarfReaderImplConstructorTest();
      const bool kExpected = true;
      BeginCfiEntryTest();
      Assert.AreEqual(0, testDb_.CallFrames.Count);
      bool actual = target_.EndCfiEntry();
      Assert.AreEqual(kExpected, actual);

      Assert.IsNull(GetPrivates().GetField("currentFrame_"));
      Assert.AreEqual(1, testDb_.CallFrames.Count);
      var callFrame = testDb_.CallFrames[kCfiEntryAddress];
      Assert.IsNotNull(callFrame);
    }

    /// <summary>
    /// A test for DefineFile
    /// </summary>
    [TestMethod()]
    public void DefineFileTest() {
      DwarfReaderImplConstructorTest();
      const string kFileName = "sampleFileName";
      target_.DefineFile(kFileName, 1, 1);
      Assert.AreEqual(testDb_.Files.Count, 1);
      var key = (ulong) GetPrivates().Invoke("MakeFileKey", (uint)1);
      Assert.AreEqual(kFileName, testDb_.Files[key].Filename);
    }

    /// <summary>
    ///A test for DefineDir
    ///</summary>
    [TestMethod()]
    public void DefineDirTest() {
      DwarfReaderImplConstructorTest();
      const string kDirName = "sampleDirName";
      const uint kDirNum = 42;
      target_.DefineDir(kDirName, kDirNum);
      var dirs = (Dictionary<uint, string>) GetPrivates().GetField("dirs_");
      Assert.AreEqual(dirs[kDirNum], kDirName);
    }

    /// <summary>
    ///A test for BeginCfiEntry
    ///</summary>
    [TestMethod()]
    public void BeginCfiEntryTest()
    {
      DwarfReaderImplConstructorTest();
      bool actual = target_.BeginCfiEntry(kCfiEntryAddress);
      Assert.AreEqual(true, actual);
      Assert.AreNotEqual(null, GetPrivates().GetField("currentFrame_"));
    }

    /// <summary>
    ///A test for AddLocListEntry
    ///</summary>
    [TestMethod()]
    public void AddLocListEntryTest()
    {
      DwarfReaderImplConstructorTest();
      const ulong kLocListOffset = 1234567;
      const ulong kLocListLowPC = 654321;
      const ulong kLocListHighPC = 654324;
      byte[] rawData = new byte[4];
      rawData[0] = 1;
      rawData[1] = 2;
      rawData[2] = 3;
      rawData[3] = 4;
      target_.AddLocListEntry(kLocListOffset, true, kLocListLowPC,
                              kLocListHighPC, rawData);
      var locList = testDb_.LocLists[kLocListOffset];
      Assert.IsNotNull(locList);
      Assert.AreEqual(1, locList.Count);
      SymbolDatabase.LocListEntry entry = locList[0];
      Assert.IsNotNull(entry);
      Assert.AreEqual(entry.StartAddress, kLocListLowPC);
      Assert.AreEqual(entry.EndAddress, kLocListHighPC);
      Assert.AreEqual(entry.Data, rawData);
    }

    /// <summary>
    ///A test for AddLine
    ///</summary>
    [TestMethod()]
    public void AddLineTest()
    {
      DwarfReaderImplConstructorTest();
      const ulong kLineAddress = 1234567;
      const ulong kLineLength = 78;
      target_.AddLine(kLineAddress, kLineLength, 1, 6, 4);

      var sourceCodeLine = testDb_.Locations[kLineAddress];
      Assert.IsNotNull(sourceCodeLine);
      Assert.AreEqual(sourceCodeLine.Length, kLineLength);
    }

    /// <summary>
    ///A test for AddCfiRule
    ///</summary>
    [TestMethod()]
    public void AddCfiRuleTest()
    {
      DwarfReaderImplConstructorTest();
      const int kUniqueId = 889999;
      const int kBaseRegister = 15;
      BeginCfiEntryTest();
      target_.AddCfiRule(kCfiEntryAddress, kUniqueId,
                         IDwarfReader.CfiRuleType.Offset,
                         kBaseRegister, 987654, new byte[4]);
      var currentFrame = 
        (SymbolDatabase.CallFrame) GetPrivates().GetField("currentFrame_");
      Assert.AreNotEqual(null, currentFrame);

      var rule = currentFrame.Rules[0];
      Assert.IsNotNull(rule);
      Assert.AreEqual(rule.Address, kCfiEntryAddress);
      Assert.AreEqual(rule.BaseRegister, kBaseRegister);
      Assert.AreEqual(rule.RegisterId, kUniqueId);
    }

    /// <summary>
    /// A test for DwarfReaderImpl Constructor
    /// This function can be called from the beginning of any of the unit
    /// tests in this file.  It will only overwrite target_ if target is null
    /// so that the unit test functions can call each other to build more
    /// complex tests.
    /// </summary>
    [TestMethod()]
    public void DwarfReaderImplConstructorTest() {
      // This function can be called from the beginning of any of the unit
      // tests in this file.
      if (null == target_) {
        target_ = new DwarfReaderImpl(testDb_);
      }
      Assert.AreNotEqual(null, target_);
    }

    #region private implementation
    private const ulong kDieOffset = 1024;
    private const ulong kCfiEntryAddress = 2048;


    private TestContext testContextInstance_;
    private SymbolDatabase testDb_;
    private DwarfReaderImpl target_;
    private ulong attributeCount_;

    /// <summary>
    /// Helper function to add a particular attribute and confirm that it was
    /// created.
    /// </summary>
    private void ProcessAttributeTest(ulong offset,
                                      ulong parent,
                                      DwarfAttribute attr,
                                      object data)
    {
      target_.ProcessAttribute(offset, parent, attr, data);

      var die = testDb_.Entries[offset];
      Assert.IsNotNull(die);
      Assert.AreEqual(die.Attributes[attr], data);

      SymbolDatabase.DebugInfoAttribute attribute =
        testDb_.Attributes[attributeCount_];
      Assert.IsNotNull(attribute);
      Assert.AreEqual(attribute.Tag, attr);
      ++attributeCount_;
    }

    private PrivateObject GetPrivates() {
      DwarfReaderImplConstructorTest();
      return new PrivateObject(target_);
    }
    #endregion
  }
}
