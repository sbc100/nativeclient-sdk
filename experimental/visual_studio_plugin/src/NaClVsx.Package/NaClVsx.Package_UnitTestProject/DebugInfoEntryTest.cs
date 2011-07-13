using Google.NaClVsx.DebugSupport.DWARF;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace NaClVsx.Package_UnitTestProject {
  /// <summary>
  /// This is a test class for DebugInfoEntryTest and is intended
  /// to contain all DebugInfoEntryTest Unit Tests
  /// </summary>
  [TestClass]
  public class DebugInfoEntryTest {
    /// <summary>
    /// A test for GetLowPC
    /// </summary>
    [TestMethod]
    public void GetLowPCTest() {
      var target = new DebugInfoEntry();
      ulong expected = 0;
      Assert.AreEqual(expected, target.GetLowPC());
      expected = 123456;
      target.Attributes.Add(DwarfAttribute.DW_AT_low_pc, expected);
      Assert.AreEqual(expected, target.GetLowPC());
    }

    /// <summary>
    /// A test for GetFrameBase
    /// </summary>
    [TestMethod]
    public void GetFrameBaseTest() {
      var target = new DebugInfoEntry();
      ulong expected = 0;
      Assert.AreEqual(expected, target.GetFrameBase());
      expected = 123456;
      target.Attributes.Add(DwarfAttribute.DW_AT_frame_base, expected);
      Assert.AreEqual(expected, target.GetFrameBase());
    }

    /// <summary>
    /// A test for DebugInfoEntry Constructor
    /// </summary>
    [TestMethod]
    public void DebugInfoEntryConstructorTest() {
      var target = new DebugInfoEntry();
      Assert.IsNotNull(target);
    }

    /// <summary>
    /// A test for GetNearestAncestorWithTag
    /// </summary>
    [TestMethod]
    public void GetNearestAncestorWithTagTest() {
      var target = new DebugInfoEntry();
      DebugInfoEntry expected = null;
      Assert.AreEqual(
          expected,
          target.GetNearestAncestorWithTag(DwarfTag.DW_TAG_subprogram));
      target.Tag = DwarfTag.DW_TAG_subprogram;
      expected = target;
      Assert.AreEqual(
          expected,
          target.GetNearestAncestorWithTag(DwarfTag.DW_TAG_subprogram));
      target.Tag = DwarfTag.DW_TAG_namelist_item;
      expected = new DebugInfoEntry();
      expected.Tag = DwarfTag.DW_TAG_subprogram;
      target.OuterScope = expected;
      Assert.AreEqual(
          expected,
          target.GetNearestAncestorWithTag(DwarfTag.DW_TAG_subprogram));
    }
  }
}
