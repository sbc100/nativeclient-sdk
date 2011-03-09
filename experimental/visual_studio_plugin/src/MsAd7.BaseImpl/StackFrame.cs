// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.DebugProperties;
using Google.MsAd7.BaseImpl.Interfaces;
using Google.MsAd7.BaseImpl.Interfaces.SimpleSymbolTypes;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  /// <summary>
  ///   StackFrame represents a frame on the stack. It also implements
  ///   IDebugExpressionContext2, because the current stack frame determines
  ///   how expressions should be evaluated.
  /// </summary>
  public class StackFrame : IDebugStackFrame2, IDebugExpressionContext2 {
    public StackFrame(RegisterSet rset,
                      IDebugThread2 thread,
                      Module module,
                      ISimpleDebugger dbg) {
      programCounter_ = rset["RIP"];
      DocumentContext dc = DocumentContext.FromAddress(programCounter_, dbg);
      if (dc == null){
        return;
      }
      CodeContext cc = dc.CodeContexts[0];
      Function fn = dbg.Symbols.FunctionFromAddress(programCounter_);
      
      codeContext_ = cc;
      thread_ = thread;
      module_ = module;
      func_ = fn;
      dbg_ = dbg;

      rootProperty_ = new DebugPropertyBase(
        null,
        fn.Name,
        "[ROOT PROPERTY TYPE]",
        fn.Id,
        0,
        enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_METHOD,
        dbg_);

      registers_ = new DebugPropertyBase(
        rootProperty_,
        "Registers", "", "", 0, enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_NONE, dbg_);
      rset.Parent = registers_;
    }

    public Function Func {
      get { return func_; }
    }

    public CodeContext CodeContext {
      get { return codeContext_; }
    }

    public Module Module {
      get { return module_; }
    }

    public ulong ReturnAddress { get { return programCounter_;  } }


    #region IDebugExpressionContext2 Members

    public int ParseText(string pszCode,
                         enum_PARSEFLAGS dwFlags,
                         uint nRadix,
                         out IDebugExpression2 ppExpr,
                         out string pbstrError,
                         out uint pichError) {
      Debug.WriteLine("StackFrame.ParseText");

      ppExpr = null;
      pbstrError = "<unknown>";
      pichError = (uint)pbstrError.Length;

      if (properties_.ContainsKey(pszCode))
      {
        ppExpr = new SimpleExpression(properties_[pszCode]);
        pbstrError = null;
        pichError = 0;
        return VSConstants.S_OK;
      } else if (Char.IsNumber(pszCode,0)) {
        // If it's a number, we interpret it as an address
        string trimmed = pszCode.Trim();
        int radix = 16; // VS assumes hex for addresses
        if (trimmed[0] == '0') {
          if (trimmed.Length >= 2 && trimmed[1] == 'x') {
            radix = 16;
            trimmed = trimmed.Substring(2);
          } else {
            radix = 8;
          }
        }
        ulong address = Convert.ToUInt64(trimmed, radix);

        if (address != 0) {
          var memProp = new DebugPropertyBase(
              null,
              pszCode,
              "void*",
              0,
              address,
              enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_DATA,
              dbg_);
          ppExpr = new SimpleExpression(memProp);
          pbstrError = null;
          pichError = 0;
        }
        return VSConstants.S_OK;
      }

      return (ppExpr != null) ? VSConstants.S_OK : VSConstants.E_FAIL;
    }

    #endregion

    #region IDebugStackFrame2 Members

    public int GetCodeContext(out IDebugCodeContext2 ppCodeCxt) {
      Debug.WriteLine("StackFrame.GetCodeContext");
      ppCodeCxt = codeContext_;
      return VSConstants.S_OK;
    }

    public int GetDocumentContext(out IDebugDocumentContext2 ppCxt) {
      Debug.WriteLine("StackFrame.GetDocumentContext");
      codeContext_.GetDocumentContext(out ppCxt);
      return VSConstants.S_OK;
    }

    public int GetName(out string pbstrName) {
      Debug.WriteLine("StackFrame.GetName");
      pbstrName = func_.Name;
      return VSConstants.S_OK;
    }

    public int GetInfo(enum_FRAMEINFO_FLAGS dwFieldSpec,
                       uint nRadix,
                       FRAMEINFO[] pFrameInfo) {
      Debug.WriteLine("StackFrame.GetInfo");
      string fmt = (nRadix == 16) ? "X" : "D";

      /*
       * Experimental value of dwFieldSpec:
          enum_FRAMEINFO_FLAGS.FIF_FUNCNAME
          | enum_FRAMEINFO_FLAGS.FIF_LANGUAGE
          | enum_FRAMEINFO_FLAGS.FIF_STACKRANGE
          | enum_FRAMEINFO_FLAGS.FIF_FRAME
          | enum_FRAMEINFO_FLAGS.FIF_DEBUGINFO
          | enum_FRAMEINFO_FLAGS.FIF_STALECODE
          | enum_FRAMEINFO_FLAGS.FIF_FLAGS
          | enum_FRAMEINFO_FLAGS.FIF_DEBUG_MODULEP
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_FORMAT
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_ARGS
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_MODULE
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_LINES
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_OFFSET
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_ARGS_TYPES
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_ARGS_NAMES
          | enum_FRAMEINFO_FLAGS.FIF_FUNCNAME_ARGS_VALUES
          | enum_FRAMEINFO_FLAGS.FIF_FILTER_NON_USER_CODE;
      */

      pFrameInfo[0].m_addrMin = codeContext_.Address;
      pFrameInfo[0].m_addrMax = codeContext_.Address + codeContext_.Count;
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_STACKRANGE;

      pFrameInfo[0].m_bstrArgs = "";
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_ARGS;

      pFrameInfo[0].m_bstrFuncName = func_.Name;
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_FUNCNAME;

      pFrameInfo[0].m_bstrLanguage = "C++";
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_LANGUAGE;

      pFrameInfo[0].m_bstrReturnType = "void";
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_RETURNTYPE;

      pFrameInfo[0].m_bstrModule = module_.Name;
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_MODULE;

      pFrameInfo[0].m_pFrame = this;
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_FRAME;

      pFrameInfo[0].m_pModule = module_;
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_DEBUG_MODULEP;

      pFrameInfo[0].m_fHasDebugInfo = 1;
      pFrameInfo[0].m_dwValidFields |= enum_FRAMEINFO_FLAGS.FIF_DEBUGINFO;

      return VSConstants.S_OK;
    }

    public int GetPhysicalStackRange(out ulong paddrMin, out ulong paddrMax) {
      Debug.WriteLine("StackFrame.GetPhysicalStackRange");
      paddrMin = codeContext_.Address;
      paddrMax = paddrMin + codeContext_.Count;
      return VSConstants.S_OK;
    }

    public int GetExpressionContext(out IDebugExpressionContext2 ppExprCxt) {
      Debug.WriteLine("StackFrame.GetExpressionContext");
      ppExprCxt = this;
      return VSConstants.S_OK;
    }

    public int GetLanguageInfo(ref string pbstrLanguage, ref Guid pguidLanguage) {
      Debug.WriteLine("StackFrame.GetLanguageInfo");

      codeContext_.GetLanguageInfo(ref pbstrLanguage, ref pguidLanguage);

      return VSConstants.S_OK;
    }

    public int GetDebugProperty(out IDebugProperty2 ppProperty) {
      Debug.WriteLine("StackFrame.GetDebugProperty");

      ppProperty = rootProperty_;
      return VSConstants.S_OK;
    }

    public int EnumProperties(enum_DEBUGPROP_INFO_FLAGS dwFields,
                              uint nRadix,
                              ref Guid guidFilter,
                              uint dwTimeout,
                              out uint pcelt,
                              out IEnumDebugPropertyInfo2 ppEnum) {
      ppEnum = null;
      pcelt = 0;

      Debug.WriteLine("StackFrame.EnumProperties");
      if (properties_.Count < 1) {
        RefreshProperties();
      }

      DebugPropertyBase root = rootProperty_;
      if (guidFilter == Guids.guidFilterRegisters) {
        root = registers_;
      }

      root.EnumChildren(
        dwFields,
        nRadix,
        ref guidFilter,
        enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_ALL,
        "",
        uint.MaxValue,
        out ppEnum);


      if (ppEnum != null) {
        ppEnum.GetCount(out pcelt);
      }
      return VSConstants.S_OK;
    }

    private void RefreshProperties() {
      properties_.Clear();

      var symbols = dbg_.Symbols.GetSymbolsInScope(codeContext_.Address);

      foreach (var symbol in symbols) {
        var prop = new Variable(
            rootProperty_,
            symbol,
            enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_DATA,
            dbg_);
        properties_.Add(prop.Name, prop);
      }
    }

    public int GetThread(out IDebugThread2 ppThread) {
      Debug.WriteLine("StackFrame.GetThread");
      ppThread = thread_;
      return VSConstants.S_OK;
    }

    #endregion

    #region Private Implementation

    private readonly ulong programCounter_;
    private readonly CodeContext codeContext_;
    private readonly Module module_;
    private readonly IDebugThread2 thread_;
    private Function func_;
    private ISimpleDebugger dbg_;
    private DebugPropertyBase rootProperty_;
    private DebugPropertyBase registers_;
    private readonly Dictionary<string, DebugPropertyBase> properties_ = new Dictionary<string, DebugPropertyBase>();

    #endregion
  }
}
