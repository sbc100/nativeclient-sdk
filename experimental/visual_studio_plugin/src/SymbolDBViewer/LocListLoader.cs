// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Collections.Generic;
using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   Specializes the DictionaryLoader for loading the Location Lists.
  /// </summary>
  public class LocListLoader
      : DictionaryLoader<List<SymbolDatabase.LocListEntry>> {
    protected override TreeNode GetTreeNode(ulong key,
                                            List<SymbolDatabase.LocListEntry>
                                                locList) {
      var locListNode = new TreeNode();
      var keyString = GetString(key);
      locListNode.Name = keyString;
      locListNode.Text = @"LocList: " + keyString;

      foreach (var loc in locList) {
        var locString = string.Format(
            "Rule: Start Address: {0} End Address: {1} Data: {2}",
            loc.StartAddress,
            loc.EndAddress,
            loc.Data);
        locListNode.Nodes.Add(locString, locString);
      }
      return locListNode;
    }
      }
}
