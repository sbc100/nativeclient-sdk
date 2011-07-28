// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   Specializes the DictionaryLoader for loading the CallFrames.
  /// </summary>
  public class CallFrameLoader : DictionaryLoader<SymbolDatabase.CallFrame> {
    protected override TreeNode GetTreeNode(ulong key,
                                            SymbolDatabase.CallFrame callFrame) {
      var callFrameNode = new TreeNode();
      var keyString = GetString(key);
      callFrameNode.Name = keyString;
      callFrameNode.Text = @"Call Frame" + keyString;
      var valuesString = string.Format(
          "Values: Address: {0}",
          callFrame.Address);
      callFrameNode.Nodes.Add("Values", valuesString);
      foreach (var rule in callFrame.Rules) {
        var ruleString = string.Format(
            "Rule: Address: {0} Base Register: {1} Expression: {2} Offset: {3} RegisterId: {4} RuleType: {5}",
            rule.Address,
            rule.BaseRegister,
            rule.Expression,
            rule.Offset,
            rule.RegisterId,
            rule.RuleType);
        callFrameNode.Nodes.Add(ruleString);
      }
      return callFrameNode;
    }
  }
}
