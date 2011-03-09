// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Google.MsAd7.BaseImpl.Ad7Enumerators;
using Google.MsAd7.BaseImpl.Interfaces;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Debugger.Interop;

namespace Google.MsAd7.BaseImpl {
  public class DebugPropertyBase : IDebugProperty2 {
    public DebugPropertyBase(
                    DebugPropertyBase parent,
                    string name,
                    string typeName,
                    object value,
                    ulong address,
                    enum_DBG_ATTRIB_FLAGS attributes,
                    ISimpleDebugger dbg) {
      name_ = name;
      typeName_ = typeName;
      value_ = value;
      address_ = address;
      attributes_ = attributes;
      Parent = parent;
      dbg_ = dbg;
    }

    public string Name {
      get { return name_; }
      protected set { name_ = value; }
    }

    public string TypeName {
      get { return typeName_; }
      protected set { typeName_ = value; }
    }

    public object Value
    {
      get {
        RefreshValue(ref value_);
        return value_;
      }
      set { value_ = value; }
    }

    public ulong Address {
      get { return address_; }
      protected set { address_ = value; }
    }

    public uint Size {
      get { return size_; }
      protected set { size_ = value; }
    }

    public ISimpleDebugger Dbg {
      get { return dbg_; }
      protected set { dbg_ = value; }
    }

    public enum_DBG_ATTRIB_FLAGS Attributes
    {
      get { return attributes_; }
      protected set { attributes_ = value; }
    }

    public DebugPropertyBase Parent {
      get { return parent_; }
      set {
        if (parent_ != null) {
          parent_.Children.Remove(this);
        }
        parent_ = value;
        if (parent_ != null) {
          parent_.Children.Add(this);
        }
      }
    }

    public List<DebugPropertyBase> Children {
      get { return children_; }
    }

    public virtual string FormatValue() {
      return Convert.ToString(Value);
    }

    public override string ToString()
    {
      return string.Format("{0} = {1}", Name, Value);
    }

    #region Implementation of IDebugProperty2

    public int GetPropertyInfo(enum_DEBUGPROP_INFO_FLAGS dwFields,
                               uint dwRadix,
                               uint dwTimeout,
                               IDebugReference2[] rgpArgs,
                               uint dwArgCount,
                               DEBUG_PROPERTY_INFO[] pPropertyInfo) {
      Debug.WriteLine("DebugPropertyBase.GetPropertyInfo");

      pPropertyInfo[0].dwFields = 0;

      if (dwFields.HasFlag(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_FULLNAME)) {
        DebugPropertyBase parent = Parent;
        Stack<DebugPropertyBase> parents = new Stack<DebugPropertyBase>();
        while (parent != null) {
          parents.Push(parent);
          parent = parent.parent_;
        }

        StringBuilder sb = new StringBuilder();
        foreach (DebugPropertyBase parentProp in parents) {
          sb.AppendFormat("{0}.", parentProp.Name);
        }
        sb.Append(Name);

        pPropertyInfo[0].bstrFullName = sb.ToString();

        pPropertyInfo[0].dwFields |=
            enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_FULLNAME;
      }
      if (dwFields.HasFlag(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_NAME)) {
        pPropertyInfo[0].bstrName = Name;
        pPropertyInfo[0].dwFields |=
            enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_NAME;
      }
      if (dwFields.HasFlag(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_TYPE)) {
        pPropertyInfo[0].bstrType = TypeName;
        pPropertyInfo[0].dwFields |=
            enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_TYPE;
      }
      if (dwFields.HasFlag(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_PROP)) {
        pPropertyInfo[0].pProperty = this;
        pPropertyInfo[0].dwFields |=
            enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_PROP;
      }
      if (dwFields.HasFlag(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_ATTRIB)) {
        pPropertyInfo[0].dwAttrib = attributes_;
        pPropertyInfo[0].dwFields |=
            enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_ATTRIB;
      }
      if (dwFields.HasFlag(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_VALUE)
        && Value != null) {
        pPropertyInfo[0].bstrValue = FormatValue();
        pPropertyInfo[0].dwFields |=
            enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_VALUE;
      }

      return VSConstants.S_OK;
    }

    public virtual int SetValueAsString(string pszValue,
                                        uint dwRadix,
                                        uint dwTimeout) {
      Debug.WriteLine("DebugPropertyBase.SetValueAsString");
      throw new NotImplementedException();
    }

    public virtual int SetValueAsReference(IDebugReference2[] rgpArgs,
                                           uint dwArgCount,
                                           IDebugReference2 pValue,
                                           uint dwTimeout) {
      Debug.WriteLine("DebugPropertyBase.SetValueAsReference");
      throw new NotImplementedException();
    }

    public int EnumChildren(enum_DEBUGPROP_INFO_FLAGS dwFields,
                            uint dwRadix,
                            ref Guid guidFilter,
                            enum_DBG_ATTRIB_FLAGS dwAttribFilter,
                            string pszNameFilter,
                            uint dwTimeout,
                            out IEnumDebugPropertyInfo2 ppEnum) {
      Debug.WriteLine("DebugPropertyBase.EnumChildren");

      List<DEBUG_PROPERTY_INFO> props = new List<DEBUG_PROPERTY_INFO>();
      DEBUG_PROPERTY_INFO[] info = new DEBUG_PROPERTY_INFO[1];

      foreach (DebugPropertyBase property in Children)
      {
        property.GetPropertyInfo(dwFields, dwRadix, dwTimeout, null, 0, info);
        props.Add(info[0]);
      }

      ppEnum = new PropertyEnumerator(props);
      return VSConstants.S_OK;
    }

    public int GetParent(out IDebugProperty2 ppParent) {
      Debug.WriteLine("DebugPropertyBase.GetParent");
      ppParent = Parent;
      return VSConstants.S_OK;
    }

    public int GetDerivedMostProperty(out IDebugProperty2 ppDerivedMost) {
      Debug.WriteLine("DebugPropertyBase.GetDerivedMostProperty");
      throw new NotImplementedException();
    }

    public int GetMemoryBytes(out IDebugMemoryBytes2 ppMemoryBytes) {
      Debug.WriteLine("DebugPropertyBase.GetMemoryBytes");
      ppMemoryBytes = new MemoryBytes(dbg_, Address, Size);
      return VSConstants.S_OK;
    }

    public int GetMemoryContext(out IDebugMemoryContext2 ppMemory) {
      Debug.WriteLine("DebugPropertyBase.GetMemoryContext");
      ppMemory = new MemoryContext(Name, Address, Size);
      return VSConstants.S_OK;
    }

    public int GetSize(out uint pdwSize) {
      Debug.WriteLine("DebugPropertyBase.GetSize");
      pdwSize = size_;
      return VSConstants.S_OK;
    }

    public int GetReference(out IDebugReference2 ppReference) {
      Debug.WriteLine("DebugPropertyBase.GetReference");
      throw new NotImplementedException();
    }

    public int GetExtendedInfo(ref Guid guidExtendedInfo,
                               out object pExtendedInfo) {
      Debug.WriteLine("DebugPropertyBase.GetExtendedInfo");
      throw new NotImplementedException();
    }

    #endregion

    protected virtual void RefreshValue(ref object value) {
      // base impl does nothing
    }

    #region Private Implementation

    private enum_DBG_ATTRIB_FLAGS attributes_;

    private List<DebugPropertyBase> children_ = new List<DebugPropertyBase>();
    private DebugPropertyBase parent_;
    private object value_ = null;
    private ulong address_ = 0;
    private uint size_ = 0;
    private string name_;
    private string typeName_;
    private ISimpleDebugger dbg_;

    #endregion
  }
}
