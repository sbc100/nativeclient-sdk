// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

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

#endregion

namespace Google.NaClVsx.DebugSupport {
  /// <summary>
  ///   Provides address and code look-ups specific to breakpoint operations.
  /// </summary>
  public class BreakpointInfo : IBreakpointInfo {
    public BreakpointInfo(SymbolDatabase database,
                          ISimpleSymbolProvider symbolProvider) {
      database_ = database;
      symbolProvider_ = symbolProvider;
    }

    public const int kBindErrorWarning = 2;

    #region IBreakpointInfo Members

    /// <summary>
    ///   This function can be called to determine what errors would occur if
    ///   an attempt were made to bind a breakpoint at a given location.
    /// </summary>
    /// <param name = "pos">
    ///   The position at which a breakpoint is being considered.
    /// </param>
    /// <param name = "ppErrorEnum">
    ///   Any errors that might be encountered if an attempt was made to set a
    ///   breakpoint at the current location.
    /// </param>
    /// <returns>Whether or not the function successfully checked for errors.
    /// </returns>
    public int GetBindErrors(DocumentPosition pos,
                             out IEnumDebugErrorBreakpoints2 ppErrorEnum) {
      IEnumerable<ulong> addresses;
      return GetBindAddresses(pos, out addresses, out ppErrorEnum);
    }

