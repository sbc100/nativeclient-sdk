// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  public class Ad7Events {
    public static int SendEvent(IDebugEventCallback2 cb,
                                IDebugEngine2 eng,
                                IDebugProcess2 proc,
                                IDebugProgram2 prog,
                                IDebugThread2 thread,
                                DebugEvent evt) {
      Guid iid = ComUtils.GuidOf(evt);
      return cb.Event(
          eng,
          proc,
          prog,
          thread,
          evt,
          ref iid,
          (uint) evt.Attributes);
    }

    #region Nested type: DebugBreakEvent

    [InheritGuid(typeof (IDebugBreakEvent2))]
    public class DebugBreakEvent : DebugEvent, IDebugBreakEvent2 {
      public DebugBreakEvent() : base(enum_EVENTATTRIBUTES.EVENT_ASYNC_STOP) {}
    }

    #endregion

    #region Nested type: DebugBreakpointErrorEvent

    [InheritGuid(typeof (IDebugBreakpointErrorEvent2))]
    public class DebugBreakpointErrorEvent
        : DebugEvent, IDebugBreakpointErrorEvent2 {
      public DebugBreakpointErrorEvent(IDebugErrorBreakpoint2 debugErrorBreakpoint)
          : base(enum_EVENTATTRIBUTES.EVENT_ASYNCHRONOUS) {
        debugErrorBreakpoint_ = debugErrorBreakpoint;
      }

      #region IDebugBreakpointErrorEvent2 Members

      public int GetErrorBreakpoint(out IDebugErrorBreakpoint2 ppErrorBP) {
        ppErrorBP = debugErrorBreakpoint_;
        return VSConstants.S_OK;
      }

      #endregion

      #region Private Implementation

      private readonly IDebugErrorBreakpoint2 debugErrorBreakpoint_;

      #endregion
    }

    #endregion

    #region Nested type: DebugEngineCreateEvent

    [InheritGuid(typeof (IDebugEngineCreateEvent2))]
    public class DebugEngineCreateEvent : DebugEvent, IDebugEngineCreateEvent2 {
      public DebugEngineCreateEvent(IDebugEngine2 engine)
          : base(enum_EVENTATTRIBUTES.EVENT_ASYNCHRONOUS) {
        engine_ = engine;
      }

      #region Implementation of IDebugEngineCreateEvent2

      public int GetEngine(out IDebugEngine2 pEngine) {
        pEngine = engine_;
        return VSConstants.S_OK;
      }

      #endregion

      #region Private Implementation

      readonly IDebugEngine2 engine_;

      #endregion
    }

    #endregion

    #region Nested type: DebugEvent

    public class DebugEvent : IDebugEvent2 {
      public DebugEvent(enum_EVENTATTRIBUTES attributes) {
        Attributes = attributes;
      }

      public enum_EVENTATTRIBUTES Attributes { get; set; }

      #region Implementation of IDebugEvent2

      public int GetAttributes(out uint pdwAttrib) {
        pdwAttrib = (uint) Attributes;
        return VSConstants.S_OK;
      }

      #endregion
    }

    #endregion

    #region Nested type: DebugLoadCompleteEvent

    [InheritGuid(typeof (IDebugLoadCompleteEvent2))]
    public class DebugLoadCompleteEvent : DebugEvent, IDebugLoadCompleteEvent2 {
      public DebugLoadCompleteEvent()
          : base(enum_EVENTATTRIBUTES.EVENT_STOPPING) {}
    }

    #endregion

    #region Nested type: DebugModuleLoadEvent

    [InheritGuid(typeof (IDebugModuleLoadEvent2))]
    public class DebugModuleLoadEvent : DebugEvent, IDebugModuleLoadEvent2 {
      public DebugModuleLoadEvent(IDebugModule2 module,
                                  string msg,
                                  bool isLoading)
          : base(enum_EVENTATTRIBUTES.EVENT_IMMEDIATE) {
        module_ = module;
        msg_ = msg;
        isLoading_ = isLoading;
      }

      #region Implementation of IDebugModuleLoadEvent2

      public int GetModule(out IDebugModule2 pModule,
                           ref string pbstrDebugMessage,
                           ref int pbLoad) {
        pModule = module_;
        pbstrDebugMessage = msg_;
        pbLoad = isLoading_ ? 1 : 0;
        return VSConstants.S_OK;
      }

      #endregion

      #region Private Implementation

      readonly bool isLoading_; // false indicates module is unloading

      readonly IDebugModule2 module_;
      readonly string msg_;

      #endregion
    }

    #endregion

    #region Nested type: DebugProgramCreateEvent

    [InheritGuid(typeof (IDebugProgramCreateEvent2))]
    public class DebugProgramCreateEvent
        : DebugEvent,
          IDebugProgramCreateEvent2 {
      public DebugProgramCreateEvent(enum_EVENTATTRIBUTES attributes)
          : base(attributes) {}
          }

    #endregion

    #region Nested type: DebugProgramDestroyEvent

    [InheritGuid(typeof (IDebugProgramDestroyEvent2))]
    public class DebugProgramDestroyEvent
        : DebugEvent,
          IDebugProgramDestroyEvent2 {
      public DebugProgramDestroyEvent(uint exitCode)
          : base(enum_EVENTATTRIBUTES.EVENT_ASYNCHRONOUS) {
        exitCode_ = exitCode;
      }

      #region Implementation of IDebugProgramDestroyEvent2

      public int GetExitCode(out uint pdwExit) {
        pdwExit = exitCode_;
        return VSConstants.S_OK;
      }

      #endregion

      #region Private Implementation

      private readonly uint exitCode_;

      #endregion
          }

    #endregion

    #region Nested type: DebugStepCompleteEvent

    [InheritGuid(typeof (IDebugStepCompleteEvent2))]
    public class DebugStepCompleteEvent : DebugEvent, IDebugStepCompleteEvent2 {
      public DebugStepCompleteEvent()
          : base(enum_EVENTATTRIBUTES.EVENT_ASYNC_STOP) {}
    }

    #endregion

    #region Nested type: DebugSymbolSearchEvent

    [InheritGuid(typeof (IDebugSymbolSearchEvent2))]
    public class DebugSymbolSearchEvent : DebugEvent, IDebugSymbolSearchEvent2 {
      public DebugSymbolSearchEvent(IDebugModule3 module,
                                    string msg,
                                    enum_MODULE_INFO_FLAGS flags)
          : base(enum_EVENTATTRIBUTES.EVENT_IMMEDIATE) {
        module_ = module;
        msg_ = msg;
        flags_ = flags;
      }

      #region Implementation of IDebugSymbolSearchEvent2

      public int GetSymbolSearchInfo(out IDebugModule3 pModule,
                                     ref string pbstrDebugMessage,
                                     enum_MODULE_INFO_FLAGS[] pdwModuleInfoFlags) {
        pModule = module_;
        pbstrDebugMessage = msg_;
        pdwModuleInfoFlags[0] = flags_;

        return VSConstants.S_OK;
      }

      #endregion

      #region Private Implementation

      private readonly enum_MODULE_INFO_FLAGS flags_;

      private readonly IDebugModule3 module_;
      private readonly string msg_;

      #endregion
    }

    #endregion
  }
}
