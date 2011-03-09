// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.NaClVsx.DebugSupport.DWARF;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using NaClVsx;

namespace Google.NaClVsx.DebugSupport {
  /// <summary>
  /// Provides address and code look-ups specific to breakpoint operations.
  /// </summary>
  public class BreakpointInfo : IBreakpointInfo {
    public BreakpointInfo(SymbolDatabase database,
                          ISimpleSymbolProvider symbolProvider) {
      database_ = database;
      symbolProvider_ = symbolProvider;
    }

    #region IBreakpointInfo Members

    public int GetBindErrors(DocumentPosition pos,
                             out IEnumDebugErrorBreakpoints2 ppErrorEnum) {
      IEnumerable<ulong> addresses;
      return GetBindAddresses(pos, out addresses, out ppErrorEnum);
    }

    /// <summary>
    /// This function exist separately from AddressFromPosition because the 
    /// debugger needs different rules for matching addresses.  If no address
    /// is found that matches "position" exactly, this function will return
    /// addresses for the next appropriate position if there is one.
    /// </summary>
    /// <param name="position">The position in the source code where the user is trying to
    /// set a breakpoint.</param>
    /// <param name="outAddresses">Any memory addresses that can be appropriately mapped to
    /// position</param>
    /// <param name="outErrorEnum">Any errors encountered while this breakpoint was being set.
    /// </param>
    /// <returns>S_OK if the breakpoint was set successfully at the exact location requested, 
    /// else, S_FALSE.</returns>
    public int GetBindAddresses(DocumentPosition position,
                                out IEnumerable<ulong> outAddresses,
                                out IEnumDebugErrorBreakpoints2 outErrorEnum) {
      int addressesBound = VSConstants.S_FALSE;
      outAddresses = new List<ulong>();
      var addresses = outAddresses as List<ulong>;
      outErrorEnum = new ErrorBreakpointEnumerator();
      var breakpointErrorEnum = outErrorEnum as ErrorBreakpointEnumerator;

      string fileName = GetFileName(position);
      if (fileName != null) {
        List<SymbolDatabase.SourceFile> files =
            database_.SourceFilesByFilename[fileName];

        if (files.Count() >= 1) {
          // Remember the path that was passed in, since that's where
          // the current debugging session knows where to find this
          // file
          files[0].CurrentAbsolutePath = position.Path;

          // Internally, MSVC uses zero-based lines. This is in contrast
          // to both DWARF and MSVC's own user interface, but whatever.
          uint line = position.BeginPos.dwLine + 1;


          IEnumerable<SymbolDatabase.SourceLocation> locations =
              GetBreakpointLocations(files, line, position);

          foreach (SymbolDatabase.SourceLocation loc in locations) {
            ulong address = loc.StartAddress + symbolProvider_.GetBaseAddress();
            addresses.Add(address);

            var documentContext = new DocumentContext(
                position.Path, loc.Line - 1, loc.Column);

            // The check decides whether we move the breakpoint to compensate for user
            // error. We have to get the addresses first because they are needed for the
            // code context part of the error.
            if (loc.Line != line) {
              var resolution = new ErrorBreakpointResolution();
              resolution.Type = enum_BP_ERROR_TYPE.BPET_SEV_LOW |
                                enum_BP_ERROR_TYPE.BPET_TYPE_WARNING;

              resolution.SetLocation(
                  enum_BP_TYPE.BPT_CODE, address, documentContext);
              resolution.Message = "Had to move breakpoint to a different line.";

              var breakpointError = new ErrorBreakpoint(resolution);
              breakpointErrorEnum.Insert(breakpointError);
            }
          }

          uint count;
          int getCountSuccess = outErrorEnum.GetCount(out count);
          if (getCountSuccess == VSConstants.S_OK && count > 0) {
            addressesBound = 2;
          } else {
            addressesBound = VSConstants.S_OK;
          }
        }
      }

      // There are any number of reasons why this could have failed completely
      if (addresses.Count() == 0) {
        // TODO(mlinck) construct a severe error for this breakpoint.
        var resolution = new ErrorBreakpointResolution();
        resolution.Type = enum_BP_ERROR_TYPE.BPET_GENERAL_ERROR;
        resolution.Message = "Could not set a breakpoint at the requested location.";
        var error = new ErrorBreakpoint(resolution);
        breakpointErrorEnum.Insert(error);
        addressesBound = VSConstants.S_FALSE;
      }
      return addressesBound;
    }

