// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using Microsoft.VisualStudio.Shell;

namespace Google.NaClVsx {
  public class DebugEngineRegistrationAttribute : RegistrationAttribute {
    public DebugEngineRegistrationAttribute() {}



    //
    // Debug Engine Properties
    // 
    // These properties correspond to all of the debug engine registry values
    // I was able to find. Not all are documented by Microsoft. Those that are,
    // I've marked with [MSDN]
    //

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for address breakpoints.
    /// </summary>
    public bool AddressBreakpoints { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero in order to always load the debug engine locally.
    /// </summary>
    public bool AlwaysLoadLocal { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool AlwaysLoadProgramProviderLocal { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool AppVerifier { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for attachment to existing programs.
    /// </summary>
    public bool Attach { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public UInt32 AutoSelectPriority { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for call stack breakpoints.
    /// </summary>
    public bool CallstackBreakpoints { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for the setting of conditional breakpoints.
    /// </summary>
    public bool ConditionalBreakpoints { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for the setting of breakpoints on changes in data.
    /// </summary>
    public bool DataBreakpoints { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public string DebugEngineClsId { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public string DebugEngineId { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for the production of a disassembly listing.
    /// </summary>
    public bool Disassembly { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for dump writing (the dumping of memory to an output device).
    /// </summary>
    public bool DumpWriting { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for Edit and Continue.
    /// Note: A custom debug engine should never set this or should always set it to 0.
    /// </summary>
    public bool EditAndContinue { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool EditAndContinueUseNativeBuilder { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool Embedded { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool EnableFuncEvalQuickAbort { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool EngineCanWatchProcess { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for exceptions.
    /// </summary>
    public bool Exceptions { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public UInt32 FuncEvalAbortLoggingLevel { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public string FuncEvalQuickAbortDlls { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public string FuncEvalQuickAbortExcludeList { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for named breakpoints 
    /// (breakpoints that break when a certain function name is called).
    /// </summary>
    public bool FunctionBreakpoints { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for the setting of
    /// "hit point" breakpoints (breakpoints that are triggered only
    /// after being hit a certain number of times).
    /// </summary>
    public bool HitCountBreakpoints { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool InterceptCurrentException { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool Interop { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for just-in-time debugging
    /// (the debugger is launched when an exception occurs in a running process).
    /// </summary>
    public bool JustInTimeDebug { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool JustMyCodeStepping { get; set; }

    /// <summary>
    /// Set to true in order to load the debug engine in-process with Visual Studio
    /// on 64-bit systems. Setting this property to false will result in the engine
    /// being loaded in-process with msvsmon.exe instead.
    /// </summary>
    public bool LoadUnderWow64 { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool LoadProgramProviderUnderWow64 { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public string Name { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool NativeInteropOk { get; set; }

    /// <summary>
    /// [MSDN] Set this to the CLSID(s) of the port supplier(s)
    /// </summary>
    public string PortSupplierClsIds { get; set; }

    /// <summary>
    /// [MSDN] Set this to the CLSID of the program provider.
    /// </summary>
    public string ProgramProviderClsId { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool Registers { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool RemoteDebugging { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool RequireFullTrustForSourceServer { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public string RuntimeClsId { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for setting
    /// the next statement (which skips execution of intermediate
    /// statements).
    /// </summary>
    public bool SetNextStatement { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool SqlClr { get; set; }

    /// <summary>
    /// [MSDN] Set to nonzero to indicate support for suspending thread
    /// execution.
    /// </summary>
    public bool SuspendThread { get; set; }

    /// <summary>
    /// 
    /// </summary>
    public bool UseShimApi { get; set; }

    public override void Register(RegistrationContext context) {
      Key key = context.CreateKey(KeyName(context));

      key.SetValue("AddressBP", AddressBreakpoints ? 1 : 0);
      key.SetValue("AlwaysLoadLocal", AlwaysLoadLocal ? 1 : 0);
      key.SetValue("AlwaysLoadProgramProviderLocal",
                   AlwaysLoadProgramProviderLocal ? 1 : 0);
      key.SetValue("AppVerifier", AppVerifier ? 1 : 0);
      key.SetValue("Attach", Attach ? 1 : 0);
      key.SetValue("AutoSelectPriority", AutoSelectPriority);
      key.SetValue("CallstackBP", CallstackBreakpoints ? 1 : 0);
      key.SetValue("ConditionalBP", ConditionalBreakpoints ? 1 : 0);
      key.SetValue("DataBP", DataBreakpoints ? 1 : 0);
      if (DebugEngineClsId != null) {
        key.SetValue("CLSID", FormatGuid(DebugEngineClsId));
      }
      key.SetValue("Disassembly", Disassembly ? 1 : 0);
      key.SetValue("DumpWriting", DumpWriting ? 1 : 0);
      key.SetValue("ENC", EditAndContinue ? 1 : 0);
      key.SetValue("EncUseNativeBuilder",
                   EditAndContinueUseNativeBuilder ? 1 : 0);
      key.SetValue("Embedded", Embedded ? 1 : 0);
      key.SetValue("EnableFuncEvalQuickAbort", EnableFuncEvalQuickAbort ? 1 : 0);
      key.SetValue("EngineCanWatchProcess", EngineCanWatchProcess ? 1 : 0);
      key.SetValue("Exceptions", Exceptions ? 1 : 0);
      key.SetValue("FunctionBP", FunctionBreakpoints ? 1 : 0);
      key.SetValue("FuncEvalAbortLoggingLevel", FuncEvalAbortLoggingLevel);
      if (FuncEvalQuickAbortDlls != null) {
        key.SetValue("FuncEvalQuickAbortDlls", FuncEvalQuickAbortDlls);
      }
      if (FuncEvalQuickAbortExcludeList != null) {
        key.SetValue("FuncEvalQuickAbortExcludeList",
                     FuncEvalQuickAbortExcludeList);
      }
      key.SetValue("FunctionBreakpoints", FunctionBreakpoints ? 1 : 0);
      key.SetValue("HitCountBP", HitCountBreakpoints ? 1 : 0);
      key.SetValue("InterceptCurrentException",
                   InterceptCurrentException ? 1 : 0);
      key.SetValue("Interop", Interop ? 1 : 0);
      key.SetValue("JITDebug", JustInTimeDebug ? 1 : 0);
      key.SetValue("JustMyCodeStepping", JustMyCodeStepping ? 1 : 0);
      key.SetValue("LoadUnderWow64", LoadUnderWow64 ? 1 : 0);
      key.SetValue("LoadProgramProviderUnderWow64",
                   LoadProgramProviderUnderWow64 ? 1 : 0);
      if (Name != null)
      {
        key.SetValue("Name", Name);
      }
      key.SetValue("NativeInteropOk", NativeInteropOk ? 1 : 0);

      // Port supplier ClsIds
      //
      string[] portSuppliers = PortSupplierClsIds.Split(',');
      if (portSuppliers.Length == 1) {
        key.SetValue("PortSupplier", FormatGuid(portSuppliers[0]));
      } else {
        Key portSuppliersKey = key.CreateSubkey("PortSupplier");
        int id = 0;
        foreach (string clsid in portSuppliers) {
          portSuppliersKey.SetValue(id.ToString(), FormatGuid(clsid));
          ++id;
        }
        portSuppliersKey.Close();
      }

      if (ProgramProviderClsId != null) {
        key.SetValue("ProgramProvider", FormatGuid(ProgramProviderClsId));
      }
      key.SetValue("Registers", Registers ? 1 : 0);
      key.SetValue("RemoteDebugging", RemoteDebugging ? 1 : 0);
      key.SetValue("RequireFullTrustForSourceServer",
                   RequireFullTrustForSourceServer ? 1 : 0);
      if (RuntimeClsId != null) {
        key.SetValue("Runtime", FormatGuid(RuntimeClsId));
      }
      key.SetValue("SetNextStatement", SetNextStatement ? 1 : 0);
      key.SetValue("SqlClr", SqlClr ? 1 : 0);
      key.SetValue("SuspendThread", SuspendThread ? 1 : 0);
      key.SetValue("UseShimApi", UseShimApi ? 1 : 0);

      key.Close();
    }

    public override void Unregister(RegistrationContext context) {
      context.RemoveKey(KeyName(context));
    }

    #region Private Implementation

    string FormatGuid(string guid) {
      return string.Format("{{{0}}}", guid.Trim());
    }

    #endregion

    #region Private Implementation

    string KeyName(RegistrationContext context) {
      return string.Format("AD7Metrics\\Engine\\{{{0}}}", DebugEngineId);
    }

    #endregion
  }
}