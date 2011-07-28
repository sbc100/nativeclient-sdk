// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   Specializes the DictionaryLoader for loading the Scope Transitions.
  /// </summary>
  public class ScopeTransitionLoader
      : DIECapableLoader<SymbolDatabase.ScopeTransition> {
    protected override TreeNode GetTreeNode(ulong key,
                                            SymbolDatabase.ScopeTransition
                                                scopeTransition) {
      var scopeTransitionNode = new TreeNode();
      var keyString = GetString(key);
      scopeTransitionNode.Name = keyString;
      scopeTransitionNode.Text = @"Scope Transition: " + keyString;
      var valuesString = string.Format(
          "Values: Address: {0}",
          scopeTransition.Address);
      scopeTransitionNode.Nodes.Add("Values", valuesString);
      var dieNode = new TreeNode("DIE");
      if (scopeTransition.Entry != null) {
        var dieValueNode = GetDIEValuesNode(scopeTransition.Entry);
        var dieAttributesNode =
            GetDIEAttributesNode(scopeTransition.Entry.Attributes);
        dieNode.Nodes.Add(dieValueNode);
        dieNode.Nodes.Add(dieAttributesNode);
      }
      scopeTransitionNode.Nodes.Add(dieNode);
      return scopeTransitionNode;
    }
      }
}
