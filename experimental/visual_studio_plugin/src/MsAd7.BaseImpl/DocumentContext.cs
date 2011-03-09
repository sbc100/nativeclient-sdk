// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.Interfaces;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  /// <summary>
  /// For documentation of IDebugDocumentContext2 members, please see the VS 2008 MSDN
  /// documentation.
  /// </summary>
  public class DocumentContext : IDebugDocumentContext2
  {
    public DocumentContext(string path, uint line, uint column) {
      path_ = path;
      line_ = line;
      column_ = column;
    }

    public DocumentContext(DocumentPosition pos) {
      path_ = pos.Path;
      line_ = pos.BeginPos.dwLine;
      column_ = pos.BeginPos.dwColumn;
    }

    public string Path {
      get { return path_; }
      set { path_ = value; }
    }

    public uint Line {
      get { return line_; }
      set { line_ = value; }
    }

    public uint Column {
      get { return column_; }
      set { column_ = value; }
    }

    public IList<CodeContext> CodeContexts {
      get { return codeContexts_; }
    }

    /// <summary>
    /// This generator helps with conversion between addresses and code locations.
    /// </summary>
    /// <param name="addr">The address of the desired DocumentContext in the code.</param>
    /// <param name="dbg">The debugger which should be queried to obtain a code location.</param>
    /// <returns>The DocumentContext corresponding to addr or null.</returns>
    public static DocumentContext FromAddress(UInt64 addr, ISimpleDebugger dbg) {
      DocumentPosition pos = dbg.Symbols.PositionFromAddress(addr);
      if (pos == null) {
        return null;
      }

      var result = new DocumentContext(pos);

      IEnumerable<ulong> addrs = dbg.Symbols.AddressesFromPosition(pos);
      foreach (ulong a in addrs) {
        pos = dbg.Symbols.PositionFromAddress(a);
        var cc = new CodeContext(
            string.Format("{0}:{1}", pos.Path, pos.BeginPos.dwLine),
            a,
            1,
            result);
        result.AddCodeContext(cc);
      }

      return result;
    }

    /// <summary>
    /// A document context can contain of many CodeContext instances.
    /// </summary>
    /// <param name="ctx">A CodeContext to be added to this document context.</param>
    public void AddCodeContext(CodeContext ctx) {
      if (!codeContexts_.Contains(ctx)) {
        codeContexts_.Add(ctx);
      }
    }

    #region Implementation of IDebugDocumentContext2

    public int GetDocument(out IDebugDocument2 ppDocument) {
      Debug.WriteLine("DocumentContext.GetDocument");

      // According to MSDN, we don't implement this unless we supply a
      // custom document type.
      ppDocument = null;
      return VSConstants.S_FALSE;
    }

    public int GetName(enum_GETNAME_TYPE gnType, out string pbstrFileName) {
      Debug.WriteLine("DocumentContext.GetName");

      pbstrFileName = path_;
      return VSConstants.S_OK;
    }

    public int EnumCodeContexts(out IEnumDebugCodeContexts2 ppEnumCodeCxts) {
      Debug.WriteLine("DocumentContext.EnumCodeContexts");
      ppEnumCodeCxts =
          new CodeContextEnumerator(
              codeContexts_.ConvertAll(e => (IDebugCodeContext2) e));
      return VSConstants.S_OK;
    }

    public int GetLanguageInfo(ref string pbstrLanguage, ref Guid pguidLanguage) {
      Debug.WriteLine("DocumentContext.GetLanguageInfo");

      // For now, just assume C/C++
      pbstrLanguage = "C++";
      pguidLanguage = Guids.guidLanguageCpp;
      return VSConstants.S_OK;
    }

    public int GetStatementRange(TEXT_POSITION[] pBegPosition,
                                 TEXT_POSITION[] pEndPosition) {
      Debug.WriteLine("DocumentContext.GetStatementRange");
      pBegPosition[0].dwLine = line_;
      pBegPosition[0].dwColumn = column_;
      pEndPosition[0].dwLine = line_;
      pEndPosition[0].dwColumn = column_;
      return VSConstants.S_OK;
    }

    public int GetSourceRange(TEXT_POSITION[] pBegPosition,
                              TEXT_POSITION[] pEndPosition) {
      Debug.WriteLine("DocumentContext.GetSourceRange");
      pBegPosition[0].dwLine = line_;
      pBegPosition[0].dwColumn = column_;
      pEndPosition[0].dwLine = line_;
      pEndPosition[0].dwColumn = column_;
      return VSConstants.S_OK;
    }

    public int Compare(enum_DOCCONTEXT_COMPARE compare,
                       IDebugDocumentContext2[] rgpDocContextSet,
                       uint dwDocContextSetLen,
                       out uint pdwDocContext) {
      Debug.WriteLine("DocumentContext.Compare");
      throw new NotImplementedException();
    }

    public int Seek(int nCount, out IDebugDocumentContext2 ppDocContext) {
      Debug.WriteLine("DocumentContext.Seek");

      // Seek only needs to be implemented if we're trying to support a
      // nontraditional document format.
      ppDocContext = null;
      return VSConstants.E_NOTIMPL;
    }

    #endregion

    #region Private Implementation

    readonly List<CodeContext> codeContexts_ = new List<CodeContext>();
    uint column_;
    uint line_;
    string path_;

    #endregion
  }
}
