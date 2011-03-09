using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl.Interfaces
{
  public struct SimpleDebuggerTypes {
    public enum ResultCode
    {
      Busy   = -4,   // Target is busy (running)
      Failed = -3,   // Transaction completed with failure
      Lost = -2,     // Lost connection during transaction
      Timeout =-1,   // Transaction Timed out
      Pending = 0,   // Transaction is pending as expected
      Ok = 1         // Transaction Succeeded
    };

    public enum EventType {
      Break,
      Continue,
      Step,
    }

    public delegate void EventHandler(ISimpleDebugger sender, EventType t, ResultCode status);

    public delegate void MessageHandler(
        ISimpleDebugger sender, ResultCode status, string msg);

    public delegate void ModuleLoadHandler(
        ISimpleDebugger sender, string modulePath, string status);
  }

  public interface ISimpleDebugger
  {
    //
    // Properties
    //
    ISimpleSymbolProvider Symbols { get; }
    string Architecture {get;}

    //
    // Events
    //
    event SimpleDebuggerTypes.EventHandler Stopped;
    event SimpleDebuggerTypes.EventHandler StepFinished;
    event SimpleDebuggerTypes.EventHandler Continuing;
    event SimpleDebuggerTypes.MessageHandler Output;

    event SimpleDebuggerTypes.ModuleLoadHandler ModuleLoaded;

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
  }
}
