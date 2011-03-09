// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;

namespace Google.MsAd7.BaseImpl.Interfaces {
  namespace SimpleSymbolTypes {
    /// <summary>
    /// An enumeration of BaseTypes that this SymbolProvider currently supports.
    /// </summary>
    public enum BaseType {
      Unknown = 0,
      Pointer,
      Struct,
      Bool,
      Char,
      Int,
      UInt,
      Float
    } ;

    /// <summary>
    /// A symbol's type, as represented by ISimpleDebugger.
    /// </summary>
    public struct SymbolType {
      public ulong Key;
      public string Name;
      public uint SizeOf;
      public BaseType TypeOf;
    }

    /// <summary>
    /// A symbol, as represented by ISimpleDebugger.
    /// </summary>
    public struct Symbol {
      public ulong Key;
      public SymbolType TypeOf;
      public string Name;
      public UInt64 Offset;
    }

    /// <summary>
    /// A function, as represented by ISimpleDebugger.
    /// </summary>
    public struct Function {
      public string Name;
      public UInt64 Id;
    }

    /// <summary>
    /// A function's signature, as represented by ISimpleDebugger.
    /// </summary>
    public struct FunctionDetails {
      public SymbolType ReturnType;
      public Symbol[] Parameters;
      public Symbol[] Locals;
    }
  }

  /// <summary>
  /// A class may implement this interface to become a symbol provider for ISimpleDebugger.
  /// </summary>
  public interface ISimpleSymbolProvider {
    /// <summary>
    /// Converts code locations to memory addresses.
    /// </summary>
    /// <param name="pos">|pos| represents the location in the code.</param>
    /// <returns>A memory address if one can be found that exactly occurs at |pos|.</returns>
    IEnumerable<ulong> AddressesFromPosition(DocumentPosition pos);
    /// <summary>
    /// Converts memory addresses to code locations.
    /// </summary>
    /// <param name="address">The memory address.</param>
    /// <returns>A position in the code that corresponds to address.</returns>
    DocumentPosition PositionFromAddress(UInt64 address);
    /// <summary>
    /// Call this function to get a class that knows how to perform functions that are specific to
    /// the task of setting breakpoints.
    /// </summary>
    /// <returns>An implementation of IBreakpointInfo.</returns>
    IBreakpointInfo GetBreakpointInfo();
    /// <summary>
    /// Called to get the memory offset of the code module served by this
    /// ISimpleSymbolProvider.
    /// </summary>
    /// <returns>The memory offset for this ISimpleSymbolProvider's code module.</returns>
    ulong GetBaseAddress();

    /// <summary>
    /// All symbols in the same scope as |programCounter|.
    /// </summary>
    /// <param name="programCounter">The address of any entity in the program being debugged.
    /// </param>
    /// <returns>All symbols in the same scope as |programCounter|</returns>
    IEnumerable<Symbol> GetSymbolsInScope(UInt64 programCounter);
    /// <summary>
    /// All addresses in the same scope as |programCounter|.
    /// </summary>
    /// <param name="programCounter">The address of any entity in the program being debugged.
    /// </param>
    /// <returns>All addresses in the same scope as |programCounter|</returns>
    IEnumerable<UInt64> GetAddressesInScope(UInt64 programCounter);
    /// <summary>
    /// This can be used to turn symbol information into a human-readable representation.
    /// </summary>
    /// <param name="key">A symbol type's integral value.</param>
    /// <param name="arrBytes">The raw data contained by the symbol.</param>
    /// <returns>A human readable representation</returns>
    String SymbolValueToString(ulong key, ArraySegment<Byte> arrBytes);

    /// <summary>
    /// This can be used to look up a function's representation.
    /// </summary>
    /// <param name="address">The desired function's address.</param>
    /// <returns>The Function defined at |address|</returns>
    Function FunctionFromAddress(UInt64 address);
    /// <summary>
    /// This can be used to look up a function's signature.
    /// </summary>
    /// <param name="fn">The function whose signature is desired.</param>
    /// <returns>The signature of |fn|</returns>
    FunctionDetails GetFunctionDetails(Function fn);

    /// <summary>
    /// Loads a code module into this ISimpleSymbolProvider.
    /// </summary>
    /// <param name="path">The path to the code module.</param>
    /// <param name="loadOffset">The offset of the module's code in memory.</param>
    /// <param name="status">The status which is mostly interesting if the module failed to load.
    /// </param>
    /// <returns>true if and only if the module was loaded successfully</returns>
    bool LoadModule(string path, UInt64 loadOffset, out string status);
    /// <summary>
    /// Gets the next location after whatever is at |address| that corresponds to a valid address.
    /// </summary>
    /// <param name="addr">An address to start searching from</param>
    /// <returns>The next code location after |addr|</returns>
    UInt64 GetNextLocation(UInt64 addr);
  }
}
