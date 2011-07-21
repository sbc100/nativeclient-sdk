// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Xml;
using System.Xml.Linq;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.Interfaces;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using NaClVsx.DebugHelpers;

#endregion

namespace Google.NaClVsx.DebugSupport {
  public class ProgramNode
      : IDebugProgramNode2,
        IDebugProgram2,
        IDebugProgramNodeAttach2 {
    public ProgramNode(NaClDebugProcess process) {
      //
      // Program nodes get created and destroyed a lot, so best to avoid
      // doing much in this constructor.
      //
      Trace.WriteLine("Created ProgramNode");

      process_ = process;

      mainThread_ = new Thread(this, "main", 0);
      modules_.Add(mainModule_);
    }

    public INaClDebugger Dbg {
      get { return dbg_; }
    }

    public Guid ProgramGuid {
      get { return programGuid_; }
    }

    public List<Thread> Threads {
      get { return threads_; }
    }

    public List<Module> Modules {
      get { return modules_; }
    }

    public string Arch {
      get { return arch_; }
    }

    public Thread MainThread {
      get { return mainThread_; }
    }

    public Module MainModule {
      get { return mainModule_; }
    }

    public Engine Engine {
      get { return engine_; }
    }

    public void AttachEngine(Engine eng) {
      Debug.WriteLine("ProgramNode.AttachEngine");
      engine_ = eng;


      dbg_.Stopped += OnStopEvent;
      dbg_.Continuing += OnContinueEvent;
      dbg_.StepFinished += OnStepFinished;
      dbg_.ModuleLoaded += OnModuleLoaded;

      dbg_.Open(process_.NaClPort.ConnectionString);

      var tids = dbg_.GetThreads();
      foreach (var tid in tids) {
        threads_.Add(
            new Thread(
                this,
                String.Format("Thread {0}", tid),
                tid));
      }

      SendEvent(
          null,
          new Ad7Events.DebugProgramCreateEvent(
              enum_EVENTATTRIBUTES.EVENT_ASYNCHRONOUS));
    }

    #region IDebugProgram2 Members

    public int EnumThreads(out IEnumDebugThreads2 ppEnum) {
      Debug.WriteLine("ProgramNode.EnumThreads");

      ppEnum = new ThreadEnumerator(threads_.ConvertAll(t => (IDebugThread2) t));
      return VSConstants.S_OK;
    }

    public int GetName(out string pbstrName) {
      // (ilewis) commenting out the debug spew for this function
      // because it's called with ludicrous frequency.
      // Debug.WriteLine("ProgramNode.GetName");

      pbstrName = process_.ImagePath;
      return VSConstants.S_OK;
    }

    public int GetProcess(out IDebugProcess2 ppProcess) {
      Debug.WriteLine("ProgramNode.GetProcess");

      // NOTE: not currently called by VS debugger
      throw new NotImplementedException();
    }

    public int Terminate() {
      Debug.WriteLine("ProgramNode.Terminate");
      // Since we have only one program per process, we
      // don't do anything here. The DebugProcess instance
      // will kill the process.
      SendEvent(MainThread, new Ad7Events.DebugProgramDestroyEvent(0));
      return VSConstants.S_OK;
    }

    public int Attach(IDebugEventCallback2 pCallback) {
      Debug.WriteLine("ProgramNode.Attach");
      // NOTE: not currently called by VS debugger
      throw new NotImplementedException();
    }

    public int CanDetach() {
      Debug.WriteLine("ProgramNode.CanDetach");
      return VSConstants.S_OK;
    }

    public int Detach() {
      Debug.WriteLine("ProgramNode.Detach");

      dbg_.Close();
      SendEvent(null, new Ad7Events.DebugProgramDestroyEvent(0));
      return VSConstants.S_OK;
    }

    public int GetProgramId(out Guid pguidProgramId) {
      Debug.WriteLine("ProgramNode.GetProgramId");
      pguidProgramId = programGuid_;
      return VSConstants.S_OK;
    }

    public int GetDebugProperty(out IDebugProperty2 ppProperty) {
      Debug.WriteLine("ProgramNode.GetDebugProperty");
      throw new NotImplementedException();
    }

    public int Execute() {
      Debug.WriteLine("ProgramNode.Execute");
      dbg_.Continue();
      return VSConstants.S_OK;
    }

    public int Continue(IDebugThread2 pThread) {
      Debug.WriteLine("ProgramNode.Continue");
      dbg_.Continue();
      return VSConstants.S_OK;
    }

    public int Step(IDebugThread2 pThread,
                    enum_STEPKIND sk,
                    enum_STEPUNIT Step) {
      uint id;
      if (pThread.GetThreadId(out id) != VSConstants.S_OK) {
        id = 0;
      }
      var thread = (Thread) pThread;
      var stack = thread.GetStack();
      var addr = stack[1].ReturnAddress;
      var rip = stack[0].ReturnAddress;

      switch (sk) {
          /*
         *  In the STEP OUT case, we want to break when the IP hits
         *  the return address on the stack.
         */
        case enum_STEPKIND.STEP_OUT:
          dbg_.AddTempBreakpoint(addr);
          dbg_.Continue();
          break;

          /*
         *  In the STEP INTO case, we want to single step until the line
         *  number changes.
         *  TODO(noelallen): This is over simplified, and should be fixed
         */
        case enum_STEPKIND.STEP_INTO:
          dbg_.Step(id);
          break;

          /*
         *  In the STEP OVER case, we want to break on any line number
         *  change in the same function, or if we return, so we add
         *  breakpoints for all lines.
         *  TODO(noelallen): This is over simplified, and should be fixed
         */
        case enum_STEPKIND.STEP_OVER:
          // Add the 'return' case
          dbg_.AddTempBreakpoint(addr);
          DocumentPosition oldPos = null;
          var addrs = dbg_.Symbols.GetAddressesInScope(rip);
          foreach (var curraddr in addrs) {
            var newPos = dbg_.Symbols.PositionFromAddress(curraddr);
            if (newPos != oldPos) {
              dbg_.AddTempBreakpoint(curraddr);
            }
            newPos = oldPos;
          }

          dbg_.Continue();
          break;
      }
      return VSConstants.S_OK;
    }

    public int CauseBreak() {
      Debug.WriteLine("ProgramNode.CauseBreak");
      dbg_.Break();
      return VSConstants.S_OK;
    }

    public int EnumCodeContexts(IDebugDocumentPosition2 pDocPos,
                                out IEnumDebugCodeContexts2 ppEnum) {
      Debug.WriteLine("ProgramNode.EnumCodeContexts");
      throw new NotImplementedException();
    }

    public int GetMemoryBytes(out IDebugMemoryBytes2 ppMemoryBytes) {
      Debug.WriteLine("ProgramNode.GetMemoryBytes");
      // TODO: replace hardcoded values with something real
      ppMemoryBytes = new MemoryBytes(dbg_, 0, 65536);
      return VSConstants.S_OK;
    }

    public int GetDisassemblyStream(
        enum_DISASSEMBLY_STREAM_SCOPE dwScope,
        IDebugCodeContext2 pCodeContext,
        out IDebugDisassemblyStream2 ppDisassemblyStream) {
      Debug.WriteLine("ProgramNode.GetDisassemblyStream");
      throw new NotImplementedException();
    }

    public int EnumModules(out IEnumDebugModules2 ppEnum) {
      Debug.WriteLine("ProgramNode.EnumModules");

      ppEnum = new ModuleEnumerator(modules_.ConvertAll(m => (IDebugModule2) m));
      return VSConstants.S_OK;
    }

    public int GetENCUpdate(out object ppUpdate) {
      Debug.WriteLine("ProgramNode.GetENCUpdate");
      throw new NotImplementedException();
    }

    public int EnumCodePaths(string pszHint,
                             IDebugCodeContext2 pStart,
                             IDebugStackFrame2 pFrame,
                             int fSource,
                             out IEnumCodePaths2 ppEnum,
                             out IDebugCodeContext2 ppSafety) {
      Debug.WriteLine("ProgramNode.EnumCodePaths");
      throw new NotImplementedException();
    }

    public int WriteDump(enum_DUMPTYPE DUMPTYPE, string pszDumpUrl) {
      Debug.WriteLine("ProgramNode.WriteDump");
      throw new NotImplementedException();
    }

    #endregion

    #region IDebugProgramNode2 Members

    public int GetProgramName(out string pbstrProgramName) {
      Debug.WriteLine("ProgramNode.GetProgramName");
      pbstrProgramName = null;
      return VSConstants.S_OK;
    }

    public int GetHostName(enum_GETHOSTNAME_TYPE dwHostNameType,
                           out string pbstrHostName) {
      Debug.WriteLine("ProgramNode.GetHostName");
      pbstrHostName = null;
      return VSConstants.S_OK;
    }

    public int GetHostPid(AD_PROCESS_ID[] pHostProcessId) {
      Debug.WriteLine("ProgramNode.GetHostPid");
      return process_.GetPhysicalProcessId(pHostProcessId);
    }

    public int GetEngineInfo(out string pbstrEngine, out Guid pguidEngine) {
      Debug.WriteLine("ProgramNode.GetEngineInfo");
      pbstrEngine = Engine.kName;
      pguidEngine = new Guid(Engine.kId);
      return VSConstants.S_OK;
    }

    public int GetHostMachineName_V7(out string pbstrHostMachineName) {
      Debug.WriteLine("ProgramNode.GetHostMachineName_V7");
      pbstrHostMachineName = null;
      return VSConstants.E_NOTIMPL;
    }

    public int Attach_V7(IDebugProgram2 pMDMProgram,
                         IDebugEventCallback2 pCallback,
                         uint dwReason) {
      Debug.WriteLine("ProgramNode.Attach_V7");
      return VSConstants.E_NOTIMPL;
    }

    public int DetachDebugger_V7() {
      Debug.WriteLine("ProgramNode.DetachDebugger_V7");
      return VSConstants.E_NOTIMPL;
    }

    #endregion

    #region Implementation of IDebugProgramNodeAttach2

    public int OnAttach(ref Guid guidProgramId) {
      Debug.WriteLine("ProgramNode.OnAttach");
      programGuid_ = guidProgramId;
      return VSConstants.S_OK;
    }

    #endregion

    #region Private Implementation

    private readonly INaClDebugger dbg_ = new NaClDebugger(0x0000000c00000000);

    private readonly Module mainModule_ = new Module();
    private readonly Thread mainThread_;
    private readonly List<Module> modules_ = new List<Module>();
    private readonly NaClDebugProcess process_;

    private readonly Dictionary<int, string> sourceIdToPath_ =
        new Dictionary<int, string>();

    private readonly List<Thread> threads_ = new List<Thread>();

    // TODO(ilewis): get the base address from the debuggee instead
    // of hardcoding it

    private string arch_;
    private Engine engine_;

    private Dictionary<string, int> pathToSourceId_ =
        new Dictionary<string, int>();

    private Guid programGuid_ = Guid.NewGuid();

    #endregion

    #region Private Implementation

    private void LoadModuleWithPath(string msg) {
      Debug.WriteLine(msg);
      mainModule_.Url = msg;
      mainModule_.Name = Path.GetFileName(msg);
      mainModule_.UrlSymbolLocation = msg;

      SendEvent(
          null,
          new Ad7Events.DebugModuleLoadEvent(
              mainModule_,
              "Loading module",
              true));

      string symbolStatus;
      dbg_.Symbols.LoadModule(msg, 0x0000000c00000000, out symbolStatus);
      mainModule_.DebugMessage = symbolStatus;
      SendEvent(
          null,
          new Ad7Events.DebugSymbolSearchEvent(
              mainModule_,
              "DWARF symbols loaded",
              enum_MODULE_INFO_FLAGS.
                  MIF_SYMBOLS_LOADED));
    }

    private void OnBreak(GdbProxy.ResultCode status, string msg, byte[] data) {
      SendEvent(mainThread_, new Ad7Events.DebugBreakEvent());
    }

    void OnContinueEvent(ISimpleDebugger sender,
                         SimpleDebuggerTypes.EventType t,
                         SimpleDebuggerTypes.ResultCode status) {
      Debug.WriteLine("Debug event: Continue");
    }

    private void OnModuleLoaded(ISimpleDebugger sender,
                                string modulepath,
                                string status) {
      Debug.WriteLine(string.Format("{0}: {1}", modulepath, status));
      mainModule_.Url = modulepath;
      mainModule_.Name = Path.GetFileName(modulepath);
      mainModule_.UrlSymbolLocation = modulepath;

      SendEvent(
          null,
          new Ad7Events.DebugModuleLoadEvent(
              mainModule_,
              "Loading module",
              true));

      SendEvent(
          null,
          new Ad7Events.DebugSymbolSearchEvent(
              mainModule_,
              "DWARF symbols loaded",
              enum_MODULE_INFO_FLAGS.
                  MIF_SYMBOLS_LOADED));
    }

    void OnStepFinished(ISimpleDebugger sender,
                        SimpleDebuggerTypes.EventType t,
                        SimpleDebuggerTypes.ResultCode status) {
      SendEvent(mainThread_, new Ad7Events.DebugStepCompleteEvent());
    }

    void OnStopEvent(ISimpleDebugger sender,
                     SimpleDebuggerTypes.EventType eventType,
                     SimpleDebuggerTypes.ResultCode status) {
      Debug.WriteLine("Debug event: Break");
      engine_.EnableAllBreakpoints(false);
      SendEvent(mainThread_, new Ad7Events.DebugBreakEvent());
    }

    private void ParseArchString(string msg) {
      Debug.WriteLine(msg);
      try {
        var targetString = XElement.Parse(msg);
        var archElements =
            targetString.Descendants("architecture");
        var el = archElements.FirstOrDefault();
        arch_ = el.Value;
      }
      catch (XmlException e) {
        Debug.WriteLine(e.Message);
      }
    }

    private void ParseThreadsString(string msg) {
      Debug.WriteLine(msg);
      try {
        var threadsString = XElement.Parse(msg);
        foreach (var el in threadsString.Descendants("thread")) {
          var tidStr = el.Attribute("id").Value;
          var tid = Convert.ToUInt32(tidStr, 16);

          threads_.Add(
              new Thread(
                  this,
                  String.Format("Thread {0}", tid),
                  tid));
        }
      }
      catch (Exception e) {
        Debug.WriteLine(e.Message);
      }
    }

    #endregion

    internal void SendEvent(IDebugThread2 thread, Ad7Events.DebugEvent evt) {
      engine_.SendEvent(this, thread, evt);
    }
        }
}
