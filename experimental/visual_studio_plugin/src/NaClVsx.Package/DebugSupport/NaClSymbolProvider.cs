// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.DebugProperties;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Google.NaClVsx.DebugSupport.DWARF;
using NaClVsx;

namespace Google.NaClVsx.DebugSupport {
  /// <summary>
  /// The NaClSymbolProvider provides generic address and code lookups.
  /// </summary>
  public class NaClSymbolProvider : ISimpleSymbolProvider {
    public NaClSymbolProvider(NaClDebugger dbg) {
      BaseAddress = 0;
      dbg_ = dbg;
    }

    public ulong BaseAddress { get; set; }

    #region Implementation of ISimpleSymbolProvider

    public IBreakpointInfo GetBreakpointInfo() {
      return new BreakpointInfo(db_, this);
    }

    public ulong GetBaseAddress() {
      return BaseAddress;
    }

    public IEnumerable<ulong> AddressesFromPosition(DocumentPosition pos) {
      string fname = Path.GetFileName(pos.Path);
      if (!db_.SourceFilesByFilename.ContainsKey(fname)) {
        return null;
      }

      List<SymbolDatabase.SourceFile> files = db_.SourceFilesByFilename[fname];

      if (files.Count() > 1) {
        // TODO(ilewis): disambiguate
      } else if (files.Count() == 0) {
        return null;
      }

      // Remember the path that was passed in, since that's where
      // the current debugging session knows where to find this
      // file
      files[0].CurrentAbsolutePath = pos.Path;

      // Internally, MSVC uses zero-based lines. This is in contrast
      // to both DWARF and MSVC's own user interface, but whatever.
      //
      uint line = pos.BeginPos.dwLine + 1;

      return from loc in db_.LocationsByFile[files.First().Key]
             where loc.Line == line
             select loc.StartAddress + BaseAddress;
    }


    public DocumentPosition PositionFromAddress(ulong address) {
      SymbolDatabase.SourceLocation loc =
          db_.GetLocationForAddress(address - BaseAddress);
      if (loc != null) {
        // The DWARF line is 1-based; switch to zero-based for MSVC's benefit.
        uint line = loc.Line - 1;
        return
            new DocumentPosition(
                db_.Files[loc.SourceFileKey].CurrentAbsolutePath, line);
      }
      return null;
    }

    public IEnumerable<UInt64> GetAddressesInScope(UInt64 programCounter) {
      var result = new List<UInt64>();
      programCounter -= BaseAddress;

      // Get the scope, current function, and frame pointer
      //
      DebugInfoEntry scopeEntry =
          db_.GetScopeForAddress(programCounter);
      if (scopeEntry == null) {
        return result;
      }


      // Find the parent most DIE which represents this function
      //
      DebugInfoEntry fnEntry = scopeEntry;
      while (fnEntry.Tag != DwarfTag.DW_TAG_subprogram
             && fnEntry.OuterScope != null) {
        fnEntry = fnEntry.OuterScope;
      }

      var functionLowPC =
          (ulong)
          fnEntry.Attributes.GetValueOrDefault(DwarfAttribute.DW_AT_low_pc, 0);

      var fnMax =
          (ulong)
          fnEntry.Attributes.GetValueOrDefault(DwarfAttribute.DW_AT_high_pc, 0);


      functionLowPC += BaseAddress;
      fnMax += BaseAddress;
      while (functionLowPC < fnMax) {
        result.Add(functionLowPC);
        functionLowPC = GetNextLocation(functionLowPC);
      }

      return result;
    }

