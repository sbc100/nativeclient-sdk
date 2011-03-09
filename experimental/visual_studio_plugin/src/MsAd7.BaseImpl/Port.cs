// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;

namespace Google.MsAd7.BaseImpl {
  public abstract class Port
      : IDebugPort2,
        IDebugPortNotify2,
        IConnectionPointContainer,
        IConnectionPoint {
    public Port(IDebugPortSupplier2 supplier,
                IDebugPortRequest2 request,
                Guid guid) {
      request_ = request;
      supplier_ = supplier;
      guid_ = guid;
    }

    public Guid Guid {
      get { return guid_; }
    }

    public IDebugPortRequest2 Request {
      get { return request_; }
    }

    public IDebugPortSupplier2 Supplier {
      get { return supplier_; }
    }

    public DebugProcess CreateProcess(ProcessStartInfo psi)
    {
      DebugProcess proc = CreateProcessInternal(psi);
      return proc;
    }

    #region Implementation of IDebugPort2

    public int GetPortName(out string pbstrName) {
      Debug.WriteLine("Port.GetPortName");
      return request_.GetPortName(out pbstrName);
    }

    public int GetPortId(out Guid pguidPort) {
      Debug.WriteLine("Port.GetPortId");
      pguidPort = guid_;
      return VSConstants.S_OK;
    }

    public int GetPortRequest(out IDebugPortRequest2 ppRequest) {
      Debug.WriteLine("Port.GetPortRequest");
      ppRequest = request_;
      return VSConstants.S_OK;
    }

    public int GetPortSupplier(out IDebugPortSupplier2 ppSupplier) {
      Debug.WriteLine("Port.GetPortSupplier");
      ppSupplier = supplier_;
      return VSConstants.S_OK;
    }

    public int GetProcess(AD_PROCESS_ID ProcessId, out IDebugProcess2 ppProcess) {
      Debug.WriteLine("Port.GetProcess");
      Debug.Assert(
          ProcessId.ProcessIdType ==
          (int) enum_AD_PROCESS_ID.AD_PROCESS_ID_SYSTEM);

      IEnumerable<DebugProcess> procList = GetProcesses();
      var proc = from p in procList
                 where p.Pid == ProcessId.dwProcessId
                 select p;
      ppProcess = proc.FirstOrDefault();
      return ppProcess != null ? VSConstants.S_OK : VSConstants.S_FALSE;
    }

    public int EnumProcesses(out IEnumDebugProcesses2 ppEnum) {
      Debug.WriteLine("Port.EnumProcesses");
      IEnumerable<DebugProcess> procList = GetProcesses();
      var processes = new List<IDebugProcess2>();
      foreach (var debugProcess in procList) {
        processes.Add(debugProcess);
      }
      ppEnum = new Ad7Enumerators.ProcessEnumerator(processes);
      return VSConstants.S_OK;
    }

    #endregion

    #region IConnectionPoint Members

    public void GetConnectionInterface(out Guid pIID) {
      pIID = typeof (IDebugPortEvents2).GUID;
    }

    public void GetConnectionPointContainer(out IConnectionPointContainer ppCPC) {
      ppCPC = this;
    }

    public void Advise(object pUnkSink, out uint pdwCookie) {
      pdwCookie = eventSinks_.Add(pUnkSink);
    }

    public void Unadvise(uint dwCookie) {
      eventSinks_.RemoveAt(dwCookie);
    }

    public void EnumConnections(out IEnumConnections ppEnum) {
      throw new NotImplementedException();
    }

    #endregion

    #region IConnectionPointContainer Members

    public void EnumConnectionPoints(out IEnumConnectionPoints ppEnum) {
      // This doesn't need to be implemented; all we care about
      // is FindConnectionPoint().
      throw new NotImplementedException();
    }

    public void FindConnectionPoint(ref Guid riid, out IConnectionPoint ppCP) {
      ppCP = null;
      if (riid == typeof (IDebugPortEvents2).GUID) {
        ppCP = this;
      }
    }

    #endregion

    #region IDebugPortNotify2 Members

    public int AddProgramNode(IDebugProgramNode2 pProgramNode) {
      IDebugProcess2 proc;
      AD_PROCESS_ID[] pid = new AD_PROCESS_ID[1];
      ComUtils.RequireOk(pProgramNode.GetHostPid(pid));
      ComUtils.RequireOk(GetProcess(pid[0], out proc));

      // Our implementation conflates ProgramNode and Program,
      // perhaps erroneously.
      IDebugProgram2 program = (IDebugProgram2) pProgramNode;

      SendEvent(null, this, proc, program, new Ad7Events.DebugProgramCreateEvent(enum_EVENTATTRIBUTES.EVENT_IMMEDIATE));
      return VSConstants.S_OK;
    }

    public int RemoveProgramNode(IDebugProgramNode2 pProgramNode) {
      throw new NotImplementedException();
    }

    #endregion

    #region Private Implementation

    private void SendEvent(IDebugCoreServer2 server, IDebugPort2 port, IDebugProcess2 process, IDebugProgram2 program, IDebugEvent2 ev) {
      Guid iid = ComUtils.GuidOf(ev);
      foreach (var eventSink in eventSinks_) {
        IDebugPortEvents2 events = eventSink as IDebugPortEvents2;
        if (events != null) {
          events.Event(server, port, process, program, ev, ref iid);
        }
      }
    }

    private readonly Guid guid_;

    private readonly IDebugPortRequest2 request_;
    private readonly IDebugPortSupplier2 supplier_;

    private EventSinkCollection eventSinks_ = new EventSinkCollection();

    #endregion

    protected abstract IEnumerable<DebugProcess> GetProcesses();

    protected abstract DebugProcess CreateProcessInternal(ProcessStartInfo psi);
  }
}
