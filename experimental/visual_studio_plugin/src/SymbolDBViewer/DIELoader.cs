// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   Specializes the DictionaryLoader for loading the DebugInfoEntries.
  /// </summary>
  public class DIELoader : DIECapableLoader<DebugInfoEntry> {
    protected override TreeNode GetTreeNode(ulong key, DebugInfoEntry die) {
      var dieNode = new TreeNode();
      var keyString = GetString(key);
      dieNode.Name = keyString;
      dieNode.Text = @"DIE: " + keyString;
      dieNode.Nodes.Add(GetDIEValuesNode(die));
      var attributesNode = GetDIEAttributesNode(die.Attributes);
      dieNode.Nodes.Add(attributesNode);
      return dieNode;
    }
  }
}
