// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using System.Diagnostics;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.DebugProperties;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using NaClVsx.DebugHelpers;
using StackFrame = Google.MsAd7.BaseImpl.StackFrame;

#endregion

namespace Google.NaClVsx.DebugSupport {
  public class Thread : IDebugThread2 {
    public Thread(ProgramNode program, string name, uint tid) {
      name_ = name;
      tid_ = tid;
      program_ = program;
    }

    #region Implementation of IDebugThread2

    public List<StackFrame> GetStack() {
      return stack_;
    }

    /// <summary>
    ///   Called by the Visual Studio Debugger to retrieve the stack frames for
    ///   this thread.  On halt, this will be called on each thread.  Detailed
    ///   information about the arguments and required usage can be found on
    ///   msdn.
    /// </summary>
    public int EnumFrameInfo(enum_FRAMEINFO_FLAGS dwFieldSpec,
                             uint nRadix,
                             out IEnumDebugFrameInfo2 ppEnum) {
      Debug.WriteLine("Thread.EnumFrameInfo");

      RefreshFrameInfo(stack_);

      var frames = new List<FRAMEINFO>(stack_.Count);
      var fi = new FRAMEINFO[1];

      foreach (var stackFrame in stack_) {
        stackFrame.GetInfo(dwFieldSpec, nRadix, fi);
        frames.Add(fi[0]);
      }

      ppEnum = new FrameInfoEnumerator(frames);

      return VSConstants.S_OK;
    }

    public int GetName(out string pbstrName) {
      Debug.WriteLine("Thread.GetName");
      pbstrName = name_;
      return VSConstants.S_OK;
    }

    public int SetThreadName(string pszName) {
      Debug.WriteLine("Thread.SetThreadName");
      name_ = pszName;
      return VSConstants.S_OK;
    }

    public int GetProgram(out IDebugProgram2 ppProgram) {
      Debug.WriteLine("Thread.GetProgram");
      ppProgram = program_;
      return VSConstants.S_OK;
    }

    public int CanSetNextStatement(IDebugStackFrame2 pStackFrame,
                                   IDebugCodeContext2 pCodeContext) {
      Debug.WriteLine("Thread.CanSetNextStatement");

      // For now, we just don't support this.
      return VSConstants.S_FALSE;
    }

    public int SetNextStatement(IDebugStackFrame2 pStackFrame,
                                IDebugCodeContext2 pCodeContext) {
      Debug.WriteLine("Thread.SetNextStatement");
      throw new NotImplementedException();
    }

    public int GetThreadId(out uint pdwThreadId) {
      Debug.WriteLine("Thread.GetThreadId");
      pdwThreadId = tid_;
      return VSConstants.S_OK;
    }

    public int Suspend(out uint pdwSuspendCount) {
      Debug.WriteLine("Thread.Suspend");
      throw new NotImplementedException();
    }

    public int Resume(out uint pdwSuspendCount) {
      Debug.WriteLine("Thread.Resume");
      throw new NotImplementedException();
    }

    public int GetThreadProperties(enum_THREADPROPERTY_FIELDS dwFields,
                                   THREADPROPERTIES[] ptp) {
      Debug.WriteLine("Thread.GetThreadProperties");
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_ID)) {
        ptp[0].dwThreadId = tid_;
      }
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_NAME)) {
        ptp[0].bstrName = name_;
      }

      // The following properties are currently bogus.
      //
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_LOCATION)) {
        ptp[0].bstrLocation = "THREADPROPERTY: Location";
      }
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_PRIORITY)) {
        ptp[0].bstrPriority = "THREADPROPERTY: Priority";
      }
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_STATE)) {
        ptp[0].dwThreadState = (uint) enum_THREADSTATE.THREADSTATE_STOPPED;
      }
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_SUSPENDCOUNT)) {
        ptp[0].dwSuspendCount = 1;
      }

      ptp[0].dwFields = dwFields;

      return VSConstants.S_OK;
    }

    public int GetLogicalThread(IDebugStackFrame2 pStackFrame,
                                out IDebugLogicalThread2 ppLogicalThread) {
      Debug.WriteLine("Thread.GetLogicalThread");
      throw new NotImplementedException();
    }

    #endregion

    #region Private Implementation

    private readonly ProgramNode program_;

    private readonly List<StackFrame> stack_ = new List<StackFrame>();
    private readonly uint tid_;
    private string name_;

    #endregion

    /// <summary>
    ///   Gets the frame information for the current stack frame.  This means
    ///   getting a snapshot of the register state for each each frame in the
    ///   stack so that memory address information can be derived at a later
    ///   stage.
    /// </summary>
    /// <param name = "stack"></param>
    protected void RefreshFrameInfo(List<StackFrame> stack) {
      stack.Clear();
      var regs = (RegsX86_64) program_.Dbg.GetRegisters(tid_);
      var rset = new RegisterSet(RegisterSetSchema.DwarfAmd64Integer, null);

      // TODO: need a less hacky way to initialize a RegisterSet from a 
      // RegsX86_64.
      foreach (
          var registerDef in
              RegisterSetSchema.DwarfAmd64Integer.Registers) {
        if (!registerDef.Pseudo) {
          rset[registerDef.Index] = regs[registerDef.Index];
        }
      }
      while (rset["RIP"] != 0) {
        stack.Add(new StackFrame(rset, this, program_.MainModule, program_.Dbg));
        // Get the register values of the previous frame.
        var sym = program_.Dbg.Symbols as NaClSymbolProvider;
        var nextRset = sym.GetPreviousFrameState(rset);
        // The toolchain creates a dummy frame at the outermost scope.  Gdb
        // has some way to get past the dummy frame, but strict application of
        // the DWARF spec makes it look circular to us.  TODO: investigate why
        if (nextRset == null || nextRset["RIP"] == rset["RIP"]) {
          break;
        }
        rset = nextRset;
      }
    }
  }
}
