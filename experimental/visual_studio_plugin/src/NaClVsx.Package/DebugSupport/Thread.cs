// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using Google.MsAd7.BaseImpl;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.DebugProperties;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using NaClVsx.DebugHelpers;
using StackFrame = Google.MsAd7.BaseImpl.StackFrame;

namespace Google.NaClVsx.DebugSupport {


  public class Thread : IDebugThread2 {
    public Thread(ProgramNode program, string name, uint tid) {
      name_ = name;
      tid_ = tid;
      program_ = program;

    }

    #region Implementation of IDebugThread2

    public int EnumFrameInfo(enum_FRAMEINFO_FLAGS dwFieldSpec,
                             uint nRadix,
                             out IEnumDebugFrameInfo2 ppEnum) {
      Debug.WriteLine("Thread.EnumFrameInfo");

      RefreshFrameInfo(stack_);

      List<FRAMEINFO> frames = new List<FRAMEINFO>(stack_.Count);
      FRAMEINFO[] fi = new FRAMEINFO[1];

      foreach (StackFrame stackFrame in stack_) {
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
    public List<StackFrame> GetStack() {
      return stack_; 
    }

    public int GetThreadProperties(enum_THREADPROPERTY_FIELDS dwFields,
                                   THREADPROPERTIES[] ptp) {
      Debug.WriteLine("Thread.GetThreadProperties");
      if(dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_ID)) {
        ptp[0].dwThreadId = tid_;
      }
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_NAME))
      {
        ptp[0].bstrName = name_;
      }

      // The following properties are currently bogus.
      //
      if (dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_LOCATION))
      {
        ptp[0].bstrLocation = "THREADPROPERTY: Location";
      }
      if(dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_PRIORITY)) {
        ptp[0].bstrPriority = "THREADPROPERTY: Priority";
      }
      if(dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_STATE)) {
        ptp[0].dwThreadState = (uint)enum_THREADSTATE.THREADSTATE_STOPPED;
      }
      if(dwFields.HasFlag(enum_THREADPROPERTY_FIELDS.TPF_SUSPENDCOUNT)) {
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

    protected void RefreshFrameInfo(List<StackFrame> stack) {
      stack.Clear();

      
      RegsX86_64 regs = new RegsX86_64();
      regs = (RegsX86_64)program_.Dbg.GetRegisters(0);

      RegisterSet rset = new RegisterSet(RegisterSetSchema.DwarfAmd64Integer, null);

      // TODO: need a less hacky way to initialize a RegisterSet from a RegsX86_64.
      foreach (var registerDef in RegisterSetSchema.DwarfAmd64Integer.Registers) {
        if (!registerDef.Pseudo) {
          rset[registerDef.Index] = regs[registerDef.Index];
        }
      }

        while(rset != null && rset["RIP"] != 0)
        {
          stack.Add(new StackFrame(rset, this, program_.MainModule, program_.Dbg));

          //
          // Get the register values of the previous frame.
          //
          NaClSymbolProvider sym = program_.Dbg.Symbols as NaClSymbolProvider;
          rset = sym.GetPreviousFrameState(rset);
        }


    }


    #region Private Implementation

    private string name_;

    private List<StackFrame> stack_ = new List<StackFrame>();
    private uint tid_;
    private ProgramNode program_;

    #endregion
  }
}