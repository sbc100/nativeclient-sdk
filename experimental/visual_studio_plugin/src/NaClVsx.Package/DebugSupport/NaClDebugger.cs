// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Xml.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Interfaces;
using Microsoft.VisualStudio.Debugger.Interop;
using NaClVsx.DebugHelpers;

namespace Google.NaClVsx.DebugSupport {
  public sealed class NaClDebugger : ISimpleDebugger {
    public NaClDebugger(ulong baseAddress) {
      baseAddress_ = baseAddress;
      symbols_ = new NaClSymbolProvider(this);
    }

    public string Arch {
      get { return arch_; }
    }

    public string Path {
      get { return path_; }
    }

    public int GdbTimeout {
      get { return gdbTimeout_; }
      set { gdbTimeout_ = value; }
    }

    // Added from Ian's RSP branch
    public ulong BaseAddress
    {
      get { return baseAddress_; }
    }

    #region Implementation of ISimpleDebugger

    public string Architecture {
      get { return arch_; }
    }

    public event SimpleDebuggerTypes.EventHandler Stopped;
    public event SimpleDebuggerTypes.EventHandler StepFinished;
    public event SimpleDebuggerTypes.EventHandler Continuing;
    public event SimpleDebuggerTypes.MessageHandler Output;
    public event SimpleDebuggerTypes.ModuleLoadHandler ModuleLoaded;
    public event SimpleDebuggerTypes.MessageHandler Opened;