    /// <summary>
    /// This function retrieves the DebugInformationEntries for symbols that 
    /// are in scope at a given instruction address.
    /// </summary>
    /// <param name="instructionAddress">The address for whose scope symbols
    /// are being requested.</param>
    /// <returns>A list of symbol descriptors for all the symbols that are in 
    /// scope at this location in the program.</returns>
    public IEnumerable<Symbol> GetSymbolsInScope(ulong instructionAddress) {
      var result = new List<Symbol>();
      // Adjust for the base of the untrusted code space.
      instructionAddress -= BaseAddress;

      // Get the scope, current function, and frame pointer.  The frame pointer
      // will be used to determine what source code location applies to our
      // current scope.
      var scopeEntry = db_.GetScopeForAddress(instructionAddress);
      if (scopeEntry == null) {
        return result;
      }
      var functionEntry =
          scopeEntry.GetNearestAncestorWithTag(DwarfTag.DW_TAG_subprogram);
      var functionFrameBase = functionEntry.GetFrameBase();

      // The code locations are given addresses relative to the compilation
      // unit that contains them, so we have to make sure we can come up with
      // that same offset.
      var compilationUnitEntry =
          functionEntry.GetNearestAncestorWithTag(DwarfTag.DW_TAG_compile_unit);
      var compilationUnitLowPC = compilationUnitEntry.GetLowPC();

      var codeAddress = instructionAddress - compilationUnitLowPC;
      // The VM inputs object handles feeding the VM whatever it asks for.
      // This VM will execute the DWARF state machine to do any necessary
      // low-level address calculations.
      var vmInputs = new VirtualMachineInputs(dbg_, 0);
      PrimeVMInputs(codeAddress, functionFrameBase, vmInputs);

      while (scopeEntry != null) {
        foreach (
            DebugInfoEntry entry in
                db_.GetChildrenForEntry(scopeEntry.Key)) {
          // The assumption here is that all useful symbols have a location and
          // a name.
          if (entry.Attributes.ContainsKey(DwarfAttribute.DW_AT_location)
              && entry.Attributes.ContainsKey(DwarfAttribute.DW_AT_name)) {
            var name = (string) entry.Attributes[DwarfAttribute.DW_AT_name];
            var loc = entry.Attributes[DwarfAttribute.DW_AT_location] as byte [];
            // Program counter will not be used by ResolveLocation in this case
            // because the loc we're handing in is always a byte array.
            ulong symbolAddr = ResolveLocation(loc, vmInputs);
            // store the symbolAddr and variable name. Note that symbolAddr is
            // relative to the base address of the NaCl app.  The base address
            // is something like 0xC00000000, but the base gets added to this
            // relative address (symbolAddr) later in functions like GetMemory
            // (located in NaClDebugger.cs).
            result.Add(
                new Symbol {
                    Key = entry.Key,
                    Name = name,
                    Offset = symbolAddr,
                    TypeOf = GetSymbolType(entry.Key)
                });
          }
        }
        scopeEntry = scopeEntry.OuterScope;
      }
      return result;
    }

    public Function FunctionFromAddress(ulong address) {
      var result = new Function {
          Id = 0,
          Name = "<unknown function>",
      };

      DebugInfoEntry scopeEntry =
          db_.GetScopeForAddress(address - BaseAddress);
      DebugInfoEntry fnEntry = scopeEntry;
      while (fnEntry != null &&
             fnEntry.Tag != DwarfTag.DW_TAG_subprogram) {
        fnEntry = fnEntry.OuterScope;
      }

      if (fnEntry != null) {
        result.Id = fnEntry.Key;
        result.Name =
            (string)
            fnEntry.Attributes[DwarfAttribute.DW_AT_name];
      }

      return result;
    }

    public FunctionDetails GetFunctionDetails(Function fn) {
      throw new NotImplementedException();
    }

    public bool LoadModule(string path, ulong loadOffset, out string status) {
      // TODO(ilewis): this should be per-module, not per-database (unless
      // we decide to make database a per-module thing too)
      BaseAddress = loadOffset;
      try {
        DwarfParser.DwarfParseElf(path, new DwarfReaderImpl(db_));
        db_.BuildIndices();
        status = "ELF/DWARF symbols loaded";
        return true;
      }
      catch (Exception e) {
        status = e.Message;
        return false;
      }
    }

    public ulong GetNextLocation(ulong addr) {
      SymbolDatabase.SourceLocation loc =
          db_.GetLocationForAddress(addr - BaseAddress);
      return loc.StartAddress + loc.Length + BaseAddress;
    }

    #endregion

