// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Collections.Generic;
using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;
using NaClVsx;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   Extends DictionaryLoader with some functionality that comes in handy for
  ///   loaders that have to parse DIE entries.
  /// </summary>
  /// <typeparam name = "TEntryType">The dictionary entry type to be parsed.
  /// </typeparam>
  public class DIECapableLoader<TEntryType> : DictionaryLoader<TEntryType> {
    /// <summary>
    ///   Builds a TreeNode representation of a DIE's Attributes.  This
    ///   function lives in the base class because it is used multiple times.
    /// </summary>
    /// <param name = "attributes">The attributes to depict.</param>
    /// <returns>The new TreeNode.</returns>
    protected static TreeNode GetDIEAttributesNode(
        Dictionary<DwarfAttribute, object> attributes) {
      var attributesNode = new TreeNode("Attributes");
      if (attributes != null) {
        foreach (var attribute in attributes) {
          var name = GetString(attribute.Key);
          var value = GetString(attribute.Value);
          attributesNode.Nodes.Add(name, string.Format("{0}: {1}", name, value));
        }
      }
      return attributesNode;
    }

    /// <summary>
    ///   Builds a TreeNode representation of a DIE's values.  This function
    ///   lives in the base class because it is used multiple times.
    /// </summary>
    /// <param name = "entry">The DIE whose values to depict.</param>
    /// <returns>A new TreeNode</returns>
    protected static TreeNode GetDIEValuesNode(DebugInfoEntry entry) {
      var valuesNode = new TreeNode("null");
      if (entry != null) {
        var valuesString = string.Format(
            "Values: Key: {0} Outer Scope: {1} Parent: {2} Tag: {3}",
            entry.Key,
            entry.OuterScope,
            entry.ParentKey,
            entry.Tag);
        valuesNode.Name = valuesString;
        valuesNode.Text = valuesString;
      }
      return valuesNode;
    }
  }
}
