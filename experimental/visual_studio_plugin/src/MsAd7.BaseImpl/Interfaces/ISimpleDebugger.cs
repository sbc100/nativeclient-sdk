// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.Collections.Generic;

namespace Google.MsAd7.BaseImpl.Interfaces {
  public struct SimpleDebuggerTypes {
    #region Delegates

    public delegate void EventHandler(
        ISimpleDebugger sender, EventType t, ResultCode status);

    public delegate void MessageHandler(
        ISimpleDebugger sender, ResultCode status, string msg);

    public delegate void ModuleLoadHandler(
        ISimpleDebugger sender, string modulePath, string status);

    #endregion

    #region EventType enum

    public enum EventType {
      Break,
      Continue,
      Step,
    }

    #endregion

    #region ResultCode enum

    public enum ResultCode {
      Busy = -4, // Target is busy (running)
      Failed = -3, // Transaction completed with failure
      Lost = -2, // Lost connection during transaction
      Timeout = -1, // Transaction Timed out
      Pending = 0, // Transaction is pending as expected
      Ok = 1 // Transaction Succeeded
    } ;

    #endregion
  }

  public interface ISimpleDebugger {
    //
    // Properties
    //
    ISimpleSymbolProvider Symbols { get; }
    string Architecture { get; }
    ulong BaseAddress { get; }

    //
    // Events
    //

    //
    // Process Control
    //
    void Break();
    void Step(uint id);
    void Continue();

    void AddBreakpoint(UInt64 addr);
    void RemoveBreakpoint(UInt64 addr);

    //
    // Queries
    //
    IEnumerable<UInt32> GetThreads();
    object GetRegisters(uint id);
    void GetMemory(UInt64 sourceAddress, Array destination, UInt32 countInBytes);
    void SetMemory(UInt64 destAddress, Array src, UInt32 countInBytes);
    ulong GetU64(ulong address);
    uint GetU32(ulong address);
    event SimpleDebuggerTypes.EventHandler Stopped;
    event SimpleDebuggerTypes.EventHandler StepFinished;
    event SimpleDebuggerTypes.EventHandler Continuing;
    event SimpleDebuggerTypes.MessageHandler Output;

    event SimpleDebuggerTypes.ModuleLoadHandler ModuleLoaded;
  }
}