    /// <summary>
    /// Uses a CallFrame record from the Symbol Database to apply a set of
    /// rules to the current registerset, in order to derive the register
    /// state for the next outer stack frame.
    /// </summary>
    /// <param name="currentRegisters"></param>
    /// <returns></returns>
    public RegisterSet GetPreviousFrameState(RegisterSet currentRegisters) {
      ulong rip = currentRegisters["RIP"];
      var pc = rip - BaseAddress;
      if (pc > BaseAddress) {
        return null;
      }
      SymbolDatabase.CallFrame frame =
        db_.GetCallFrameForAddress(rip - BaseAddress);
      if (frame == null) {
        return null;
      }

      var result = currentRegisters.Clone() as RegisterSet;
      var rules = new Dictionary<int, SymbolDatabase.CallFrame.Rule>();

      // Find all of the rules that apply at the current address.
      // Rules are stored in ascending order of address. If two
      // rules exist for the same register, then the one with the
      // highest address wins.
      foreach (SymbolDatabase.CallFrame.Rule rule in frame.Rules) {
        if (rule.Address > rip) {
          break;
        }
        rules[rule.RegisterId] = rule;
      }

      // First we will hit the CFA rule and calculate the value of the previous
      // Frame offset.  Once we have that, the rest of the registers are
      // updated using memory locations that are relative to the previous CFA.
      // TODO(mlinck): This seems to end up with stackframes that represent the
      // functions' return locations, not the call site.
      foreach (SymbolDatabase.CallFrame.Rule rule in rules.Values) {
        switch (rule.RuleType) {
          case IDwarfReader.CfiRuleType.Expression:
            throw new NotImplementedException();
          case IDwarfReader.CfiRuleType.Offset:
            var addr =
                (ulong) ((long) result[rule.BaseRegister] + rule.Offset);
            Debug.WriteLine(
                "SymbolProvider:: result[rule.BaseRegister]: " +
                String.Format(
                    "{0,4:X}",
                    result[rule.BaseRegister]) +
                " rule.Offset: " + rule.Offset +
                " addr: " + String.Format("{0,4:X}", addr) +
                " rule.Address: " + String.Format(
                    "{0,4:X}",
                    rule.Address) +
                " BaseAddress: " + String.Format(
                    "{0,4:X}",
                    BaseAddress));
            var newValue = dbg_.GetU64(addr - BaseAddress);
            result[rule.RegisterId] = newValue;
            break;
          case IDwarfReader.CfiRuleType.Register:
            result[rule.RegisterId] = result[rule.BaseRegister];
            break;
          case IDwarfReader.CfiRuleType.SameValue:
            // this rule seems to exist only to restore values that 
            // have gotten munged earlier.
            result[rule.RegisterId] = currentRegisters[rule.RegisterId];
            break;
          case IDwarfReader.CfiRuleType.Undefined:
            // do nothing 
            break;
          case IDwarfReader.CfiRuleType.ValExpression:
            throw new NotImplementedException();
          case IDwarfReader.CfiRuleType.ValOffset:
            var baseValue = (long) result[rule.BaseRegister];
            var offset = (long) rule.Offset;
            newValue = (ulong) (baseValue + offset);
            result[rule.RegisterId] = newValue;
            break;
          default:
            throw new IndexOutOfRangeException("Bad rule type for CFI");
        }
      }
      return result;
    }

