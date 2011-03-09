using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using Google.MsAd7.BaseImpl.Interfaces;
using NaClVsx;
using NaClVsx.DebugHelpers;

namespace Google.NaClVsx.DebugSupport.DWARF
{
  class VirtualMachineInputs : IDwarfVM
  {
    public VirtualMachineInputs(NaClDebugger dbg, ulong frameBase) {
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
        result = dbg_.GetU64(address);
      }
      return result;
    }

    public ulong ReadFrameBase() {
      return frameBase_;
    }

    #endregion

    private uint bitWidth_ = 64;
    private bool isLsb_ = true;
    private string msg_ = "OK";
    private readonly NaClDebugger dbg_;
    ulong frameBase_;
    private RegsX86_64 registers_ = null;
  }
}
