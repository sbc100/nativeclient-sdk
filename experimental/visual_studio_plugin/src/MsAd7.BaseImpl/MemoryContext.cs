// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System.Diagnostics;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  public class MemoryContext : IDebugMemoryContext2 {
    public MemoryContext(string name, ulong address, ulong count) {
      name_ = name;
      address_ = address;
      count_ = count;
    }

    public MemoryContext(string name, ulong address)
        : this(name, address, 0) {}

    public ulong Address {
      get { return address_; }
    }

    public ulong Count {
      get { return count_; }
    }

    public string Name {
      get { return name_; }
    }

    #region Implementation of IDebugMemoryContext2

    public int GetName(out string pbstrName) {
      Debug.WriteLine("MemoryContext.GetName");
      pbstrName = name_;
      return VSConstants.S_OK;
    }

    public int GetInfo(enum_CONTEXT_INFO_FIELDS dwFields, CONTEXT_INFO[] pinfo) {
      Debug.WriteLine("MemoryContext.GetInfo");

      pinfo[0].bstrAddress = Address.ToString("X");
      pinfo[0].bstrAddressAbsolute = pinfo[0].bstrAddress;
      pinfo[0].bstrAddressOffset = pinfo[0].bstrAddress;
      pinfo[0].bstrFunction = "";
      pinfo[0].bstrModuleUrl = "";
      pinfo[0].dwFields = enum_CONTEXT_INFO_FIELDS.CIF_ALLFIELDS;

      return VSConstants.S_OK;
    }

    public int Add(ulong dwCount, out IDebugMemoryContext2 ppMemCxt) {
      Debug.WriteLine("MemoryContext.Add");
      ppMemCxt = new MemoryContext(name_, address_ + dwCount, count_);
      return VSConstants.S_OK;
    }

    public int Subtract(ulong dwCount, out IDebugMemoryContext2 ppMemCxt) {
      Debug.WriteLine("MemoryContext.Subtract");
      ppMemCxt = new MemoryContext(name_, address_ - dwCount, count_);
      return VSConstants.S_OK;
    }

    public int Compare(enum_CONTEXT_COMPARE Compare,
                       IDebugMemoryContext2[] rgpMemoryContextSet,
                       uint dwMemoryContextSetLen,
                       out uint pdwMemoryContext) {
      Debug.WriteLine("MemoryContext.Compare");

      // TODO(ilewis): 
      // must implement at least CONTEXT_EQUAL, CONTEXT_LESS_THAN,
      // CONTEXT_GREATER_THAN and CONTEXT_SAME_SCOPE. (MSDN)
      pdwMemoryContext = uint.MaxValue;
      return VSConstants.S_OK;
    }

    #endregion

    #region Private Implementation

    private readonly ulong address_;
    private readonly ulong count_;
    private readonly string name_;

    #endregion
  }
}