    public SymbolType GetSymbolType(ulong key) {
      var result = new SymbolType {
          Key = 0,
          Name = "unknown",
          SizeOf = 0,
          TypeOf = BaseType.Unknown,
      };
      DebugInfoEntry symbolEntry;

      // If we do not find this symbol, then return unknown.
      if (!db_.Entries.TryGetValue(key, out symbolEntry)) {
        return result;
      }


      if (symbolEntry.Attributes.ContainsKey(DwarfAttribute.DW_AT_type)) {
        var val =
            (DwarfReference) symbolEntry.Attributes[DwarfAttribute.DW_AT_type];
        result.Key = val.offset;
      } else {
        return result;
      }
      result.Name = "";
      // Process the linked list of modifiers as right associative to the base type;
      while (symbolEntry.Attributes.ContainsKey(DwarfAttribute.DW_AT_type)) {
        var val =
            (DwarfReference) symbolEntry.Attributes[DwarfAttribute.DW_AT_type];

        if (!db_.Entries.TryGetValue(val.offset, out symbolEntry)) {
          result.Key = 0;
          result.Name = null;
          return result;
        }

        // Set the size of the symbol to the "right most" sized component such that
        // char *p is size 4 since '*' is size 4.
        if (symbolEntry.Attributes.ContainsKey(DwarfAttribute.DW_AT_byte_size)) {
          object attrib = symbolEntry.Attributes[DwarfAttribute.DW_AT_byte_size];
          if (0 == result.SizeOf) {
            result.SizeOf = (uint) (ulong) attrib;
          }
        }

        switch (symbolEntry.Tag) {
          case DwarfTag.DW_TAG_const_type:
            result.Name = " const" + result.Name;
            break;

          case DwarfTag.DW_TAG_pointer_type:
            result.Name = " *" + result.Name;
            if (BaseType.Unknown == result.TypeOf) {
              result.TypeOf = BaseType.Pointer;
            }
            break;

          case DwarfTag.DW_TAG_reference_type:
            result.Name = " &" + result.Name;
            break;

          case DwarfTag.DW_TAG_volatile_type:
            result.Name = " volatile" + result.Name;
            break;

          case DwarfTag.DW_TAG_restrict_type:
            result.Name = " restrict" + result.Name;
            break;

          case DwarfTag.DW_TAG_base_type:
            result.Name =
                (string) symbolEntry.Attributes[DwarfAttribute.DW_AT_name] +
                result.Name;
            if (BaseType.Unknown == result.TypeOf) {
              var enc =
                  (DwarfEncoding)
                  (ulong) symbolEntry.Attributes[DwarfAttribute.DW_AT_encoding];
              switch (enc) {
                case DwarfEncoding.DW_ATE_address:
                  result.TypeOf = BaseType.Pointer;
                  break;

                case DwarfEncoding.DW_ATE_boolean:
                  result.TypeOf = BaseType.Bool;
                  break;

                case DwarfEncoding.DW_ATE_signed_char:
                case DwarfEncoding.DW_ATE_unsigned_char:
                  result.TypeOf = BaseType.Char;
                  break;

                case DwarfEncoding.DW_ATE_signed:
                  result.TypeOf = BaseType.Int;
                  break;

                case DwarfEncoding.DW_ATE_unsigned:
                  result.TypeOf = BaseType.UInt;
                  break;

                case DwarfEncoding.DW_ATE_float:
                  result.TypeOf = BaseType.Float;
                  break;
              }
            }
            return result;

          case DwarfTag.DW_TAG_structure_type:
            result.Name =
                (string) symbolEntry.Attributes[DwarfAttribute.DW_AT_name] +
                result.Name;
            if (BaseType.Unknown == result.TypeOf) {
              result.TypeOf = BaseType.Struct;
            }
            break;

          case DwarfTag.DW_TAG_subroutine_type: {
            result.Name = " (" + result.Name.TrimStart(null) + ")(";
            foreach (
                DebugInfoEntry parms in
                    db_.GetChildrenForEntry(val.offset)) {
              if (parms.Tag == DwarfTag.DW_TAG_formal_parameter) {
                result.Name += GetSymbolType(parms.Key).Name + ", ";
              }
            }
            char[] trim = {',', ' '};
            result.Name = result.Name.TrimEnd(trim) + ")";
            break;
          }

          default:
            result.Name = "Parse Err";
            result.Key = 0;
            break;
        }
      }

      return result;
    }

    #region ISimpleSymbolProvider Members