    public ISimpleSymbolProvider Symbols {
      get { return symbols_; }
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Break() {
      var result = gdb_.RequestBreak();
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public object GetRegisters(uint id)
    {
      // FIXME -- |id| does NOT appear to be used by this function!!  
      var regs = new RegsX86_64();
      gdb_.GetRegisters(ref regs);
      Debug.WriteLine(" GetRegisters.... Rip=" +
        String.Format("{0,4:X}", regs.Rip) +
        " Rsp=" + String.Format("{0,4:X}", regs.Rsp) +
        " SegCS=" + String.Format("{0,4:X}", regs.SegCs) +
        " SegDS=" + String.Format("{0,4:X}", regs.SegDs) +
        " EFlags=" + String.Format("{0,4:X}", regs.EFlags));
      return regs;
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Step(uint id) {
      // SingleStep until
      //  - rip no longer points to the same source line.
      //  - some other thread throws a signal.
      //  - some signal other than STEP is thrown on this thread
      //
      // Some debugers might implement this a just a single step however,
      // given the number of "NOP" required by the jump alignment, this 
      // could be painful, so instead we look for a different line.
      ulong rip = ((RegsX86_64) GetRegisters(id)).Rip;
      DocumentPosition pos = symbols_.PositionFromAddress(rip);

      // Check if we are starting on a breakpoint.  If so we need to 
      // temporarily remove it or we will immediately trigger a break
      // without moving.
      bool bp = gdb_.HasBreakpoint(rip);
      if (StepFinished != null) { 
        StepFinished(this, 
                     SimpleDebuggerTypes.EventType.Step,
                     SimpleDebuggerTypes.ResultCode.Ok);
      }

      do
      {
        // If we are on a breakpoint, temporarily remove it by
        // stepping over it
        if (bp) {
          RemoveBreakpoint(rip);
          sendStopMessages_ = false;
        }
        gdb_.RequestStep();
        if (bp) 
        {
          AddBreakpoint(rip);
          sendStopMessages_ = true;

          // We only need to check the first step, other BPs are valid
          bp = false;
        }

        // If the signal is not a break trap, or if the thead changed
        // something else triggered the stop, so we are done.
        int sig;
        gdb_.GetLastSig(out sig);
        if (sig != 5)
          break;

        //TODO(noelallen) Add check for thread change...
        //if (id != ) break;

        rip = ((RegsX86_64) GetRegisters(id)).Rip;
      } while (pos == symbols_.PositionFromAddress(rip));
    }
    

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Continue() {
      //TODO(noelallen) - use correct ID below
      RegsX86_64 regs = (RegsX86_64) GetRegisters(0);
      ulong rip = regs.Rip;

      Debug.WriteLine("CONTINUE, rip=0x" + String.Format("{0,4:X}", rip));
      if (gdb_.HasBreakpoint(rip))
      {
        Debug.WriteLine("NaClDebugger.cs, Continue()" + 
                        "-HasBreakpoint = true, rip=" + rip);
        RemoveBreakpoint(rip);
        // First step one instruction, to prevent a race condition
        // where the IP gets back to the current line before we have
        // a chance to re-enable the breakpoint
        sendStopMessages_ = false;
        gdb_.RequestStep();
        sendStopMessages_ = true;
        AddBreakpoint(rip);
      }
      else {
        Debug.WriteLine("NaClDebugger.cs, Continue()-HasBreakpoint = false");
      }

      var result = gdb_.RequestContinueBackground();
      OnGdbContinue(result);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public bool HasBreakpoint(ulong addr)
    {
      return gdb_.HasBreakpoint(addr);
    }
    
    [MethodImpl(MethodImplOptions.Synchronized)]
    public void AddBreakpoint(ulong addr) {
      gdb_.AddBreakpoint(addr);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void RemoveBreakpoint(ulong addr) {
      gdb_.RemoveBreakpoint(addr);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void AddTempBreakpoint(ulong addr)
    {
      // Don't if a real one exists
      if (HasBreakpoint(addr)) return;
      gdb_.AddBreakpoint(addr);
      tempBreakpoints_.Add(addr);
      
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void RemoveTempBreakpoints()
    {
      foreach (ulong addr in tempBreakpoints_) {
        gdb_.RemoveBreakpoint(addr);
      }
      tempBreakpoints_.Clear();
    }


    [MethodImpl(MethodImplOptions.Synchronized)]
    public IEnumerable<uint> GetThreads() {
      EventWaitHandle evt = new EventWaitHandle(false, EventResetMode.AutoReset);
      GdbProxy.ResultCode status;
      List<uint> tids = new List<uint>();
      gdb_.GetThreads(
          (r, s, d) => {
            status = r;
            if (status == GdbProxy.ResultCode.DHR_OK) {
              ParseThreadsString(s, tids);
            }
            evt.Set();
          });
      evt.WaitOne();
      return tids;
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void GetMemory(ulong sourceAddress, Array destination, uint countInBytes) {
      GdbProxy.ResultCode result;

      sourceAddress += baseAddress_;

      result = gdb_.GetMemory(sourceAddress, destination, countInBytes);
      if (result != GdbProxy.ResultCode.DHR_OK) {
        throw new ApplicationException("Failed GetMemory query");
      }
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void SetMemory(ulong destAddress, Array src, uint count)
    {
      GdbProxy.ResultCode result;

      destAddress += baseAddress_;

      result = gdb_.SetMemory(destAddress, src, count);
      if (result != GdbProxy.ResultCode.DHR_OK)
      {
        throw new ApplicationException("Failed GetMemory query");
      }
    }

    #endregion

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Open(string connectionString) {
      sendStopMessages_ = false;
      gdb_.Open(connectionString);

      // Can't set these until after Open() returns
      gdb_.SetStopAsync(OnGdbStop);
      gdb_.SetOutputAsync(OnGdbOutput);

      EventWaitHandle evt = new EventWaitHandle(false, EventResetMode.AutoReset);
      GdbProxy.ResultCode status;      
      gdb_.GetArch(
          (r, s, d) => {
            status = r;
            if (status == GdbProxy.ResultCode.DHR_OK) {
              ParseArchString(s);
            }
            evt.Set();
          });
      if (!evt.WaitOne(gdbTimeout_)) {
        throw new TimeoutException("GDB connection timed out");
      }     
      gdb_.GetPath(
          (r, s, d) => {
            status = r;
            if (status == GdbProxy.ResultCode.DHR_OK && s!="") {
              SetPath(s);
            }
            else if (s == "") {
              // FIXME -- this is a HACK -- I need to set the
              // path based on Project data, rather than from
              // querying sel_ldr
              SetPath("c:\\src\\NACL_SDK\\src\\examples\\hello_world\\Project3\\Project3\\bin\\Debug\\Project3_x86_64.nexe");
            }
            evt.Set();
          });
      if (!evt.WaitOne(gdbTimeout_)) {
        throw new TimeoutException("GDB connection timed out");
      }

      InvokeOpened(SimpleDebuggerTypes.ResultCode.Ok, path_);

      sendStopMessages_ = true;
      gdbWorkerThread_ = new System.Threading.Thread(GdbWorkerThreadProc);
      gdbWorkerThread_.Name = "GDB Proxy Background Worker";
      gdbWorkerThread_.Start();
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Close() {
      gdbTermEvent_.Set();
      gdb_.Close();
      gdbWorkerThread_.Join(gdbPingInterval_ * 8);
      if (gdbWorkerThread_.IsAlive) {
        gdbWorkerThread_.Abort();
      }
      gdbWorkerThread_ = null;
    }



    #region Private Implementation

    private readonly ulong baseAddress_;

    private readonly EventWaitHandle gdbTermEvent_
        = new EventWaitHandle(
            false,
            EventResetMode.ManualReset);

    private readonly GdbProxy gdb_ = new GdbProxy();
    private readonly List<Closure> stoppingEventClosures_ = new List<Closure>();
    private readonly NaClSymbolProvider symbols_;
    private bool sendStopMessages_ = true;
    private List<ulong> tempBreakpoints_ = new List<ulong>();

    private string arch_;
    private int gdbPingInterval_ = 2000; // in ms
    private System.Threading.Thread gdbWorkerThread_;
    private string path_;
    private int gdbTimeout_ = 1000; // in ms
//    private RegsX86_64 registers_ = new RegsX86_64();

    #endregion

    #region Private Implementation

    //
    // FIXME -- WHY?? do we keep pinging sel_ldr?  gdb does NOT do this!
    //
    private void GdbWorkerThreadProc() {
      do {
        lock (this) {
          int lastSig = 0;
          /// FIXME -- why do we also ask for last signal?  Should we only
          /// do this when needed, instead of ALWAYS?
          gdb_.GetLastSig(out lastSig);
        }
      } while (gdbTermEvent_.WaitOne(gdbPingInterval_*1) == false);
    }

    private void OnDebuggeeContinue() {}

    private void OnGdbContinue(GdbProxy.ResultCode result) {
      if (Continuing != null) {
        Continuing(this,
                   SimpleDebuggerTypes.EventType.Continue,
                   (SimpleDebuggerTypes.ResultCode) result);
      }
    }

    private void OnGdbOutput(GdbProxy.ResultCode result, string msg, byte[] data) {
      if (Output != null) {
        Output(this, (SimpleDebuggerTypes.ResultCode) result, msg);
      }
    }

    private void OnGdbStop(GdbProxy.ResultCode result, string msg, byte[] data) {
      //
      // Remove all temporary breakpoints
      //
      RemoveTempBreakpoints();

      foreach (Closure stoppingEventClosure in stoppingEventClosures_) {
        stoppingEventClosure();
      }
      stoppingEventClosures_.Clear();

      if (sendStopMessages_ && Stopped != null) {
        Debug.WriteLine("Sending stopped message");
        Stopped(this,
                SimpleDebuggerTypes.EventType.Break,
                (SimpleDebuggerTypes.ResultCode) result);
      }
    }

    private void ParseArchString(string msg) {
      Debug.WriteLine(msg);
      try {
        XElement targetString = XElement.Parse(msg);
        var archElements = targetString.Descendants("architecture");
        var el = archElements.FirstOrDefault();
        arch_ = el.Value;
      }
      catch (Exception e) {
        Debug.WriteLine(e.Message);
      }
    }

    private void ParseThreadsString(string msg, List<uint> tids) {
      Debug.WriteLine(msg);
      try {
        XElement threadsString = XElement.Parse(msg);
        foreach (XElement el in threadsString.Descendants("thread")) {
          string tidStr = el.Attribute("id").Value;
          uint tid = Convert.ToUInt32(tidStr, 16);

          tids.Add(tid);
        }
      }
      catch (Exception e) {
        Debug.WriteLine(e.Message);
      }
    }

    private void SetPath(string msg) {
      // TODO(ilewis): get load offset from debuggee instead of
      // hardcoding like this.
      string status;
      path_ = msg;
      Debug.WriteLine("SetPath {" + msg + "}");
      symbols_.LoadModule(path_, 0x0000000c00000000, out status);
      if (ModuleLoaded != null) {
        ModuleLoaded(this, msg, status);
      }
    }

    private void InvokeOpened(SimpleDebuggerTypes.ResultCode status, string msg)
    {
      SimpleDebuggerTypes.MessageHandler handler = Opened;
      if (handler != null)
      {
        handler(this, status, msg);
      }
    }
    #endregion

    #region Private Implementation

    private delegate void Closure();

    #endregion
  }
}
