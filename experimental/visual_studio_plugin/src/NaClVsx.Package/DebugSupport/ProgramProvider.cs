// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.Shell.Interop;
using NaClVsx.DebugHelpers;

namespace Google.NaClVsx.DebugSupport {
  [ComVisible(true)]
  [Guid(kClsId)]
  [ClassInterface(ClassInterfaceType.None)]
  public class ProgramProvider : IDebugProgramProvider2 {
    #region constants

    public const string kClsId = "69303596-96E6-4741-905C-769ABCA7D550";

    #endregion

    #region Private Implementation

    private ushort locale_ = 1033; // defaults to en-US

    #endregion

    public ProgramProvider() {
      Trace.WriteLine("Entered ProgramProvider constructor");
      Trace.WriteLine(typeof(Engine).Assembly.ToString());
    }

    #region Implementation of IDebugProgramProvider2

    public int GetProviderProcessData(enum_PROVIDER_FLAGS     flags,
                                      IDebugDefaultPort2      pPort,
                                      AD_PROCESS_ID           processId,
                                      CONST_GUID_ARRAY        engineFilter,
                                      PROVIDER_PROCESS_DATA[] pProcesses) {
      Trace.WriteLine("ProgramProvider.GetProviderProcessData");
      int result = VSConstants.S_FALSE;
      pProcesses[0] = new PROVIDER_PROCESS_DATA();
/*
      if ((flags & enum_PROVIDER_FLAGS.PFLAG_GET_PROGRAM_NODES) != 0) {
        try {
          if (GdbProxy.CanConnect((int)(processId.dwProcessId))) {
            IDebugProgramNode2 node =
                (IDebugProgramNode2) new ProgramNode((int)(processId.dwProcessId));

            IntPtr[] programNodes = {
                                        Marshal.GetComInterfaceForObject(
                                            node,
                                            typeof (IDebugProgramNode2))
                                    };

            IntPtr destinationArray =
                Marshal.AllocCoTaskMem(IntPtr.Size * programNodes.Length);
            Marshal.Copy(programNodes,
                         0,
                         destinationArray,
                         programNodes.Length);

            pProcesses[0].Fields = enum_PROVIDER_FIELDS.PFIELD_PROGRAM_NODES;
            pProcesses[0].ProgramNodes.Members = destinationArray;
            pProcesses[0].ProgramNodes.dwCount = (uint) programNodes.Length;

            IDebugCoreServer3 server;
            pPort.GetServer(out server);
            
//              server.CreateInstanceInServer(typeof(Engine).Assembly.CodeBase,0, ref new Guid(Engine.ClsId),  )

            result = VSConstants.S_OK;
          }
        }
        catch (Exception e) {
          Debug.WriteLine(e.ToString());
          result = VSConstants.E_INVALIDARG;
        }
      }
 */
      return result;
    }

    public int GetProviderProgramNode(enum_PROVIDER_FLAGS Flags,
                                      IDebugDefaultPort2 pPort,
                                      AD_PROCESS_ID ProcessId,
                                      ref Guid guidEngine,
                                      ulong programId,
                                      out IDebugProgramNode2 ppProgramNode) {
                                        Trace.WriteLine("ProgramProvider.GetProviderProgramNode");
      ppProgramNode = null;
      return 0;
    }

    public int WatchForProviderEvents(enum_PROVIDER_FLAGS Flags,
                                      IDebugDefaultPort2 pPort,
                                      AD_PROCESS_ID ProcessId,
                                      CONST_GUID_ARRAY EngineFilter,
                                      ref Guid guidLaunchingEngine,
                                      IDebugPortNotify2 pEventCallback) {
      Trace.WriteLine("ProgramProvider.WatchForProviderEvents");
      return VSConstants.S_OK;
    }

    public int SetLocale(ushort wLangID) {
      Trace.WriteLine("ProgramProvider.SetLocale");
      locale_ = wLangID;
      return VSConstants.S_OK;
    }

    #endregion

  
  }
}