    /// <summary>
    ///   This function exists separately from AddressFromPosition because the debugger needs
    ///   different rules for matching addresses.  If no address is found that matches "position"
    ///   exactly, this function will return addresses for the next appropriate position if there
    ///   is one.
    /// </summary>
    /// <param name = "msvcPosition">The position in the source code where the user is trying to
    ///   set a breakpoint.  Since this is supplied by msvc, it's 0-indexed</param>
    /// <param name = "outAddresses">Any memory addresses that can be appropriately mapped to
    ///   position</param>
    /// <param name = "outErrorEnum">Any errors encountered while this breakpoint was being set.
    /// </param>
    /// <returns>If the breakpoint was set successfully at the exact location requested, this
    ///   function returns VSConstants.S_OK.  If the breakpoint was set in a different location,
    ///   the function returns kBindErrorWarning.  If the breakpoint cannot be set at all, it
    ///   returns S_FALSE.</returns>
    public int GetBindAddresses(DocumentPosition msvcPosition,
                                out IEnumerable<ulong> outAddresses,
                                out IEnumDebugErrorBreakpoints2 outErrorEnum) {
      var addressesBound = VSConstants.S_FALSE;
      outAddresses = new List<ulong>();
      var addresses = outAddresses as List<ulong>;
      outErrorEnum = new ErrorBreakpointEnumerator();
      var breakpointErrorEnum = outErrorEnum as ErrorBreakpointEnumerator;

      var fileName = GetFileName(msvcPosition);
      if (fileName != null) {
        var files =
            database_.SourceFilesByFilename[fileName];

        if (files.Count() >= 1) {
          var file = files.First();
          // Remember the path that was passed in, since that's where
          // the current debugging session knows where to find this
          // file
          file.CurrentAbsolutePath = msvcPosition.Path;

          // Internally, MSVC uses zero-based lines. This is in contrast
          // to both DWARF and MSVC's own user interface, but whatever.
          var line =
              NaClSymbolProvider.GetDwarfLineIndex(msvcPosition.BeginPos.dwLine);
          var locations =
              GetBreakpointLocations(file, line);

          foreach (var loc in locations) {
            var address = loc.StartAddress + symbolProvider_.GetBaseAddress();
            addresses.Add(address);

            var documentContext = new DocumentContext(
                msvcPosition.Path,
                NaClSymbolProvider.GetMSVCLineIndex(loc.Line),
                loc.Column);

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
          var getCountSuccess = outErrorEnum.GetCount(out count);
          if (getCountSuccess == VSConstants.S_OK && count > 0) {
            addressesBound = kBindErrorWarning;
          } else {
            addressesBound = VSConstants.S_OK;
          }
        }
      }

      // There are any number of reasons why this could have failed completely
      if (addresses.Count() == 0) {
        var resolution = new ErrorBreakpointResolution();
        resolution.Type = enum_BP_ERROR_TYPE.BPET_GENERAL_ERROR;
        resolution.Message =
            "Could not set a breakpoint at the requested location.";
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

    /// <summary>
    ///   This function retrieves plausible code locations for the given
    ///   breakpoint information.
    /// </summary>
    /// <param name = "file"></param>
    /// <param name = "line"></param>
    /// <returns></returns>
    private IEnumerable<SymbolDatabase.SourceLocation> GetBreakpointLocations(
        SymbolDatabase.SourceFile file,
        uint line) {
      IEnumerable<SymbolDatabase.SourceLocation> finalLocations =
          new List<SymbolDatabase.SourceLocation>();

      // This will match any lines of code in a compatible scope at or after the 
      // line where the breakpoint was requested.
      var locationGuesses =
          new Dictionary<ulong, List<SymbolDatabase.SourceLocation>>();
      foreach (var loc in database_.LocationsByFile[file.Key]) {
        if (IsSuitableBreakpoint(loc, line)) {
          if (!locationGuesses.ContainsKey(loc.Line)) {
            locationGuesses.Add(
                loc.Line, new List<SymbolDatabase.SourceLocation>());
          }
          locationGuesses[loc.Line].Add(loc);
        }
      }

      if (locationGuesses.Count() > 0) {
        var locationLines = new List<ulong>();
        locationLines.AddRange(locationGuesses.Keys);
        locationLines.Sort();
        finalLocations = locationGuesses[locationLines.First()];
      }

      return finalLocations;
    }

    /// <summary>
    ///   Gets the name of hte file that contains |position|.
    /// </summary>
    /// <param name = "position">
    ///   The position whose file name is needed.
    /// </param>
    /// <returns>
    ///   Returns the name of the file that contains |position|.
    /// </returns>
    private string GetFileName(DocumentPosition position) {
      string rValue = null;
      var fileName = Path.GetFileName(position.Path);
      if (fileName != null) {
        if (database_.SourceFilesByFilename.ContainsKey(fileName)) {
          rValue = fileName;
        }
      }
      return rValue;
    }

    /// <summary>
    ///   This function checks whether a given |location| would make a 
    ///   suitable breakpoint for a |originalLine| of code the user requested.
    ///   In order to qualify the location must be either equal to or greater
    ///   than the requested line of code and must be in the same scope.
    ///   The caller of this function will then have to resolve the suitable
    ///   location that is closest to what the user requested.
    /// </summary>
    /// <param name = "location">
    ///   A source code location to examine as a possible breakpoint location.
    /// </param>
    /// <param name = "originalLine">
    ///   The line at which the user wants to set the breakpoint.
    /// </param>
    /// <returns>
    ///   |true| iff the location could serve as a breakpoint location.
    /// </returns>
    private bool IsSuitableBreakpoint(SymbolDatabase.SourceLocation location,
                                      uint originalLine) {
      var suitableBreakpoint = false;

      if (location.Line >= originalLine) {
        // This could be suitable breakpoint if the current location's
        // line is in the same scope as the original line.
        var scope =
            database_.GetScopeForAddress(location.StartAddress);

        var sortedLocationsInScope =
            database_.GetLocationsByLine(scope, location.StartAddress);
        if (sortedLocationsInScope.First().Line <= originalLine) {
          suitableBreakpoint = true;
        } else if (scope.Tag == DwarfTag.DW_TAG_catch_block ||
                   scope.Tag == DwarfTag.DW_TAG_try_block ||
                   scope.Tag == DwarfTag.DW_TAG_lexical_block) {
          // This could still be a suitable breakpoint if this scope entry
          // is for a subscope of a function that begins before the 
          // original line.
          var outerScope = scope.OuterScope;
          var sortedLocationsInOuterScope =
              database_.GetLocationsByLine(outerScope, location.StartAddress);
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
