// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.Interfaces;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.NaClVsx.DebugSupport {
  /// <summary>
  /// For documentation of IDebugEngine3 and IDebugEngineLaunch2 members, please see the VS 2008
  /// MSDN documentation.
  /// </summary>
  [ComVisible(true)]
  [Guid(kClsId)]
  [ClassInterface(ClassInterfaceType.None)]
  public class Engine : IDebugEngine3, IDebugEngineLaunch2 {
    #region  constants

    public const string kClsId = "547F36D7-FC60-47F5-AE6A-0F351E9F7D8C";
    public const string kId = "2E061BBF-B1ED-40D8-8D5D-8E2F9B2ADEC7";
    public const string kName = "NaCl";

    #endregion

    public Engine() {
      Trace.WriteLine("NaCl Debug Engine Loaded");
    }

    public List<PendingBreakpoint> Breakpoints {
      get { return breakpoints_; }
    }

    /// <summary>
    /// This function can change the state of all breakpoints, managed by this engine.
    /// </summary>
    /// <param name="enable">If |enable| is true, enables all breakpoints, else disables them.
    /// </param>
    public void EnableAllBreakpoints(bool enable) {
      foreach (PendingBreakpoint bp in breakpoints_) {
        bp.Enable(enable ? 1 : 0);
      }
    }

    #region Implementation of IDebugEngine3

    public int EnumPrograms(out IEnumDebugPrograms2 ppEnum) {
      Debug.WriteLine("DebugEngine.EnumPrograms");
      ppEnum = new ProgramEnumerator(
          programs_.ConvertAll(n => (IDebugProgram2) n));
      return VSConstants.S_OK;
    }

    public int Attach(IDebugProgram2[] rgpPrograms,
                      IDebugProgramNode2[] rgpProgramNodes,
                      uint celtPrograms,
                      IDebugEventCallback2 pCallback,
                      enum_ATTACH_REASON dwReason) {
      Debug.WriteLine("DebugEngine.Attach");
      debugEventCallback_ = pCallback;

      SendEvent(null, null, new Ad7Events.DebugEngineCreateEvent(this));

      for (uint i = 0; i < celtPrograms; ++i) {
        var prog = rgpPrograms[i] as ProgramNode;
        if (prog == null) {
          continue;
        }

        programs_.Add(prog);
        prog.AttachEngine(this);
      }
      return VSConstants.S_OK;
    }

    public int CreatePendingBreakpoint(
        IDebugBreakpointRequest2 pBPRequest,
        out IDebugPendingBreakpoint2 ppPendingBP) {
      Debug.WriteLine("DebugEngine.CreatePendingBreakpoint");
      //var breakpointCreated = VSConstants.S_FALSE;
      var rq = new BreakpointRequest(pBPRequest);
      var bp = new PendingBreakpoint(programs_[0].Dbg, rq);
      IEnumDebugErrorBreakpoints2 bindErrors;
      int canBindResult = bp.CanBind(out bindErrors);
      if (canBindResult != VSConstants.S_OK) {
        uint errorCount;
        if (bindErrors.GetCount(out errorCount) == VSConstants.S_OK) {
          uint errorsInArray = 0;
          var errorArray = new IDebugErrorBreakpoint2[1];
          for (uint i = 0; i < errorCount; ++i) {
            if ((bindErrors.Next(1, errorArray, ref errorsInArray) !=
                 VSConstants.S_OK) || (errorsInArray != 1)) {
              continue;
            }
            var errorEvent =
                new Ad7Events.DebugBreakpointErrorEvent(errorArray[0]);
            SendEvent(null, null, errorEvent);
          }
        }
      }
      ppPendingBP = bp;
      breakpoints_.Add(bp);
      return VSConstants.S_OK;
    }

    public int SetException(EXCEPTION_INFO[] pException) {
      Debug.WriteLine("DebugEngine.SetException");
      throw new NotImplementedException();
    }

    public int RemoveSetException(EXCEPTION_INFO[] pException) {
      Debug.WriteLine("DebugEngine.RemoveSetException");
      throw new NotImplementedException();
    }

    public int RemoveAllSetExceptions(ref Guid guidType) {
      Debug.WriteLine("DebugEngine.RemoveAllSetExceptions");
      throw new NotImplementedException();
    }

    public int GetEngineId(out Guid pguidEngine) {
      Debug.WriteLine("DebugEngine.GetEngineId");
      pguidEngine = engineGuid_;
      return VSConstants.S_OK;
    }

    public int DestroyProgram(IDebugProgram2 pProgram) {
      Debug.WriteLine("DebugEngine.DestroyProgram");
      throw new NotImplementedException();
    }

    public int ContinueFromSynchronousEvent(IDebugEvent2 pEvent) {
      Debug.WriteLine("DebugEngine.ContinueFromSynchronousEvent");
      throw new NotImplementedException();
    }

    public int SetLocale(ushort wLangID) {
      Debug.WriteLine("DebugEngine.SetLocale");
      locale_ = wLangID;
      return VSConstants.S_OK;
    }

    public int SetRegistryRoot(string pszRegistryRoot) {
      Debug.WriteLine("DebugEngine.SetRegistryRoot");
      registryRoot_ = pszRegistryRoot;
      return VSConstants.S_OK;
    }

    public int SetMetric(string pszMetric, object varValue) {
      Debug.WriteLine("DebugEngine.SetMetric");
      throw new NotImplementedException();
    }

    public int CauseBreak() {
      Debug.WriteLine("DebugEngine.CauseBreak");
      throw new NotImplementedException();
    }

    public int SetSymbolPath(string szSymbolSearchPath,
                             string szSymbolCachePath,
                             uint Flags) {
      Debug.WriteLine("DebugEngine.SetSymbolPath");
      symbolSearchPath_ = szSymbolSearchPath;
      symbolCachePath_ = szSymbolCachePath;
      return VSConstants.S_OK;
    }

    public int LoadSymbols() {
      Debug.WriteLine("DebugEngine.LoadSymbols");
      throw new NotImplementedException();
    }

    public int SetJustMyCodeState(int fUpdate,
                                  uint dwModules,
                                  JMC_CODE_SPEC[] rgJMCSpec) {
      Debug.WriteLine("DebugEngine.SetJustMyCodeState");
      throw new NotImplementedException();
    }

    public int SetEngineGuid(ref Guid guidEngine) {
      Debug.WriteLine("DebugEngine.SetEngineGuid");
      engineGuid_ = guidEngine;
      return VSConstants.S_OK;
    }

    public int SetAllExceptions(enum_EXCEPTION_STATE dwState) {
      Debug.WriteLine("DebugEngine.SetAllExceptions");
      throw new NotImplementedException();
    }

    #endregion

    #region Private Implementation

    private readonly List<PendingBreakpoint> breakpoints_ =
        new List<PendingBreakpoint>();

    private readonly List<ProgramNode> programs_ = new List<ProgramNode>();
    private IDebugEventCallback2 debugEventCallback_;
    private Guid engineGuid_;
    private ushort locale_;
    private string registryRoot_;
    private string symbolCachePath_;
    private string symbolSearchPath_;

    #endregion

    #region Implementation of IDebugEngineLaunch2

    public int LaunchSuspended(string pszServer,
                               IDebugPort2 pPort,
                               string pszExe,
                               string pszArgs,
                               string pszDir,
                               string bstrEnv,
                               string pszOptions,
                               enum_LAUNCH_FLAGS dwLaunchFlags,
                               uint hStdInput,
                               uint hStdOutput,
                               uint hStdError,
                               IDebugEventCallback2 pCallback,
                               out IDebugProcess2 ppProcess) {
      Debug.WriteLine("DebugEngine.LaunchSuspended");

      var port = (NaClPort) pPort;

      ppProcess = null;
      debugEventCallback_ = pCallback;

      var psi = new ProcessStartInfo {
          Arguments = pszArgs,
          FileName = pszExe,
          WorkingDirectory = pszDir
      };


      ppProcess = port.CreateProcess(psi);

      return VSConstants.S_OK;
    }

    public int ResumeProcess(IDebugProcess2 pProcess) {
      Debug.WriteLine("DebugEngine.ResumeProcess");

      var proc = (NaClDebugProcess) pProcess;
      NaClPort port = proc.NaClPort;
      var program = new ProgramNode(proc);
      program.Dbg.Opened +=
          (sender, status, msg) => OnDebuggerOpened(sender, program);
      ComUtils.RequireOk(port.AddProgramNode(program));
      // program.Continue(null);
      return VSConstants.S_OK;
    }


    public int CanTerminateProcess(IDebugProcess2 pProcess) {
      Debug.WriteLine("DebugEngine.CanTerminateProcess");
      return VSConstants.S_OK;
    }

    public int TerminateProcess(IDebugProcess2 pProcess) {
      Debug.WriteLine("DebugEngine.TerminateProcess");
      pProcess.Terminate();
      return VSConstants.S_OK;
    }

    #endregion

    #region Private Implementation

    private void ForEachProgram(IDebugProcess2 process, ProgramDelegate action) {
      IEnumDebugPrograms2 programs;
      process.EnumPrograms(out programs);
      var prog = new IDebugProgram2[1];
      uint count = 0;
      while (programs.Next(1, prog, ref count) == VSConstants.S_OK) {
        action(prog[0]);
      }
    }

    private void OnDebuggerOpened(ISimpleDebugger sender, ProgramNode program) {
      SendEvent(
          program, program.MainThread, new Ad7Events.DebugLoadCompleteEvent());
    }

    #endregion

    #region Private Implementation

    private delegate void ProgramDelegate(IDebugProgram2 p);

    #endregion

    internal void SendEvent(IDebugProgram2 prog,
                            IDebugThread2 thread,
                            Ad7Events.DebugEvent evt) {
      Debug.WriteLine(string.Format("DebugEngine.SendEvent({0})", evt));
      Ad7Events.SendEvent(debugEventCallback_, this, null, prog, thread, evt);
    }
  }
}
