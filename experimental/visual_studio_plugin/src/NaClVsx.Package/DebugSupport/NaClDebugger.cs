// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Xml.Linq;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.NaClVsx.ProjectSupport;
using NaClVsx.DebugHelpers;

#endregion

namespace Google.NaClVsx.DebugSupport {
  public sealed class NaClDebugger : INaClDebugger {
    public NaClDebugger() {
      BaseAddress = 0;
      symbols_ = new NaClSymbolProvider(this);
    }

    public event SimpleDebuggerTypes.EventHandler Stopped;
    public event SimpleDebuggerTypes.EventHandler StepFinished;
    public event SimpleDebuggerTypes.EventHandler Continuing;
    public event SimpleDebuggerTypes.MessageHandler Output;
    public event SimpleDebuggerTypes.ModuleLoadHandler ModuleLoaded;
    public event SimpleDebuggerTypes.MessageHandler Opened;

    public string Arch { get; private set; }

    public string Path { get; private set; }

    #region INaClDebugger Members

    public ulong BaseAddress { get; private set; }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Open(string connectionString) {
      sendStopMessages_ = false;
      gdbProxy_.Open(connectionString);

      // Can't set these until after Open() returns
      gdbProxy_.SetStopAsync(OnGdbStop);
      gdbProxy_.SetOutputAsync(OnGdbOutput);

      var evt = new EventWaitHandle(false, EventResetMode.AutoReset);
      gdbProxy_.GetArch(
          (r, s, d) => {
            if (GdbProxy.ResultCode.DHR_OK == r) {
              ParseArchString(s);
            }
            evt.Set();
          });
      if (!evt.WaitOne(kGdbTimeout)) {
        throw new TimeoutException("GDB connection timed out");
      }

      var regs = new RegsX86_64();
      gdbProxy_.GetRegisters(ref regs);
      BaseAddress = regs.R15;

      var fullNexePath = NaClProjectConfig.GetLastNexe();
      // note -- LoadModuleWithPath uses |BaseAddress| which we set
      // above by reading register |R15|. Works only in x86-64 sandbox.
      LoadModuleWithPath(fullNexePath);
      /**
       * TODO(mmortensen): In the future we may want to
       * query sel_ldr for the nexe name (and full path?) to
       * make sure we are running the nexe we built...esp
       * when we are launching through chrome and using a server
       * to run our app.
        gdb_proxy_.GetPath(
          (r, s, d) => {
            status = r;
            if (status == GdbProxy.ResultCode.DHR_OK && s!="") {
              LoadModuleWithPath(s);
            }
            else if (s == "") {
              // Set the path based on project data, as obtained from
              // NaClProjectconfig.NexeList.
              LoadModuleWithPath(full_nexe_path);
            }
            evt.Set();
          });
        if (!evt.WaitOne(gdbTimeout_)) {
          throw new TimeoutException("GDB connection timed out");
        }
      **/
      InvokeOpened(SimpleDebuggerTypes.ResultCode.Ok, Path);

      sendStopMessages_ = true;
      gdbWorkerThread_ = new System.Threading.Thread(GdbWorkerThreadProc);
      gdbWorkerThread_.Name = "GDB Proxy Background Worker";
      gdbWorkerThread_.Start();

      // Debuggee is stopped at nexe entry point.
      // Calling |GetLastSig| results in debugger getting notification about
      // debuggee stopped status.
      var lastSig = 0;
      gdbProxy_.GetLastSig(out lastSig);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Close() {
      gdbTermEvent_.Set();
      gdbProxy_.Close();
      gdbWorkerThread_.Join(kGdbPingInterval * 8);
      if (gdbWorkerThread_.IsAlive) {
        gdbWorkerThread_.Abort();
      }
      gdbWorkerThread_ = null;
    }

    #endregion

    #region Implementation of ISimpleDebugger

    public string Architecture {
      get { return Arch; }
    }

    public ISimpleSymbolProvider Symbols {
      get { return symbols_; }
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Break() {
      gdbProxy_.RequestBreak();
    }

      [MethodImpl(MethodImplOptions.Synchronized)]
    public object GetRegisters(uint id) {
      // FIXME -- |id| does NOT appear to be used by this function!!
      // Note: |id| can be probably used to indicate register sets other than 'general' registers.
      // For example, set of SSE registers.
      var regs = new RegsX86_64();
      gdbProxy_.GetRegisters(ref regs);
      if (regs.Rip == 0) {
        Debug.WriteLine("ERROR: regs.RIPS is 0");
      } else {
        Debug.WriteLine("regs.RIPS is " + String.Format("{0,4:X}", regs.Rip));
      }
      Debug.WriteLine(
          " GetRegisters.... Rip=" +
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
      var rip = ((RegsX86_64) GetRegisters(id)).Rip;
      var pos = symbols_.PositionFromAddress(rip);

      // Check if we are starting on a breakpoint.  If so we need to 
      // temporarily remove it or we will immediately trigger a break
      // without moving.
      var bp = gdbProxy_.HasBreakpoint(rip);
      if (StepFinished != null) {
        StepFinished(
            this,
            SimpleDebuggerTypes.EventType.Step,
            SimpleDebuggerTypes.ResultCode.Ok);
      }

      do {
        // If we are on a breakpoint, temporarily remove it by
        // stepping over it
        if (bp) {
          RemoveBreakpoint(rip);
          sendStopMessages_ = false;
        }
        gdbProxy_.RequestStep();
        if (bp) {
          AddBreakpoint(rip);
          sendStopMessages_ = true;

          // We only need to check the first step, other BPs are valid
          bp = false;
        }

        // If the signal is not a break trap, or if the thead changed
        // something else triggered the stop, so we are done.
        int sig;
        gdbProxy_.GetLastSig(out sig);
        if (sig != kTrapSignal)
          break;

        //TODO(noelallen) Add check for thread change...
        //if (id != ) break;

        rip = ((RegsX86_64) GetRegisters(id)).Rip;
      } while (pos == symbols_.PositionFromAddress(rip));
    }


    [MethodImpl(MethodImplOptions.Synchronized)]
    public void Continue() {
      //TODO(noelallen) - use correct ID below
      var regs = (RegsX86_64)GetRegisters(0);
      var rip = regs.Rip;

      Debug.WriteLine("CONTINUE, rip=0x" + String.Format("{0,4:X}", rip));
      if (gdbProxy_.HasBreakpoint(rip)) {
        Debug.WriteLine(
            "NaClDebugger.cs, Continue()" +
            "-HasBreakpoint = true, rip=" +
            String.Format("{0,4:X}", rip));
        RemoveBreakpoint(rip);
        // First step one instruction, to prevent a race condition
        // where the IP gets back to the current line before we have
        // a chance to re-enable the breakpoint
        sendStopMessages_ = false;
        gdbProxy_.RequestStep();
        sendStopMessages_ = true;
        AddBreakpoint(rip);
      } else {
        Debug.WriteLine("NaClDebugger.cs, Continue()-HasBreakpoint = false");
      }

      var result = gdbProxy_.RequestContinue();
      OnGdbContinue(result);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public bool HasBreakpoint(ulong addr) {
      return gdbProxy_.HasBreakpoint(addr);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void AddBreakpoint(ulong addr) {
      gdbProxy_.AddBreakpoint(addr);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void RemoveBreakpoint(ulong addr) {
      gdbProxy_.RemoveBreakpoint(addr);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void AddTempBreakpoint(ulong addr) {
      // Don't if a real one exists
      if (HasBreakpoint(addr)) {
        return;
      }
      gdbProxy_.AddBreakpoint(addr);
      tempBreakpoints_.Add(addr);
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void RemoveTempBreakpoints() {
      foreach (var addr in tempBreakpoints_) {
        gdbProxy_.RemoveBreakpoint(addr);
      }
      tempBreakpoints_.Clear();
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public IEnumerable<uint> GetThreads() {
      var evt = new EventWaitHandle(false, EventResetMode.AutoReset);
      var tids = new List<uint>();
      gdbProxy_.GetThreads(
          (r, s, d) => {
            if (GdbProxy.ResultCode.DHR_OK == r) {
              ParseThreadsString(s, tids);
            }
            evt.Set();
          });
      evt.WaitOne();
      return tids;
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void GetMemory(ulong sourceAddress,
                          Array destination,
                          uint countInBytes) {
      var result = gdbProxy_.GetMemory(sourceAddress, destination, countInBytes);
      if (result != GdbProxy.ResultCode.DHR_OK) {
        throw new ApplicationException("Failed GetMemory query");
      }
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    public void SetMemory(ulong destAddress, Array src, uint count) {
      var result = gdbProxy_.SetMemory(destAddress, src, count);
      if (result != GdbProxy.ResultCode.DHR_OK) {
        throw new ApplicationException("Failed SetMemory query");
      }
    }

    public ulong GetU64(ulong address) {
      var data = new byte[8];
      GetMemory(address, data, 8);
      return BitConverter.ToUInt64(data, 0);
    }

    public uint GetU32(ulong address) {
      var data = new byte[4];
      GetMemory(address, data, 4);
      return BitConverter.ToUInt32(data, 0);
    }

    #endregion

    #region Private Implementation

    private readonly EventWaitHandle gdbTermEvent_ =
        new EventWaitHandle(false, EventResetMode.ManualReset);

    private readonly GdbProxy gdbProxy_ = new GdbProxy();
    private readonly NaClSymbolProvider symbols_;
    private readonly List<ulong> tempBreakpoints_ = new List<ulong>();

    private const int kGdbPingInterval = 1000; // in ms
    private const int kGdbTimeout = 10000; // in ms
    private const int kTrapSignal = 5;
    private System.Threading.Thread gdbWorkerThread_;
    private bool sendStopMessages_ = true;

    #endregion

    #region Private Implementation

    private void GdbWorkerThreadProc() {
      do {
        lock (this) {
          // gdb_proxy_::RequestContinue is not blocking anymore, so this worker thread
          // has to poll RSP connection for reply (sent by debug server when debuggee
          // stops for some reason).
          // gdb_proxy_.WaitForReply will notify callback registered by gdb_proxy_.SetStopAsync.
          if (gdbProxy_.IsRunning())
            gdbProxy_.WaitForReply();
        }
      } while (gdbTermEvent_.WaitOne(kGdbPingInterval * 1) == false);
    }

    private void InvokeOpened(SimpleDebuggerTypes.ResultCode status, string msg) {
      var handler = Opened;
      if (handler != null) {
        handler(this, status, msg);
      }
    }

    private void LoadModuleWithPath(string fullPathToNexe) {
      string status;
      Path = fullPathToNexe;
      Debug.WriteLine("LoadModuleWithPath {" + Path + "}");
      symbols_.LoadModule(Path, BaseAddress, out status);
      if (ModuleLoaded != null) {
        ModuleLoaded(this, Path, status);
      }
    }

    private void OnGdbContinue(GdbProxy.ResultCode result) {
      if (Continuing != null) {
        Continuing(
            this,
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
      RemoveTempBreakpoints();

      if (sendStopMessages_ && Stopped != null) {
        Debug.WriteLine("Sending stopped message");
        Stopped(
            this,
            SimpleDebuggerTypes.EventType.Break,
            (SimpleDebuggerTypes.ResultCode) result);
      }
    }

    private void ParseArchString(string msg) {
      Debug.WriteLine(msg);
      try {
        var targetString = XElement.Parse(msg);
        var archElements =
            targetString.Descendants("architecture");
        var el = archElements.FirstOrDefault();
        Arch = el.Value;
      }
      catch (Exception e) {
        Debug.WriteLine(e.Message);
      }
    }

    private void ParseThreadsString(string msg, List<uint> tids) {
      Debug.WriteLine(msg);
      try {
        var threadsString = XElement.Parse(msg);
        foreach (var el in threadsString.Descendants("thread")) {
          var tidStr = el.Attribute("id").Value;
          var tid = Convert.ToUInt32(tidStr, 16);

          tids.Add(tid);
        }
      }
      catch (Exception e) {
        Debug.WriteLine(e.Message);
      }
    }

    #endregion
  }
}
