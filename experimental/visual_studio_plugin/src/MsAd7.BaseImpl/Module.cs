using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.OLE.Interop;

namespace Google.MsAd7.BaseImpl
{
  public class Module : IDebugModule3
  {
    public string Name {
      get { return name_; }
      set { name_ = value; }
    }

    public string Url {
      get { return url_; }
      set { url_ = value; }
    }

    public string Version {
      get { return version_; }
      set { version_ = value; }
    }

    public string DebugMessage {
      get { return debugMessage_; }
      set { debugMessage_ = value; }
    }

    public ulong LoadAddress {
      get { return loadAddress_; }
      set { loadAddress_ = value; }
    }

    public ulong PreferredLoadAddress {
      get { return preferredLoadAddress_; }
      set { preferredLoadAddress_ = value; }
    }

    public uint Size {
      get { return size_; }
      set { size_ = value; }
    }

    public uint LoadOrder {
      get { return loadOrder_; }
      set { loadOrder_ = value; }
    }

    public FILETIME TimeStamp {
      get { return timeStamp_; }
      set { timeStamp_ = value; }
    }

    public string UrlSymbolLocation {
      get { return urlSymbolLocation_; }
      set { urlSymbolLocation_ = value; }
    }

    public enum_MODULE_FLAGS ModuleFlags {
      get { return moduleFlags_; }
      set { moduleFlags_ = value; }
    }

    #region Implementation of IDebugModule2

    public int GetInfo(enum_MODULE_INFO_FIELDS dwFields, MODULE_INFO[] pinfo) {
      Debug.WriteLine("Module.GetInfo");

      pinfo[0].m_addrLoadAddress = loadAddress_;
      pinfo[0].m_addrPreferredLoadAddress = preferredLoadAddress_;
      pinfo[0].m_bstrDebugMessage = debugMessage_;
      pinfo[0].m_bstrName = name_;
      pinfo[0].m_bstrUrl = url_;
      pinfo[0].m_bstrUrlSymbolLocation = urlSymbolLocation_;
      pinfo[0].m_bstrVersion = version_;
      pinfo[0].m_dwLoadOrder = loadOrder_;
      pinfo[0].m_dwModuleFlags = moduleFlags_;
      pinfo[0].m_dwSize = size_;
      pinfo[0].dwValidFields = enum_MODULE_INFO_FIELDS.MIF_ALLFIELDS;

      return VSConstants.S_OK;
    }

    public int ReloadSymbols_Deprecated(string pszUrlToSymbols, out string pbstrDebugMessage) {
      Debug.WriteLine("Module.ReloadSymbols_Deprecated");
      throw new NotImplementedException();
    }

    public int GetSymbolInfo(enum_SYMBOL_SEARCH_INFO_FIELDS dwFields, MODULE_SYMBOL_SEARCH_INFO[] pinfo) {
      Debug.WriteLine("Module.GetSymbolInfo");
      throw new NotImplementedException();
    }

    public int LoadSymbols() {
      Debug.WriteLine("Module.LoadSymbols");
      throw new NotImplementedException();
    }

    public int IsUserCode(out int pfUser) {
      Debug.WriteLine("Module.IsUserCode");
      pfUser = 1;
      return VSConstants.S_OK;
    }

    public int SetJustMyCodeState(int fIsUserCode) {
      Debug.WriteLine("Module.SetJustMyCodeState");
      throw new NotImplementedException();
    }

    #endregion

    private string name_ = "<unknown>";
    private string url_ = "<unknown>";
    private string version_ = "<unknown>";
    private string debugMessage_ = "<unknown>";
    private ulong loadAddress_ = 0x0000000c00000000;
    private ulong preferredLoadAddress_ = 0;
    private uint size_ = 0;
    private uint loadOrder_ = 0;
    private FILETIME timeStamp_ = new FILETIME();
    private string urlSymbolLocation_ = "<unknown>";
    private enum_MODULE_FLAGS moduleFlags_ = 0;

  }
}
