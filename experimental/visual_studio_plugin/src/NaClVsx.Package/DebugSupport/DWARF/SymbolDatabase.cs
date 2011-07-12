using System.Collections.Generic;
using System.Linq;
using NaClVsx;

namespace Google.NaClVsx.DebugSupport.DWARF {
  public class SymbolDatabase {
    #region  constants

    public const ulong InvalidKey = ulong.MaxValue;

    #endregion

    //
    // Tables
    //
    public Dictionary<ulong, SourceFile> Files {
      get { return files_; }
    }

    public Dictionary<ulong, DebugInfoEntry> Entries {
      get { return entries_; }
    }

    public Dictionary<ulong, DebugInfoAttribute> Attributes {
      get { return attributes_; }
    }

    public Dictionary<ulong, SourceLocation> Locations {
      get { return locations_; }
    }

    public Dictionary<ulong, ScopeTransition> ScopeTransitions {
      get { return scopeTransitions_; }
    }

    public Dictionary<ulong, List<LocListEntry>> LocLists {
      get { return locLists_; }
    }

    public Dictionary<ulong, CallFrame> CallFrames {
      get { return callFrames_; }
    }

    //
    // Indexed lookups
    //
    public Dictionary<string, List<SourceFile>> SourceFilesByFilename {
      get { return sourceFilesByFilename_; }
    }

    public Dictionary<DwarfAttribute, List<DebugInfoAttribute>> AttributesByTag {
      get { return attributesByTag_; }
    }

    public Dictionary<ulong, List<DebugInfoAttribute>> AttributesByEntry {
      get { return attributesByEntry_; }
    }

    public Dictionary<ulong, List<SourceLocation>> LocationsByFile {
      get { return locationsByFile_; }
    }

    public Dictionary<ulong, List<DebugInfoEntry>> EntriesByParent {
      get { return entriesByParent_; }
    }


    //
    // Query methods
    //
    public SourceLocation GetLocationForAddress(ulong address) {
      return GetRowForAddress(address, locations_, locationAddresses_);
    }

    public IEnumerable<SourceLocation> GetLocationsByLine(DebugInfoEntry scope) {
      IEnumerable<SourceLocation> locationsByLine = null;

      var lowPc = (ulong) scope.Attributes[DwarfAttribute.DW_AT_low_pc];
      if (scopeToLocationsByLine_.ContainsKey(lowPc)) {
        locationsByLine = scopeToLocationsByLine_[lowPc];
      }
      return locationsByLine;
    }

    public DebugInfoEntry GetScopeForAddress(ulong address) {
      return
          GetRowForAddress(
              address, scopeTransitions_, scopeTransitionAddresses_).Entry;
    }

    public CallFrame GetCallFrameForAddress(ulong address) {
      return GetRowForAddress(address, callFrames_, callFrameAddresses_);
    }

    public IEnumerable<DebugInfoEntry> GetChildrenForEntry(ulong entryKey) {
      List<DebugInfoEntry> result;
      if (!entriesByParent_.TryGetValue(entryKey, out result)) {
        result = new List<DebugInfoEntry>();
      }
      return result;
    }

    //
    // Other methods
    //
    public void BuildIndices() {
      BuildIndex(entries_, entriesByParent_, r => r.ParentKey);
      BuildIndex(files_, sourceFilesByFilename_, r => r.Filename);
      BuildIndex(attributes_, attributesByTag_, r => r.Tag);
      BuildIndex(attributes_, attributesByEntry_, r => r.ParentKey);
      BuildIndex(locations_, locationsByFile_, r => r.SourceFileKey);

      locationAddresses_.AddRange(locations_.Keys);
      locationAddresses_.Sort();

      scopeTransitionAddresses_.AddRange(scopeTransitions_.Keys);
      scopeTransitionAddresses_.Sort();

      callFrameAddresses_.AddRange(callFrames_.Keys);
      callFrameAddresses_.Sort();

      var scopeToLocations =
          new Dictionary<ulong, List<SourceLocation>>();

      foreach (var sourceLocation in locations_) {
        // TODO(mlinck) build up scope to locations
        // now also add the location to the scope that contains it
        DebugInfoEntry scope = GetScopeForAddress(sourceLocation.Key);
        if (scope != null) {
          var scopeStart = (ulong) scope.Attributes[DwarfAttribute.DW_AT_low_pc];
          if (!scopeToLocations.ContainsKey(scopeStart)) {
            scopeToLocations.Add(scopeStart, new List<SourceLocation>());
          }
          scopeToLocations[scopeStart].Add(sourceLocation.Value);
        }
      }

      foreach (var entry in scopeToLocations) {
        if (!scopeToLocationsByLine_.ContainsKey(entry.Key)) {
          scopeToLocationsByLine_.Add(entry.Key, null);
        }
        scopeToLocationsByLine_[entry.Key] = entry.Value.OrderBy(r => r.Line);
      }
    }

    #region Private Implementation

    private readonly Dictionary<ulong, List<DebugInfoAttribute>>
        attributesByEntry_ = new Dictionary<ulong, List<DebugInfoAttribute>>();

    private readonly Dictionary<DwarfAttribute, List<DebugInfoAttribute>>
        attributesByTag_ =
            new Dictionary<DwarfAttribute, List<DebugInfoAttribute>>();

