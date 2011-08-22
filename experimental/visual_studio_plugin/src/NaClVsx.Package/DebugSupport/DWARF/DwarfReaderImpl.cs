// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using NaClVsx;

namespace Google.NaClVsx.DebugSupport.DWARF {
  /// <summary>
  /// The interface implemented by this class lives in
  /// NaClVsx.DebugHelpers->Header Files->DwarfParser.h
  /// </summary>
  class DwarfReaderImpl : IDwarfReader {
    public DwarfReaderImpl(SymbolDatabase db) {
      db_ = db;
    }

    #region Implementation of IDwarfReader

    public void StartCompilationUnit() {
      // File and directory ids are unique only within a compilation unit.
      dirs_.Clear();
      compilationUnitIndex_++;
    }

    public void EndCompilationUnit() {}

    public void StartDIE(ulong parent, ulong offset, DwarfTag tag) {
      var entry =
          new DebugInfoEntry {
              Key = offset,
              OuterScope = scopeStack_.PeekOrDefault(),
              ParentKey = parent,
              Tag = tag
          };

      db_.Entries[offset] = entry;
    }

    public void EndDIE(ulong offset) {
      DebugInfoEntry entry = db_.Entries[offset];

      // If this DIE is a scope, it should at this point be at the top of the
      // scope stack.
      if (scopeStack_.PeekOrDefault() == entry) {
        scopeStack_.Pop();

        // Add a transition back to the parent scope. This may get overwritten
        // if a sibling scope starts where this one ends, but that's cool.
        //
        // Note: we can only do this if the scope has a high pc value. Labels
        // don't.
        object highpc;
        if (entry.Attributes.TryGetValue(
            DwarfAttribute.DW_AT_high_pc, out highpc)) {
          if (db_.ScopeTransitions.ContainsKey((ulong) highpc)) {
            // This can happen if a parent and child scope end on the same pc value.
            // The parent wins, so overwrite the previous entry.
            db_.ScopeTransitions[(ulong) highpc].Entry = entry.OuterScope;
          } else {
            db_.ScopeTransitions.Add(
                (ulong) highpc,
                new SymbolDatabase.ScopeTransition {
                    Address = (ulong) highpc,
                    Entry = entry.OuterScope
                });
          }
        }
      }
    }

    public void ProcessAttribute(ulong offset,
                                 ulong parent,
                                 DwarfAttribute attr,
                                 object data) {
      DebugInfoEntry entry = db_.Entries[offset];

      ulong key = attributeIndex_++;
      db_.Attributes.Add(
          key,
          new SymbolDatabase.DebugInfoAttribute {
              Key = key,
              ParentKey = parent,
              Tag = attr,
              Value = data
          });
      entry.Attributes.Add(attr, data);


      // If we have a PC range, it's time to make a new scope
      //
      if (attr == DwarfAttribute.DW_AT_low_pc) {
        var addr = (ulong) data;

        // Add a scope transition for this entry.
        // Replace any existing scope transition at the point where  
        // this entry starts. Existing transitions are expected--it 
        // just means that this entry starts where its sibling ends.
        //
        if (db_.ScopeTransitions.ContainsKey(addr)) {
          db_.ScopeTransitions[addr].Entry = entry;
        } else {
          db_.ScopeTransitions.Add(
              addr,
              new SymbolDatabase.ScopeTransition {
                  Address = addr,
                  Entry = entry
              });
        }
        scopeStack_.Push(db_.Entries[parent]);
      }
    }

    public void DefineDir(string name, uint dirNum) {
      try {
        dirs_[dirNum] = name;
      }
      catch (Exception e) {
        Debug.WriteLine(e.Message);
      }
    }

    public void DefineFile(string name,
                           int fileNum,
                           uint dirNum) {
      ulong key = MakeFileKey((uint) fileNum);
      string relpath;
      if (!dirs_.TryGetValue(dirNum, out relpath)) {
        relpath = "";
      }
      db_.Files.Add(
          key,
          new SymbolDatabase.SourceFile {
              Key = key,
              Filename = name,
              RelativePath = relpath,
              CurrentAbsolutePath = name
          });
    }

    public void AddLine(ulong address,
                        ulong length,
                        uint fileNum,
                        uint lineNum,
                        uint columnNum) {
      //
      // There are several cases in the C++ standard library where a line entry
      // has zero length. This appears to happen when a line of code is
      // elided for whatever reason. The problem is that the address might be
      // perfectly valid for a different line of code, which would lead to
      // duplicate entries in our location table. So if length is zero, bail.
      //
      if (length == 0) {
        return;
      }

      //
      // Add the address and location to our locations table.
      //
      try {
        var
            sourceLocation = new SymbolDatabase.SourceLocation {
                StartAddress = address,
                Length = length,
                SourceFileKey = MakeFileKey(fileNum),
                Line = lineNum,
                Column = columnNum,
            };

        db_.Locations.Add(address, sourceLocation);
      }
      catch (Exception e) {
        Debug.WriteLine(e.Message);
      }
    }

    public void AddLocListEntry(ulong offset,
                                bool isFirstEntry,
                                ulong lowPc,
                                ulong highPc,
                                byte[] data) {
      if (isFirstEntry) {
        currentLocList_ = offset;
        db_.LocLists.Add(offset, new List<SymbolDatabase.LocListEntry>());
      }
      List<SymbolDatabase.LocListEntry> list = db_.LocLists[currentLocList_];
      list.Add(
          new SymbolDatabase.LocListEntry {
              StartAddress = lowPc,
              EndAddress = highPc,
              Data = data
          });
    }

    public bool BeginCfiEntry(ulong address) {
      currentFrame_ = new SymbolDatabase.CallFrame {Address = address};
      return true;
    }

    public bool AddCfiRule(ulong address,
                           int reg,
                           IDwarfReader.CfiRuleType ruleType,
                           int baseRegister,
                           int offset,
                           byte[] expression) {
      currentFrame_.Rules.Add(
          new SymbolDatabase.CallFrame.Rule {
              Address = address,
              BaseRegister = baseRegister,
              Expression = expression,
              Offset = offset,
              RegisterId = reg,
              RuleType = ruleType
          });
      return true;
    }

    public bool EndCfiEntry() {
      if (currentFrame_ == null) {
        throw new DwarfParseException("Mismatched begin/end CFI entries");
      }
      db_.CallFrames.Add(currentFrame_.Address, currentFrame_);

      currentFrame_ = null;
      return true;
    }

    public void AddRangeListEntry(ulong offset,
                                  ulong baseAddress,
                                  ulong lowPC,
                                  ulong highPC) {
      // TODO(mlinck) implement this function
    }

    private ulong MakeFileKey(uint fileNum) {
      return ((ulong) compilationUnitIndex_ << 32) | fileNum;
    }

    #endregion

    #region Private Implementation

    private readonly SymbolDatabase db_;

    private readonly Dictionary<uint, string> dirs_ =
        new Dictionary<uint, string>();

    private readonly Stack<DebugInfoEntry> scopeStack_ =
        new Stack<DebugInfoEntry>();

    private ulong attributeIndex_;
    private ushort compilationUnitIndex_;
    private SymbolDatabase.CallFrame currentFrame_;
    private ulong currentLocList_;

    #endregion
  }

  internal class DwarfParseException : Exception {
    public DwarfParseException(string msg) : base(msg) {}
  }
}
