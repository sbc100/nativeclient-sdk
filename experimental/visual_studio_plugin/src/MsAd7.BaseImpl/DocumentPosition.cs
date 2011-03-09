using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl
{
  public class DocumentPosition : IDebugDocumentPosition2
  {

    public DocumentPosition(IDebugDocumentPosition2 pos) {
      pos.GetDocument(out doc_);
      pos.GetFileName(out path_);
      TEXT_POSITION[] begin = new TEXT_POSITION[1];
      TEXT_POSITION[] end = new TEXT_POSITION[1];
      pos.GetRange(begin, end);
      beginPos_ = begin[0];
      endPos_ = end[0];
    }

    public DocumentPosition(string path, uint line) {
      path_ = path;
      beginPos_.dwLine = line;
      beginPos_.dwColumn = 0;
      endPos_ = beginPos_;
    }

    public string Path {
      get { return path_; }
      set { path_ = value; }
    }

    public TEXT_POSITION BeginPos {
      get { return beginPos_; }
      set { beginPos_ = value; }
    }

    public TEXT_POSITION EndPos {
      get { return endPos_; }
      set { endPos_ = value; }
    }

    public IDebugDocument2 Doc {
      get { return doc_; }
      set { doc_ = value; }
    }

    public override bool Equals(object obj)
    {
      DocumentPosition other = obj as DocumentPosition;
      bool result = (other != null);
      result = result && (path_ == other.path_);
      result = result && (beginPos_.Equals(other.beginPos_));
      result = result && (endPos_.Equals(other.endPos_));
      return result;
    }

    public override int GetHashCode()
    {
      Int64 accumulator = path_.GetHashCode() << 32 | beginPos_.GetHashCode();
      accumulator = accumulator.GetHashCode() << 32 | endPos_.GetHashCode();
      return accumulator.GetHashCode();
    }

    #region Implementation of IDebugDocumentPosition2

    public int GetFileName(out string pbstrFileName) {
      Debug.WriteLine("DocumentPosition.GetFileName");
      pbstrFileName = path_;
      return VSConstants.S_OK;
    }

    public int GetDocument(out IDebugDocument2 ppDoc) {
      Debug.WriteLine("DocumentPosition.GetDocument");
      ppDoc = doc_;
      return VSConstants.S_OK;
    }

    public int IsPositionInDocument(IDebugDocument2 pDoc) {
      Debug.WriteLine("DocumentPosition.IsPositionInDocument");
      throw new NotImplementedException();
    }

    public int GetRange(TEXT_POSITION[] pBegPosition, TEXT_POSITION[] pEndPosition) {
      Debug.WriteLine("DocumentPosition.GetRange");
      pBegPosition[0] = beginPos_;
      pEndPosition[0] = endPos_;
      return VSConstants.S_OK;
    }

    #endregion

    private string path_;
    private TEXT_POSITION beginPos_ = new TEXT_POSITION();
    private TEXT_POSITION endPos_ = new TEXT_POSITION();
    private IDebugDocument2 doc_ = null;
  }
}