    private readonly Dictionary<ulong, DebugInfoAttribute> attributes_ =
        new Dictionary<ulong, DebugInfoAttribute>();

    private readonly List<ulong> callFrameAddresses_ = new List<ulong>();

    private readonly Dictionary<ulong, CallFrame> callFrames_ =
        new Dictionary<ulong, CallFrame>();

    //
    // Indices
    //
    private readonly Dictionary<ulong, List<DebugInfoEntry>> entriesByParent_ =
        new Dictionary<ulong, List<DebugInfoEntry>>();

    private readonly Dictionary<ulong, DebugInfoEntry> entries_ =
        new Dictionary<ulong, DebugInfoEntry>();

    private readonly Dictionary<ulong, SourceFile> files_ =
        new Dictionary<ulong, SourceFile>();

    private readonly Dictionary<ulong, List<LocListEntry>> locLists_ =
        new Dictionary<ulong, List<LocListEntry>>();

    private readonly List<ulong> locationAddresses_ = new List<ulong>();

    private readonly Dictionary<ulong, List<SourceLocation>> locationsByFile_ =
        new Dictionary<ulong, List<SourceLocation>>();

    private readonly Dictionary<ulong, SourceLocation> locations_ =
        new Dictionary<ulong, SourceLocation>();

    private readonly Dictionary<ulong, IEnumerable<SourceLocation>>
        scopeToLocationsByLine_ =
            new Dictionary<ulong, IEnumerable<SourceLocation>>();

    // Sorted address lists
    private readonly List<ulong> scopeTransitionAddresses_ = new List<ulong>();

    private readonly Dictionary<ulong, ScopeTransition> scopeTransitions_ =
        new Dictionary<ulong, ScopeTransition>();

    private readonly Dictionary<string, List<SourceFile>> sourceFilesByFilename_
        = new Dictionary<string, List<SourceFile>>();

    #endregion

    #region Private Implementation

    private static void BuildIndex<TRow, TColumn>(
        IEnumerable<KeyValuePair<ulong, TRow>> table,
        IDictionary<TColumn, List<TRow>> index,
        FieldAccessor<TRow, TColumn> accessor) {
      // Clear out the index so we can rebuild it
      index.Clear();

      // Iterate over each row in the table and insert it
      // into the appropriate entry in the index.
      foreach (var pair in table) {
        List<TRow> indexList;
        TColumn columnValue = accessor(pair.Value);

        // If the index doesn't yet have a list for this value,
        // add one.
        if (!index.TryGetValue(columnValue, out indexList)) {
          indexList = new List<TRow>();
          index.Add(columnValue, indexList);
        }
        indexList.Add(pair.Value);
      }
    }

    private static T GetRowForAddress<T>(ulong address,
                                         IDictionary<ulong, T> table,
                                         List<ulong> index) where T : class {
      T result = null;

      int pos = index.BinarySearch(address);
      if (pos < 0) {
        // a negative value means there was no exact match. The
        // value is the two's complement of the next largest match
        // (which is not what we want; we want the next smaller match)
        pos = (~pos) - 1;
      }
      if (pos >= 0) {
        ulong closestAddress = index[pos];
        result = table[closestAddress];
      }
      return result;
    }

    #endregion

    #region Private Implementation

    private delegate TColumn FieldAccessor<TRow, TColumn>(TRow row);

    #endregion

    #region Nested type: CallFrame

    public class CallFrame {
      public ulong Address; //use as key

      public List<Rule> Rules = new List<Rule>();

      #region Nested type: Rule

      public class Rule {
        public ulong Address;
        public int RegisterId;

        public IDwarfReader.CfiRuleType RuleType;
        public int BaseRegister;
        public int Offset;
        public byte[] Expression;
      }

      #endregion
    }

    #endregion

    #region Nested type: DebugInfoAttribute

    public class DebugInfoAttribute {
      // Offset from beginning of dwarf section.
      public ulong Key;
      // Key of the DebugInfoEntry that this attribute helps describe.
      public ulong ParentKey;
      public DwarfAttribute Tag;
      public object Value;
    }

    #endregion

    #region Nested type: DebugInfoEntry

    public class DebugInfoEntry {
      public ulong Key;
      public DwarfTag Tag;
      public ulong ParentKey;
      public DebugInfoEntry OuterScope;
      public Dictionary<DwarfAttribute, object> Attributes =
          new Dictionary<DwarfAttribute, object>();
    }

    #endregion

    #region Nested type: LocListEntry

    public class LocListEntry {
      public ulong StartAddress;
      public ulong EndAddress;
      public byte[] Data;
    }

    #endregion

    #region Nested type: ScopeTransition

    public class ScopeTransition {
      public ulong Address;
      public DebugInfoEntry Entry;
    }

    #endregion

    #region Nested type: SourceFile

    public class SourceFile {
      public ulong Key;
      public string Filename;
      public string RelativePath;
      public string CurrentAbsolutePath;
    }

    #endregion

    #region Nested type: SourceLocation

    public class SourceLocation {
      public ulong StartAddress; // Use as key, since it is unique
      public ulong Length;

      public ulong SourceFileKey;
      public uint Line;
      public uint Column;
    }

    #endregion
  }
}
