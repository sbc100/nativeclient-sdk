// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Diagnostics;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  public class CodeContext : MemoryContext, IDebugCodeContext2 {
    public CodeContext(string name,
                       ulong address,
                       ulong count,
                       DocumentContext docContext)
        : base(name, address, count) {
      docContext_ = docContext;
      if (docContext_ != null) {
        docContext_.AddCodeContext(this);
      }
    }

    public static readonly CodeContext NullContext = 
      new CodeContext("<No symbols available>", 0, 0, null);

    #region Implementation of IDebugCodeContext2

    public int GetDocumentContext(out IDebugDocumentContext2 ppSrcCxt) {
      Debug.WriteLine("CodeContext.GetDocumentContext");

      ppSrcCxt = docContext_;
      return VSConstants.S_OK;
    }

    public int GetLanguageInfo(ref string pbstrLanguage, ref Guid pguidLanguage) {
      Debug.WriteLine("CodeContext.GetLanguageInfo");

      return docContext_.GetLanguageInfo(ref pbstrLanguage, ref pguidLanguage);
    }

    #endregion

    #region Private Implementation

    private readonly DocumentContext docContext_;

    #endregion
  }
}