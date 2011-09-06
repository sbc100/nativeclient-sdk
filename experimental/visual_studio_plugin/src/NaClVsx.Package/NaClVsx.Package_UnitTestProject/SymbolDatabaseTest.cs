// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using System.Linq;
using Google.NaClVsx.DebugSupport.DWARF;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  ///<summary>
  ///  This is a test class for SymbolDatabaseTest and is intended to contain all
  ///  SymbolDatabaseTest Unit Tests.  Note that it is not possible to build these tests on a
  ///  compiled nexe, like the NaClSymbolProvider test suite because these all deal in
  ///  addresses and we can't predict how address information might change on a recompilation
  ///  of loop.cc or a change in the toolchain.
  ///  In order to make this test stable, all the addresses and constants in the test are made
  ///  up to test particular characteristics of the SymbolDatabase without interfering with each
  ///  other.  When modifying these tests, care must be taken to choose offsets and values that
  ///  are not already in use.
  ///</summary>
  [TestClass]
  public class SymbolDatabaseTest {
    ///<summary>
    ///  Gets or sets the test context which provides information about and functionality for the
    ///  current test run.
    ///</summary>
    public TestContext TestContext { get; set; }

    #region Additional test attributes

    //Use TestInitialize to run code before running each test
    [TestInitialize]
    public void MyTestInitialize() {
      target_ = null;
    }

    #endregion

    ///<summary>
    ///  A test for SourceFilesByFilename
    ///</summary>
    [TestMethod]
    public void SourceFilesByFilenameTest() {
      SymbolDatabaseConstructorTest();
      AddTestFiles();
      target_.BuildIndices();
      Assert.AreEqual(3, target_.SourceFilesByFilename.Count);
      // File names are not enough to disambiguate between files.  Path should
      // be considered as well.  Once that is the case, the count below should
      // be 2, not 3.
      var fileEntries = target_.SourceFilesByFilename[kRecurringName];
      Assert.AreEqual(3, fileEntries.Count);
    }

    ///<summary>
    ///  A test for ScopeTransitions
    ///</summary>
    [TestMethod]
    public void ScopeTransitionsTest() {
      SymbolDatabaseConstructorTest();
      var scopeTransition = new SymbolDatabase.ScopeTransition {
          Address = 123456,
          Entry = null
      };
      TestDictionaryAccessor(
          scopeTransition.Address,
          scopeTransition,
          "ScopeTransitions");
    }

    ///<summary>
    ///  A test for LocLists
    ///</summary>
    [TestMethod]
    public void LocListsTest() {
      SymbolDatabaseConstructorTest();
      var locListEntry = new SymbolDatabase.LocListEntry {
          Data = null,
          StartAddress = 123456,
          EndAddress = 234567
      };
      var locList = new List<SymbolDatabase.LocListEntry> {locListEntry};
      TestDictionaryAccessor(locListEntry.StartAddress, locList, "LocLists");
    }

    /// <summary>
    ///   A simple test for the RangeLists accessor.
    /// </summary>
    [TestMethod]
    public void RangeListsTest() {
      SymbolDatabaseConstructorTest();
      var rangeListEntry = new RangeListEntry {
          BaseAddress = 123456,
          HighPC = 456,
          LowPC = 123,
          Offset = 12
      };
      var rangeList = new Dictionary<ulong, RangeListEntry> {{123, rangeListEntry}};
      TestDictionaryAccessor(rangeListEntry.Offset, rangeList, "RangeLists");
    }

    /// <summary>
    ///   A simple test of the RangeListsByDIE accessor.
    /// </summary>
    [TestMethod]
    public void RangeListsByDIETest() {
      SymbolDatabaseConstructorTest();
      AddTestDIEs();
      AddTestAttributes();
      AddTestRangeLists();
      AddTestScopeTransitions();
      target_.BuildIndices();
      Assert.AreEqual(2, target_.RangeListsByDIE.Count);
    }

    ///<summary>
    ///  A test for LocationsByFile
    ///</summary>
    [TestMethod]
    public void LocationsByFileTest() {
      SymbolDatabaseConstructorTest();
      AddTestLocations();
      // If you add locations to the DB, you also need to add scope transitions
      // or BuildIndices will fail.
      AddTestScopeTransitions();
      target_.BuildIndices();
      Assert.AreEqual(2, target_.LocationsByFile.Count);
      var locationEntries = target_.LocationsByFile[kRecurringLocationKey];
      Assert.AreEqual(3, locationEntries.Count);
    }

    ///<summary>
    ///  A test for Locations
    ///</summary>
    [TestMethod]
    public void LocationsTest() {
      SymbolDatabaseConstructorTest();
      var location = new SymbolDatabase.SourceLocation {
          StartAddress = 123456,
          Length = 1,
          SourceFileKey = 2,
          Line = 3,
          Column = 4
      };
      TestDictionaryAccessor(location.StartAddress, location, "Locations");
    }

    ///<summary>
    ///  A test for Files
    ///</summary>
    [TestMethod]
    public void FilesTest() {
      SymbolDatabaseConstructorTest();
      var file = new SymbolDatabase.SourceFile {
          Key = 1,
          Filename = "name",
          RelativePath = "/path",
          CurrentAbsolutePath = "drive:/long/path"
      };
      TestDictionaryAccessor(file.Key, file, "Files");
    }

    ///<summary>
    ///  A test for EntriesByParent
    ///</summary>
    [TestMethod]
    public void EntriesByParentTest() {
      SymbolDatabaseConstructorTest();
      AddTestDIEs();
      target_.BuildIndices();
      Assert.AreEqual(4, target_.EntriesByParent.Count);
      var childEntries = target_.EntriesByParent[parentDIE_.Key];
      Assert.AreEqual(9, childEntries.Count);
    }

    ///<summary>
    ///  A test for Entries
    ///</summary>
    [TestMethod]
    public void EntriesTest() {
      SymbolDatabaseConstructorTest();
      var entry = new DebugInfoEntry {
          Key = 1,
          OuterScope = null,
          ParentKey = 0,
          Tag = DwarfTag.DW_TAG_with_stmt
      };
      TestDictionaryAccessor(entry.Key, entry, "Entries");
    }

    ///<summary>
    ///  A test for CallFrames
    ///</summary>
    [TestMethod]
    public void CallFramesTest() {
      SymbolDatabaseConstructorTest();
      var callFrame = new SymbolDatabase.CallFrame {
          Address = 123456,
      };
      TestDictionaryAccessor(callFrame.Address, callFrame, "CallFrames");
    }

    ///<summary>
    ///  A test for Attributes. Simple test to ensure we can modify attributes.
    ///</summary>
    [TestMethod]
    public void AttributesTest() {
      SymbolDatabaseConstructorTest();
      var attribute = new SymbolDatabase.DebugInfoAttribute {
          Key = 1,
          ParentKey = 1,
          Tag = DwarfAttribute.DW_AT_trampoline,
          Value = null
      };
      TestDictionaryAccessor(1, attribute, "Attributes");
    }

    /// <summary>
    ///   A test for GetScopeAddress
    /// </summary>
    [TestMethod]
    public void GetScopeAddressTest() {
      SymbolDatabaseConstructorTest();
      AddTestScopeTransitions();
      target_.BuildIndices();
      // A small offset is added here to make sure that the function will find the address for
      // our scope, even if it does not fall on an exact scope entry point.
      var scopeAddress = target_.GetScopeAddress(kRecurringScopeKey + 5);
      Assert.AreEqual(kEncapsulatingScopeAddress, scopeAddress);
    }

    ///<summary>
    ///  A test for GetScopeForAddress
    ///</summary>
    [TestMethod]
    public void GetScopeForAddressTest() {
      SymbolDatabaseConstructorTest();
      AddTestScopeTransitions();
      target_.BuildIndices();
      // A small offset is added here to make sure that the function will find the address for
      // our scope, even if it does not fall on an exact scope entry point.
      var scopeForAddress = target_.GetScopeForAddress(kRecurringScopeKey + 5);
      Assert.AreEqual(
          kEncapsulatingScopeAddress,
          scopeForAddress.Attributes[DwarfAttribute.DW_AT_low_pc]);
    }

    ///<summary>
    ///  A test for GetLocationsByLine
    ///</summary>
    [TestMethod]
    public void GetLocationsByLineTest() {
      SymbolDatabaseConstructorTest();
      AddTestDIEs();
      AddTestLocations();
      AddTestScopeTransitions();
      target_.BuildIndices();
      var sourceLocationsForScope = target_.GetLocationsByLine(parentDIE_, 0);
      Assert.AreEqual(4, sourceLocationsForScope.Count());
    }

    [TestMethod]
    public void GetRangeForAddressTest() {
      SymbolDatabaseConstructorTest();
      AddTestDIEs();
      AddTestAttributes();
      AddTestLocations();
      AddTestRangeLists();
      AddTestScopeTransitions();

      target_.BuildIndices();
      var testAddress = kFirstRangeListLowPC + firstRangeListEntry_.BaseAddress;
      // A small offset is added here to make sure that the function will find the address for
      // our scope, even if it does not fall on an exact scope entry point.
      var result = target_.GetRangeForAddress(
          testAddress + 4, rangeListDie0_);
      Assert.IsNotNull(result);
      Assert.AreEqual(kFirstRangeListLowPC, result.LowPC);
    }

    /// <summary>
    ///   This test checks to ensure that the GetLocationsByLine function
    ///   works for DIEs with ranges instead of loclists.
    /// </summary>
    [TestMethod]
    public void GetLocationsByLineRangesTest() {
      SymbolDatabaseConstructorTest();
      AddTestDIEs();
      AddTestAttributes();
      AddTestLocations();
      AddTestRangeLists();
      AddTestScopeTransitions();

      target_.BuildIndices();
      var rangeListBaseAddress = target_.RangeLists[0][kFirstRangeListLowPC].BaseAddress +
                                 kFirstRangeListLowPC;
      var sourceLocationsForScope = target_.GetLocationsByLine
          (rangeListDie0_, rangeListBaseAddress);
      Assert.AreEqual(2, sourceLocationsForScope.Count());
    }

    ///<summary>
    ///  A test for GetLocationForAddress
    ///</summary>
    [TestMethod]
    public void GetLocationForAddressTest() {
      SymbolDatabaseConstructorTest();
      AddTestLocations();
      // If you add locations to the DB, you also need to add scope transitions
      // or BuildIndices will fail.
      AddTestScopeTransitions();
      target_.BuildIndices();
      var locationForAddress =
          target_.GetLocationForAddress(kRecurringLocationKey + 5);
      Assert.AreEqual(kRecurringLocationKey, locationForAddress.StartAddress);
    }

    /// <summary>
    ///   A test for GetChildrenForEntry
    /// </summary>
    [TestMethod]
    public void GetChildrenForEntryTest() {
      SymbolDatabaseConstructorTest();
      AddTestDIEs();
      target_.BuildIndices();
      var children = target_.GetChildrenForEntry(parentDIE_.Key);
      Assert.AreEqual(9, children.Count());
    }

    /// <summary>
    ///   A test for GetCallFrameForAddress
    /// </summary>
    [TestMethod]
    public void GetCallFrameForAddressTest() {
      SymbolDatabaseConstructorTest();
      var callFrameKeys = new List<ulong> {
          kRecurringCallFrameKey,
          kHigherRecurringCallFrameKey,
          1000
      };

      AddTestCallFrames(callFrameKeys);

      target_.BuildIndices();

      var frameForAddress = target_.GetCallFrameForAddress(5);
      Assert.AreEqual(kRecurringCallFrameKey, frameForAddress.Address);
      frameForAddress = target_.GetCallFrameForAddress(50);
      Assert.AreEqual(kHigherRecurringCallFrameKey, frameForAddress.Address);
    }

    ///<summary>
    ///  A test for BuildIndices,  This test makes some fairly simple
    ///  collections and then validates that BuildIndices creates the
    ///  expected associations between them.
    ///</summary>
    [TestMethod]
    public void BuildIndicesTest() {
      SymbolDatabaseConstructorTest();

      AddTestDIEs();
      AddTestFiles();
      AddTestAttributes();
      AddTestLocations();
      AddTestRangeLists();
      AddTestScopeTransitions();
      var callFrameKeys = new List<ulong> {
          kRecurringCallFrameKey,
          kHigherRecurringCallFrameKey,
          1000
      };

      AddTestCallFrames(callFrameKeys);

      target_.BuildIndices();
      ValidateIndices();
    }

    /// <summary>
    ///   A test for SymbolDatabase Constructor.  It creates a sample database
    ///   loading loop.nexe.
    /// </summary>
    [TestMethod]
    public void SymbolDatabaseConstructorTest() {
      if (null == target_) {
        target_ = new SymbolDatabase();
      }

      Assert.IsNotNull(target_);
    }

    // Constants that are used in more than one place for validation purposes
    // are declared here.

    #region Private Implementation

    const ulong kEncapsulatingScopeAddress = 123400;
    const ulong kFirstRangeListLowPC = 10;
    const ulong kHigherRecurringCallFrameKey = 10;
    const ulong kRecurringCallFrameKey = 1;
    const uint kRecurringLocationKey = 123456;
    const string kRecurringName = "name_1";
    const ulong kRecurringScopeKey = 123450;

    #endregion

    #region Private Implementation

    private readonly RangeListEntry firstRangeListEntry_ = new RangeListEntry {
        BaseAddress = 123456,
        HighPC = 20,
        LowPC = kFirstRangeListLowPC,
        Offset = 0
    };

    private readonly DebugInfoEntry parentDIE_ = GenerateDIE(
        290, null, 279, DwarfTag.DW_TAG_compile_unit);

    private readonly DebugInfoEntry rangeListDie0_ = GenerateDIE(
        400, null, 290, DwarfTag.DW_TAG_lexical_block);

    private readonly DebugInfoEntry rangeListDie1_ = GenerateDIE(
        420, null, 290, DwarfTag.DW_TAG_lexical_block);

    private SymbolDatabase target_;

    #endregion

    #region Private Implementation

    /// <summary>
    ///   Simple wrapper for creating a DebugInfoAttribute since the type lacks
    ///   a constructor.
    /// </summary>
    /// <param name = "key">A unique integer ID.</param>
    /// <param name = "parentKey">The parent's ID.</param>
    /// <param name = "tag">The tag, describing the purpose of the attribute.
    /// </param>
    /// <param name = "value">The attribute's value.</param>
    /// <returns>An instance of a DebugInfoAttribute</returns>
    private static SymbolDatabase.DebugInfoAttribute GenerateAttribute(
        ulong key,
        ulong parentKey,
        DwarfAttribute tag,
        Object value) {
      return new SymbolDatabase.DebugInfoAttribute {
          Key = key,
          ParentKey = parentKey,
          Tag = tag,
          Value = value
      };
    }

    /// <summary>
    ///   Simple wrapper for creating a CallFrame since the type lacks a
    ///   constructor.
    /// </summary>
    /// <param name = "address">The CallFrame's address.</param>
    /// <param name = "rules">The rules for generating a CallFrame on the current
    ///   platform.</param>
    /// <returns>An instance of a CallFrame.</returns>
    private static SymbolDatabase.CallFrame GenerateCallFrame(
        ulong address,
        List<SymbolDatabase.CallFrame.Rule> rules) {
      return new SymbolDatabase.CallFrame {
          Address = address,
          Rules = rules
      };
    }

    /// <summary>
    ///   Simple wrapper for creating a debug info entry since the aforementioned
    ///   lacks a constructor.
    /// </summary>
    /// <returns>An instance of a debug info entry.</returns>
    private static DebugInfoEntry GenerateDIE(
        ulong key,
        DebugInfoEntry scope,
        ulong parentKey,
        DwarfTag tag) {
      return new DebugInfoEntry {
          Key = key,
          OuterScope = scope,
          ParentKey = parentKey,
          Tag = tag
      };
    }

    /// <summary>
    ///   Simple wrapper for creating a SourceLocation since the type lacks a
    ///   constructor.
    /// </summary>
    /// <param name = "column">Column in the source file.</param>
    /// <param name = "length">Length of the thing being referenced in
    ///   number of characters.</param>
    /// <param name = "line">The line the file entry is on.</param>
    /// <param name = "sourceFileKey">The unique ID of the file.</param>
    /// <param name = "startAddress">The address in memory associated with the
    ///   source location</param>
    /// <returns></returns>
    private static SymbolDatabase.SourceLocation GenerateLocation(
        uint column,
        ulong length,
        uint line,
        ulong sourceFileKey,
        ulong startAddress) {
      return new SymbolDatabase.SourceLocation {
          Column = column,
          Length = length,
          Line = line,
          SourceFileKey = sourceFileKey,
          StartAddress = startAddress
      };
    }

    /// <summary>
    ///   Simple wrapper for creating a ScopeTransition since the type lacks a
    ///   constructor.
    /// </summary>
    /// <param name = "address">The address in memory associated with the scope
    ///   transition</param>
    /// <param name = "entry">The debug information entry that describes the
    ///   scope transition.</param>
    /// <returns></returns>
    private static SymbolDatabase.ScopeTransition GenerateScopeTransition(
        ulong address,
        DebugInfoEntry entry) {
      return new SymbolDatabase.ScopeTransition {
          Address = address,
          Entry = entry
      };
    }


    /// <summary>
    ///   Simple wrapper for creating a file entry since the type lacks a
    ///   constructor.
    /// </summary>
    /// <param name = "path">The path.</param>
    /// <param name = "fileName">The file name.</param>
    /// <param name = "key">The key.</param>
    /// <param name = "relativePath">The relative path.</param>
    /// <returns>An instance of a file entry</returns>
    private static SymbolDatabase.SourceFile GenerateSourceFile(
        string path,
        string fileName,
        ulong key,
        string relativePath) {
      return new SymbolDatabase.SourceFile {
          CurrentAbsolutePath = path,
          Filename = fileName,
          Key = key,
          RelativePath = relativePath
      };
    }

    /// <summary>
    ///   Validates an individual index to ensure that it is not empty.  Used to
    ///   ensure that BuildIndices() created all the secondary indices that it
    ///   was supposed to create.
    /// </summary>
    /// <typeparam name = "TValueType"></typeparam>
    /// <param name = "index"></param>
    private static void ValidateIndex<TValueType>(IEnumerable<TValueType> index) {
      Assert.IsNotNull(index);
      Assert.AreNotEqual(0, index.Count());
    }

    /// <summary>
    ///   Populates the "Attributes" list on the target_ SymbolDatabase.
    /// </summary>
    private void AddTestAttributes() {
      // Here we expect BuildIndices to build 4 lists of attributes grouped
      // by DwarfAttribute and 3 lists of attributes grouped by Parent Key.
      var attributeList = new List<SymbolDatabase.DebugInfoAttribute> {
          GenerateAttribute(1, 0, DwarfAttribute.DW_AT_start_scope, null),
          GenerateAttribute(2, 1, DwarfAttribute.DW_AT_stmt_list, null),
          GenerateAttribute(3, 1, DwarfAttribute.DW_AT_low_pc, null),
          GenerateAttribute(4, 1, DwarfAttribute.DW_AT_high_pc, null),
          GenerateAttribute(5, 0, DwarfAttribute.DW_AT_start_scope, null),
          GenerateAttribute(6, 5, DwarfAttribute.DW_AT_low_pc, null),
          GenerateAttribute(7, 5, DwarfAttribute.DW_AT_high_pc, null),
          GenerateAttribute(8, rangeListDie0_.Key, DwarfAttribute.DW_AT_ranges, (ulong) 0),
          GenerateAttribute(9, rangeListDie1_.Key, DwarfAttribute.DW_AT_ranges, (ulong) 12)
      };
      foreach (var attribute in attributeList) {
        if (target_.Entries.ContainsKey(attribute.ParentKey)) {
          target_.Entries[attribute.ParentKey].Attributes.Add(attribute.Tag, attribute.Value);
        }
        TestDictionaryAccessor(attribute.Key, attribute, "Attributes");
      }
    }

    /// <summary>
    ///   Populates the "CallFrames" list on the target_ SymbolDatabase.
    /// </summary>
    private void AddTestCallFrames(IEnumerable<ulong> callFrameKeys) {
      var callFrameList =
          callFrameKeys.Select(key => GenerateCallFrame(key, null)).ToList();
      foreach (var frame in callFrameList) {
        TestDictionaryAccessor(frame.Address, frame, "CallFrames");
      }
    }

    /// <summary>
    ///   Populates the "Entries" list on the target_ SymbolDatabase.
    /// </summary>
    private void AddTestDIEs() {
      // Add DIEs to be indexed by parent (these are a few values captured from a test run with a
      // lot of output enabled.)  The address values in this test are chosen to create a
      // parent-child relationship between die 290 and all the DIE's that follow it.  Like in real
      // life, DIE address occur in ascending order.  The specific numbers are unimportant.
      var dieList = new List<DebugInfoEntry> {
          GenerateDIE(11, null, 0, DwarfTag.DW_TAG_compile_unit),
          GenerateDIE(155, null, 144, DwarfTag.DW_TAG_compile_unit),
          parentDIE_,
          GenerateDIE(324, parentDIE_, 290, DwarfTag.DW_TAG_base_type),
          GenerateDIE(331, parentDIE_, 290, DwarfTag.DW_TAG_base_type),
          GenerateDIE(334, parentDIE_, 290, DwarfTag.DW_TAG_pointer_type),
          GenerateDIE(336, parentDIE_, 290, DwarfTag.DW_TAG_base_type),
          GenerateDIE(343, parentDIE_, 290, DwarfTag.DW_TAG_base_type),
          GenerateDIE(350, parentDIE_, 290, DwarfTag.DW_TAG_base_type),
          GenerateDIE(357, parentDIE_, 290, DwarfTag.DW_TAG_base_type),
          rangeListDie0_,
          rangeListDie1_,
      };
      foreach (var debugInfoEntry in dieList) {
        TestDictionaryAccessor(debugInfoEntry.Key, debugInfoEntry, "Entries");
      }
    }

    /// <summary>
    ///   Populates the "Files" list on the target_ SymbolDatabase.
    /// </summary>
    private void AddTestFiles() {
      // Add Files to be indexed by filename.
      // Filelist entries that are identical often ahve different IDs which is
      // why buildIndices checks the file names.
      // The last entry added illustrates that path is currently not
      // disambiguated when files are stored.
      var fileList = new List<SymbolDatabase.SourceFile> {
          GenerateSourceFile("path/1/", kRecurringName, 1111, "."),
          GenerateSourceFile("path/1/", "name_2", 1112, "."),
          GenerateSourceFile("path/1/", "name_3", 1113, "."),
          GenerateSourceFile("path/1/", kRecurringName, 1114, "."),
          GenerateSourceFile("path/2/", kRecurringName, 1115, ".")
      };
      foreach (var file in fileList) {
        TestDictionaryAccessor(file.Key, file, "Files");
      }
    }

    /// <summary>
    ///   Populates the "Locations" list on the target SymbolDatabase.
    /// </summary>
    private void AddTestLocations() {
      var locationList = new List<SymbolDatabase.SourceLocation> {
          GenerateLocation(1, 10, 1, kRecurringLocationKey, 123456),
          GenerateLocation(12, 1, 1, kRecurringLocationKey, 123466),
          GenerateLocation(13, 1, 1, kRecurringLocationKey, 123470),
          GenerateLocation(1, 4, 2, 123457, 123654)
      };
      foreach (var location in locationList) {
        TestDictionaryAccessor(location.StartAddress, location, "Locations");
      }
    }

    /// <summary>
    ///   Populates the "RangeLists" list on the target SymbolDatabase.
    /// </summary>
    private void AddTestRangeLists() {
      var rangeList0 = new Dictionary<ulong, RangeListEntry> {
          {kFirstRangeListLowPC, firstRangeListEntry_},
          {20, new RangeListEntry {BaseAddress = 123456, HighPC = 40, LowPC = 20, Offset = 0}},
          {40, new RangeListEntry {BaseAddress = 123456, HighPC = 80, LowPC = 40, Offset = 0}}
      };
      var rangeList1 = new Dictionary<ulong, RangeListEntry> {
          {81, new RangeListEntry {BaseAddress = 123456, HighPC = 102, LowPC = 81, Offset = 12}},
          {102, new RangeListEntry {BaseAddress = 123456, HighPC = 112, LowPC = 102, Offset = 12}},
          {112, new RangeListEntry {BaseAddress = 123456, HighPC = 132, LowPC = 112, Offset = 12}}
      };
      TestDictionaryAccessor(0, rangeList0, "RangeLists");
      TestDictionaryAccessor(12, rangeList1, "RangeLists");
    }

    /// <summary>
    ///   Populates the "ScopeTransitions" list on the target SymbolDatabase.  These are designed
    ///   to bracket the CodeLocations added by AddTestLocations.
    /// </summary>
    private void AddTestScopeTransitions() {
      parentDIE_.Attributes.Add(
          DwarfAttribute.DW_AT_low_pc,
          kEncapsulatingScopeAddress);
      var scopeTransitionList = new List<SymbolDatabase.ScopeTransition> {
          GenerateScopeTransition(kEncapsulatingScopeAddress, parentDIE_),
          GenerateScopeTransition(
              234567,
              GenerateDIE(234567, null, 0, DwarfTag.DW_TAG_compile_unit))
      };
      foreach (var scopeTransition in scopeTransitionList) {
        TestDictionaryAccessor(
            scopeTransition.Address,
            scopeTransition,
            "ScopeTransitions");
      }
    }

    /// <summary>
    ///   This function can be used to ensure that modifying the item returned
    ///   by a symbol database accessor actually changes the state of the
    ///   database.
    /// </summary>
    private void TestDictionaryAccessor<TEntryType>(ulong keyValue,
                                                    TEntryType testEntry,
                                                    String accessorName) {
      // We use PrivateObject to get call by name semantics.
      var privates = new PrivateObject(target_);
      var dictionary =
          privates.GetProperty(accessorName) as Dictionary<ulong, TEntryType>;
      Assert.IsNotNull(dictionary);
      var firstCount = dictionary.Count;
      dictionary.Add(keyValue, testEntry);
      var dictionaryRef2 =
          privates.GetProperty(accessorName) as Dictionary<ulong, TEntryType>;
      Assert.IsNotNull(dictionaryRef2);
      Assert.AreEqual(firstCount + 1, dictionaryRef2.Count);
    }

    /// <summary>
    ///   Runs a set of validations to ensure all that indices were built.
    /// </summary>
    private void ValidateIndices() {
      ValidateIndex(target_.SourceFilesByFilename);
      ValidateIndex(target_.LocationsByFile);
      ValidateIndex(target_.EntriesByParent);
      ValidateIndex(target_.RangeListsByDIE);
    }

    #endregion
  }
}
