// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System.Collections.Generic;
using NaClVsx;

namespace Google.NaClVsx.DebugSupport.DWARF {
  /// <summary>
  /// Represent a debug information entry.
  /// </summary>
  public class DebugInfoEntry {
    /// <summary>
    ///   Adds a RangeListEntry for a given address, provided the DIE doesn't already contain one
    ///   for that address.  
    /// </summary>
    /// <param name="address">The address.  Note that this list is expected to be the address at
    ///   runtime, not just the LowPC values from the rangeListEntry.</param>
    /// <param name="rangeListEntry">The RangeList entry to be added to this DIE.</param>
    public void AddRangeListEntryByAddress(ulong address, RangeListEntry rangeListEntry) {
      if (!RangeListsByAddress.ContainsKey(address)) {
        RangeListsByAddress.Add(address, rangeListEntry);
      }
    }

    /// <summary>
    /// Gets the nearest ancestor of this DIE with a requested DwarfTag.
    /// </summary>
    /// <param name="tag">The tag that must be matched.</param>
    /// <returns>The innermost DIE that has the requested DwarfTag.  If this
    /// DIE matches it returns itself.  If no ancestor matches, the function
    /// will return NULL.</returns>
    public DebugInfoEntry GetNearestAncestorWithTag(DwarfTag tag)
    {
      var target = this;
      while (target.Tag != tag && target.OuterScope != null) {
        target = target.OuterScope;
      }
      if (target.Tag != tag) {
        return null;
      }
      return target;
    }

    /// <summary>
    /// Gets the Frame Base address if this DIE has one.
    /// </summary>
    /// <returns>The frame base address or 0 if this DIE doesn't have one.
    /// </returns>
    public ulong GetFrameBase() {
      return (ulong) Attributes.GetValueOrDefault(
          DwarfAttribute.DW_AT_frame_base, (ulong) 0);
    }

    /// <summary>
    /// Gets the LowPC address if this DIE has one.
    /// </summary>
    /// <returns>The frame base address or 0 if this DIE doesn't have one.
    /// </returns>
    public ulong GetLowPC() {
      return (ulong) Attributes.GetValueOrDefault(
          DwarfAttribute.DW_AT_low_pc, (ulong) 0);
    }
    
    /// <summary>
    ///   Gets the offset of this DIE's ranges entry in.  The offset can be used as a key to
    ///   retrieve the ranges entry from the SymbolDatabase.
    /// </summary>
    /// <returns>The offset if this DIE has a ranges entry, ulong.MaxValue if it doesn't.
    ///   </returns>
    public ulong GetRangesOffset() {
      return (ulong) Attributes.GetValueOrDefault(
          DwarfAttribute.DW_AT_ranges, ulong.MaxValue);
    }

    /// <summary>
    ///   Determines whether this DebugInformationEntry has an ancestor whose key is
    ///   |ancestorKey|.
    /// </summary>
    /// <param name="ancestorKey">The key of the possible ancestor's entry in the symbol
    ///   database.</param>
    /// <returns>True iff this DIE has an ancestor with key |ancestorKey|.</returns>
    public bool HasAsAncestor(ulong ancestorKey) {
      var ancestor = OuterScope;
      while (ancestor != null) {
        if (ancestor.Key == ancestorKey ) {
          return true;
        }
        ancestor = ancestor.OuterScope;
      }
      return false;
    }

    /// <summary>
    ///   Checks whether this DIE has a given attribute.  Different DIEs have different
    ///   attributes.  For a complete list, see NaClVsx.DebugHelpers\DwarfParser.h
    /// </summary>
    /// <param name="attribute">The attribute to be checked.</param>
    /// <returns>True iff this DIE has the given attribute.</returns>
    public bool HasAttribute(DwarfAttribute attribute) {
      return Attributes.ContainsKey(attribute);
    }

    public ulong Key;
    public DwarfTag Tag;
    public ulong ParentKey;
    public DebugInfoEntry OuterScope;
    public Dictionary<DwarfAttribute, object> Attributes =
        new Dictionary<DwarfAttribute, object>();

    public Dictionary<ulong, RangeListEntry> RangeListsByAddress = 
      new Dictionary<ulong, RangeListEntry>();
  }
}