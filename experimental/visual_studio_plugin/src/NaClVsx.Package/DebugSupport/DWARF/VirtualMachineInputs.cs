// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using Google.MsAd7.BaseImpl.Interfaces;
using NaClVsx;
using NaClVsx.DebugHelpers;

#endregion

namespace Google.NaClVsx.DebugSupport.DWARF {
  class VirtualMachineInputs : IDwarfVM {
    public VirtualMachineInputs(ISimpleDebugger dbg, ulong frameBase) {
      dbg_ = dbg;
      frameBase_ = frameBase;
    }

    public ulong FrameBase {
      get { return frameBase_; }
      set { frameBase_ = value; }
    }

    #region Implementation of IDwarfVM

    public uint BitWidth() {
      return bitWidth_;
    }

    public bool IsLSB() {
      return isLsb_;
    }

    public void ErrorString(string str) {
      msg_ = str;
    }

    public ulong ReadRegister(int regNumber) {
      if (registers_ == null) {
        if (dbg_ != null) {
          //TODO(noelallen) : this should have a copy of the regs?
          registers_ = (RegsX86_64) dbg_.GetRegisters(0);
        } else {
          registers_ = new RegsX86_64();
        }
      }
      return registers_[regNumber];
    }

    public ulong ReadMemory(ulong address, int count) {
      ulong result = 0;
      if (dbg_ != null) {
        // line below from Ian's RSP branch
        result = dbg_.GetU64(address + dbg_.BaseAddress);

        // this is what the code originally was.  
        //result = dbg_.GetU64(address);
        // FIXME -- clean this up...but I am not hitting this
        // code yet (when I set a breakpoint) so I don't want
        // to remove the comments and FIXME until I evaluate it
        // more
      }
      return result;
    }

    public ulong ReadFrameBase() {
      return frameBase_;
    }

    #endregion

    #region Private Implementation

    private readonly ISimpleDebugger dbg_;

    private uint bitWidth_ = 64;
    ulong frameBase_;
    private bool isLsb_ = true;
    private string msg_ = "OK";
    private RegsX86_64 registers_;

    #endregion
  }
}