    public String SymbolValueToString(ulong key, ArraySegment<Byte> arrBytes) {
      SymbolType result = GetSymbolType(key);
      byte[] bytes = arrBytes.Array;

      // If we don't have enough data
      if (bytes.Count() < result.SizeOf) {
        return "INVALID";
      }

      // Or if we were unable to determine the type
      if (result.TypeOf == BaseType.Unknown) {
        return "INVALID";
      }

      Int64 i64 = 0;
      double d64 = 0;

      try {
        switch (result.SizeOf) {
          case 1:
            // We don't need to convert to number first because the switch
            // statement below will convert byte[0] to string val.
            break;

          case 2:
            i64 = BitConverter.ToInt16(bytes, 0);
            break;

          case 4:
            i64 = BitConverter.ToInt32(bytes, 0);
            d64 = BitConverter.ToSingle(bytes, 0);
            break;

          case 8:
            i64 = BitConverter.ToInt64(bytes, 0);
            d64 = BitConverter.ToDouble(bytes, 0);
            break;
        }
      }
      catch (Exception e) {
        // Without a catch here, the VSX SDK catches the exception and we
        // just don't get a value back...  Printing helps us realize this
        // is happening (when we had a bad conversion for case 1 previously)
        // and also lets us set a breakpoint inside the catch.
        Debug.WriteLine("An exception was caught when calling BitConverter!" +
          e.ToString());
      }


      var typeInfo = result.TypeOf;
      Debug.WriteLine("typeOf: " + typeInfo + " size: " + result.SizeOf);
      switch (result.TypeOf) {
        case BaseType.Pointer:
          return i64.ToString("X");

        case BaseType.Bool:
          if (0 != i64) {
            return "True";
          } else {
            return "False";
          }

        case BaseType.Char:
          return bytes[0].ToString();

        case BaseType.Int:
          return i64.ToString();

        case BaseType.UInt:
          return ((UInt64) i64).ToString();

        case BaseType.Float:
          return d64.ToString();

        case BaseType.Struct:
          return "{" + result.Name + "}";
      }
      return "INVALID";
    }

    #endregion

    #region Private Implementation

    readonly SymbolDatabase db_ = new SymbolDatabase();
    private readonly NaClDebugger dbg_;

    #endregion

    #region Private Implementation

    /// <summary>
    /// Primes the inputs that will be handed to the DWARF VM, by giving it
    /// relevant context.
    /// </summary>
    /// <param name="codeAddress">The code address for which context will be
    /// needed.</param>
    /// <param name="functionFrameBase">The frame base of the function that
    /// contains the code address.</param>
    /// <param name="vmInputs">This parameter is modified.  It is primed with
    /// A framebase that is calculated for the current context so that future
    /// DWARF address calculations can be peformed.</param>
    private void PrimeVMInputs(ulong codeAddress,
                               ulong functionFrameBase,
                               VirtualMachineInputs vmInputs) {
      ulong frameBase = ulong.MaxValue;
      List<SymbolDatabase.LocListEntry> loclist = db_.LocLists[functionFrameBase];
      foreach (SymbolDatabase.LocListEntry locListEntry in loclist) {
        if (locListEntry.StartAddress == ulong.MaxValue) {
          frameBase = locListEntry.EndAddress;
          break;
        }
        // Try to determine whether this is the address of a symbol and
        // return the referenced address in memory.
        if (locListEntry.StartAddress <= codeAddress
            && codeAddress <= locListEntry.EndAddress) {
          frameBase = DwarfParser.DwarfParseVM(vmInputs, locListEntry.Data);
          break;
        }
      }

      if (frameBase < (ulong) DwarfOpcode.DW_OP_regX) {
        Debug.WriteLine("WARNING: Register vars not currently supported");
      }
      vmInputs.FrameBase = frameBase;
    }


    /// <summary>
    /// Calculates the address on the stack, of a given location in the
    /// instruction queue.
    /// </summary>
    /// <param name="loc">The location in the instruction space.</param>
    /// <param name="vm">The virtual machine that is to be used to calculate
    /// the stack address.  It must be primed with context.</param>
    /// <returns>The stack address corresponding to loc.</returns>
    private static ulong ResolveLocation(byte [] loc,
                                         VirtualMachineInputs vm) {
      if (vm == null) {
        throw new ArgumentNullException("vm");
      }
      // A byte[] location is a Dwarf VM program, not an actual location.
      // We need to run the VM and (possibly) supply inputs.
      var stackAddress = DwarfParser.DwarfParseVM(vm, loc);

      if (stackAddress < (ulong) DwarfOpcode.DW_OP_regX) {
        Debug.WriteLine("WARNING: Register vars not currently supported");
      }
      return stackAddress;
    }

    #endregion
  }
}