    #endregion

    #region Private Implementation

    private readonly SymbolDatabase database_;
    private readonly ISimpleSymbolProvider symbolProvider_;

    #endregion

    #region Private Implementation

    private IEnumerable<SymbolDatabase.SourceLocation> GetBreakpointLocations(
        IEnumerable<SymbolDatabase.SourceFile> files,
        uint line,
        DocumentPosition position) {
      IEnumerable<SymbolDatabase.SourceLocation> finalLocations;

      // This will match any lines of code in a compatible scope at or after the 
      // line where the breakpoint was requested.
      IEnumerable<SymbolDatabase.SourceLocation> locationGuesses =
          from loc in database_.LocationsByFile[files.First().Key]
          where IsSuitableBreakpoint(loc, line, position)
          select loc;

      if (locationGuesses.Count() > 0) {
        IEnumerable<SymbolDatabase.SourceLocation> locationGuessesByLine =
            locationGuesses.OrderBy(r => r.Line);
        // We never need locations for more than one line of source code.       
        finalLocations = from loc in locationGuessesByLine
                         where loc.Line == locationGuessesByLine.First().Line
                         select loc;
      } else {
        finalLocations = new List<SymbolDatabase.SourceLocation>();
      }

      return finalLocations;
    }

    private string GetFileName(DocumentPosition position) {
      string rValue = null;
      string fileName = Path.GetFileName(position.Path);
      if (fileName != null) {
        if (database_.SourceFilesByFilename.ContainsKey(fileName)) {
          rValue = fileName;
        }
      }
      return rValue;
    }

    // This function checks whether a given |location| would make a 
    // suitable breakpoint for a |originalLine| of code the user requested.
    // In order to qualify the location must be either equal to or greater
    // than the requested line of code and must be in the same scope.
    // The caller of this function will then have to resolve the suitable
    // location that is closest to what the user requested.
    private bool IsSuitableBreakpoint(SymbolDatabase.SourceLocation location,
                                      uint originalLine,
                                      DocumentPosition originalPosition) {
      bool suitableBreakpoint = false;

      if (location.Line >= originalLine) {
        // This could be suitable breakpoint if the current location's
        // line is in the same scope as the original line.
        // TODO(mlinck) Does this address need a base offset?
        SymbolDatabase.DebugInfoEntry scope =
            database_.GetScopeForAddress(location.StartAddress);

        IEnumerable<SymbolDatabase.SourceLocation> sortedLocationsInScope =
            database_.GetLocationsByLine(scope);
        if (sortedLocationsInScope.First().Line <= originalLine) {
          suitableBreakpoint = true;
        } else if (scope.Tag == DwarfTag.DW_TAG_catch_block ||
                   scope.Tag == DwarfTag.DW_TAG_try_block ||
                   scope.Tag == DwarfTag.DW_TAG_lexical_block) {
          // This could still be a suitable breakpoint if this scope entry
          // is for a subscope of a function that begins before the 
          // original line.
          SymbolDatabase.DebugInfoEntry outerScope = scope.OuterScope;

          IEnumerable<SymbolDatabase.SourceLocation> sortedLocationsInOuterScope
              =
              database_.GetLocationsByLine(outerScope);
          if (sortedLocationsInOuterScope.First().Line <= originalLine) {
            suitableBreakpoint = true;
          }
        }
      }
      return suitableBreakpoint;
    }

    #endregion
  }
}
