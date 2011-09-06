// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using Google.NaClVsx.DebugSupport.DWARF;
using Microsoft.VisualStudio.TestTools.UnitTesting;

#endregion

namespace NaClVsx.Package_UnitTestProject {
  /// <summary>
  ///   This is a test class for DebugInfoEntryTest and is intended
  ///   to contain all DebugInfoEntryTest Unit Tests
  /// </summary>
  [TestClass]
  public class DebugInfoEntryTest {
    /// <summary>
    ///   Adds a few RangeList entries to a DIE.
    /// </summary>
    [TestMethod]
    public void AddRangeListEntryByAddressTest() {
      var target = new DebugInfoEntry();
      var rangeListEntry1 = new RangeListEntry();
      var rangeListEntry2 = new RangeListEntry();
      var redundantRangeListEntry = new RangeListEntry();
      target.AddRangeListEntryByAddress(1234, rangeListEntry1);
      target.AddRangeListEntryByAddress(4567, rangeListEntry2);
      target.AddRangeListEntryByAddress(1234, redundantRangeListEntry);
      Assert.AreEqual(2, target.RangeListsByAddress.Count);
    }

    /// <summary>
    ///   A test for GetLowPC
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
    ///   A test for GetFrameBase
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
    ///   A test for DebugInfoEntry Constructor
    /// </summary>
    [TestMethod]
    public void DebugInfoEntryConstructorTest() {
      var target = new DebugInfoEntry();
      Assert.IsNotNull(target);
    }

    /// <summary>
    ///   A test for GetNearestAncestorWithTag
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

    /// <summary>
    ///   A test for HasAttribute.  Checks both "true" and "false" cases.
    /// </summary>
    [TestMethod]
    public void HasAttributeTest() {
      var target = new DebugInfoEntry();
      const DwarfAttribute kAttribute = DwarfAttribute.DW_AT_ranges;
      Assert.IsFalse(target.HasAttribute(kAttribute));
      target.Attributes.Add(DwarfAttribute.DW_AT_ranges, 0);
      Assert.IsTrue(target.HasAttribute(kAttribute));
    }

    /// <summary>
    ///   A test for HasAsAncestor.  Checks the case where the ancestor does not exist, the case
    ///   where the ancestor is a direct parent, and the case where the ancestor is more distant.
    /// </summary>
    [TestMethod]
    public void HasAsAncestorTest() {
      var target = new DebugInfoEntry();
      const ulong kAncestorKey = 123456;
      Assert.IsFalse(target.HasAsAncestor(kAncestorKey));
      var ancestor = new DebugInfoEntry();
      ancestor.Key = kAncestorKey;
      target.ParentKey = kAncestorKey;
      target.OuterScope = ancestor;
      Assert.IsTrue(target.HasAsAncestor(kAncestorKey));
      var parent = new DebugInfoEntry();
      parent.OuterScope = ancestor;
      parent.ParentKey = kAncestorKey;
      target.OuterScope = parent;
      Assert.IsTrue(target.HasAsAncestor(kAncestorKey));
    }

    /// <summary>
    ///   A test for GetRangesOffset
    /// </summary>
    [TestMethod]
    public void GetRangesOffsetTest() {
      var target = new DebugInfoEntry();
      Assert.AreEqual(ulong.MaxValue, target.GetRangesOffset());
      const ulong kRangesOffset = 123456;
      target.Attributes.Add(DwarfAttribute.DW_AT_ranges, kRangesOffset);
      Assert.AreEqual(kRangesOffset, target.GetRangesOffset());
    }
  }
}
