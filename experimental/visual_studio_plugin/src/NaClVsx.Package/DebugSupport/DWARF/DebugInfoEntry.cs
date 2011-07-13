using System.Collections.Generic;
using NaClVsx;

namespace Google.NaClVsx.DebugSupport.DWARF {
  /// <summary>
  /// Represent a debug information entry.
  /// </summary>
  public class DebugInfoEntry {
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
      while (target.Tag != tag && target.OuterScope != null)
      {
        target = target.OuterScope;
      }
      if (target.Tag != tag)
      {
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
    /// Gets teh LowPC address if this DIE has one.
    /// </summary>
    /// <returns>The frame base address or 0 if this DIE doesn't have one.
    /// </returns>
    public ulong GetLowPC() {
      return (ulong) Attributes.GetValueOrDefault(
          DwarfAttribute.DW_AT_low_pc, (ulong) 0);
    }

    public ulong Key;
    public DwarfTag Tag;
    public ulong ParentKey;
    public DebugInfoEntry OuterScope;
    public Dictionary<DwarfAttribute, object> Attributes =
        new Dictionary<DwarfAttribute, object>();


  }
}