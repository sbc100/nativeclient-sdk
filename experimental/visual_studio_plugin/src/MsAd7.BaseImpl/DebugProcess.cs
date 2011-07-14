// Copyright 2009 The Native Client Authors. All rights reserved.
// 
// Use of this source code is governed by a BSD-style license that can
// 
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  public abstract class DebugProcess : IDebugProcess2 {
    public DebugProcess(IDebugPort2 port, int pid, string imagePath) {
      imagePath_ = imagePath;
      pid_ = pid;
      port_ = port;
    }

    public string ImagePath {
      get { return imagePath_; }
      protected set { imagePath_ = value; }
    }

    public Guid Guid {
      get { return guid_; }
    }

    public int Pid {
      get { return pid_; }
    }

    public IDebugPort2 Port {
      get { return port_; }
    }


    #region Implementation of IDebugProcess2

    public int GetInfo(enum_PROCESS_INFO_FIELDS Fields,
                       PROCESS_INFO[] pProcessInfo) {
      Debug.WriteLine("DebugProcess.GetInfo");
      PROCESS_INFO info = new PROCESS_INFO();

      // Currently Visual Studio seems to be asking for these
      // fields:
      // PIF_FILE_NAME | PIF_BASE_NAME | PIF_TITLE 
      // | PIF_SESSION_ID | PIF_ATTACHED_SESSION_NAME | PIF_FLAGS

      if ((Fields & enum_PROCESS_INFO_FIELDS.PIF_BASE_NAME) != 0) {
        GetName(enum_GETNAME_TYPE.GN_BASENAME, out info.bstrBaseName);
        info.Fields |= enum_PROCESS_INFO_FIELDS.PIF_BASE_NAME;
      }
      if ((Fields & enum_PROCESS_INFO_FIELDS.PIF_FILE_NAME) != 0) {
        GetName(enum_GETNAME_TYPE.GN_FILENAME, out info.bstrBaseName);
        info.Fields |= enum_PROCESS_INFO_FIELDS.PIF_FILE_NAME;
      }
      if ((Fields & enum_PROCESS_INFO_FIELDS.PIF_TITLE) != 0) {
        GetName(enum_GETNAME_TYPE.GN_TITLE, out info.bstrBaseName);
        info.Fields |= enum_PROCESS_INFO_FIELDS.PIF_TITLE;
      }

      if ((Fields & enum_PROCESS_INFO_FIELDS.PIF_PROCESS_ID) != 0) {
        info.ProcessId = MakePhysicalProcessId();
        info.Fields |= enum_PROCESS_INFO_FIELDS.PIF_PROCESS_ID;
      }

      if ((Fields & enum_PROCESS_INFO_FIELDS.PIF_FLAGS) != 0) {
        info.Flags = GetProcessStatus();
        info.Fields |= enum_PROCESS_INFO_FIELDS.PIF_FLAGS;
      }

      if ((Fields & enum_PROCESS_INFO_FIELDS.PIF_SESSION_ID) != 0) {
        // We currently don't support multiple sessions, so all
        // processes are in session 1.
        info.dwSessionId = 1;
        info.Fields |= enum_PROCESS_INFO_FIELDS.PIF_SESSION_ID;
      }

      // Oddly enough, SESSION_NAME is requested... even though
      // the docs clearly state that it's deprecated.
      if ((Fields & enum_PROCESS_INFO_FIELDS.PIF_ATTACHED_SESSION_NAME) != 0) {
        info.bstrAttachedSessionName = "[Attached session name is deprecated]";
        info.Fields |= enum_PROCESS_INFO_FIELDS.PIF_ATTACHED_SESSION_NAME;
      }


      pProcessInfo[0] = info;
      return VSConstants.S_OK;
    }

    public abstract enum_PROCESS_INFO_FLAGS GetProcessStatus();

    public int EnumPrograms(out IEnumDebugPrograms2 ppEnum) {
      Debug.WriteLine("DebugProcess.EnumPrograms");
      ICollection<IDebugProgram2> programs = GetPrograms();
      ppEnum = new Ad7Enumerators.ProgramEnumerator(programs);
      return VSConstants.S_OK;
    }

    protected abstract ICollection<IDebugProgram2> GetPrograms();

    public int GetName(enum_GETNAME_TYPE gnType, out string pbstrName) {
      Debug.WriteLine("DebugProcess.GetName");
      switch (gnType) {
        case enum_GETNAME_TYPE.GN_BASENAME:
          pbstrName = Path.GetFileName(imagePath_);
          break;
        case enum_GETNAME_TYPE.GN_FILENAME:
          pbstrName = imagePath_;
          break;
        case enum_GETNAME_TYPE.GN_MONIKERNAME:
          pbstrName = imagePath_;
          break;
        case enum_GETNAME_TYPE.GN_NAME:
          pbstrName = Path.GetFileNameWithoutExtension(imagePath_);
          break;
        case enum_GETNAME_TYPE.GN_STARTPAGEURL:
          pbstrName = "[DebugProcess start page URL goes here]";
          break;
        case enum_GETNAME_TYPE.GN_TITLE:
          // This actually goes into the "process name" column in the
          // VS UI.
          pbstrName = Path.GetFileName(imagePath_);
          break;
        case enum_GETNAME_TYPE.GN_URL:
          pbstrName = "[DebugProcess URL goes here]";
          break;
        default:
          throw new ArgumentException("DebugProcess.GetName:gnType");
      }
      return VSConstants.S_OK;
    }

    public int GetServer(out IDebugCoreServer2 ppServer) {
      Debug.WriteLine("DebugProcess.GetServer");
      throw new NotImplementedException();
    }

    public int Terminate() {
      Debug.WriteLine("DebugProcess.Terminate");
      try {
        // Try to get the process, so we can kill it.
        // If the process is no longer running, GetProcess
        // will throw an ArgumentException.
        var proc = Process.GetProcessById(Pid);
        proc.Kill();        
      } catch (ArgumentException e) {
        // This happens legitimately if the program-under-debug
        // has already finished.
        // We need to catch this exception or the user gets a
        // confusing message about not being able to terminate
        // the (already finished) program-under-debug.
        Debug.WriteLine("Caught Exception {" + e.Message +
          "} in DebugProcess.cs");
      }
      return VSConstants.S_OK;
    }

    public int Attach(IDebugEventCallback2 pCallback,
                      Guid[] rgguidSpecificEngines,
                      uint celtSpecificEngines,
                      int[] rghrEngineAttach) {
      Debug.WriteLine("DebugProcess.Attach");
      throw new NotImplementedException();
    }

    public int CanDetach() {
      Debug.WriteLine("DebugProcess.CanDetach");
      throw new NotImplementedException();
    }

    public int Detach() {
      Debug.WriteLine("DebugProcess.Detach");
      throw new NotImplementedException();
    }

    public int GetPhysicalProcessId(AD_PROCESS_ID[] pProcessId) {
      Debug.WriteLine("DebugProcess.GetPhysicalProcessId");
      pProcessId[0] = MakePhysicalProcessId();
      return VSConstants.S_OK;
    }

    public int GetProcessId(out Guid pguidProcessId) {
      Debug.WriteLine("DebugProcess.GetProcessId");
      pguidProcessId = guid_;
      return VSConstants.S_OK;
    }

    public int GetAttachedSessionName(out string pbstrSessionName) {
      Debug.WriteLine("DebugProcess.GetAttachedSessionName");
      throw new NotImplementedException();
    }

    public int EnumThreads(out IEnumDebugThreads2 ppEnum) {
      Debug.WriteLine("DebugProcess.EnumThreads");
      throw new NotImplementedException();
    }

    public int CauseBreak() {
      Debug.WriteLine("DebugProcess.CauseBreak");
      throw new NotImplementedException();
    }

    public int GetPort(out IDebugPort2 ppPort) {
      Debug.WriteLine("DebugProcess.GetPort");
      ppPort = port_;
      return VSConstants.S_OK;
    }

    private AD_PROCESS_ID MakePhysicalProcessId() {
      return new AD_PROCESS_ID {
          dwProcessId = (uint) Pid,
          ProcessIdType = (uint) enum_AD_PROCESS_ID.AD_PROCESS_ID_SYSTEM
      };
    }

    #endregion

    #region Private Implementation

    private string imagePath_;

    // Although the AD_PROCESS_ID structure provides the option to
    // use a GUID pid, the MSVS UI doesn't like anything but
    // ints.
    readonly int pid_;

    readonly Guid guid_ = Guid.NewGuid();

    private readonly IDebugPort2 port_;

    #endregion
  }
}
