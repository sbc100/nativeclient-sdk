// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   Specializes the DictionaryLoader for loading the Source Locations.
  /// </summary>
  public class SourceLocationLoader
      : DictionaryLoader<SymbolDatabase.SourceLocation> {
    protected override TreeNode GetTreeNode(ulong key,
                                            SymbolDatabase.SourceLocation
                                                location) {
      var locationNode = new TreeNode();
      var keyString = GetString(key);
      locationNode.Name = keyString;
      locationNode.Text = @"Location: " + keyString;
      var valuesString = string.Format(
          "Values: Address: {0} SourceFile: {1} Line: {2} Column: {3} Length: {4}",
          location.StartAddress,
          location.SourceFileKey,
          location.Line,
          location.Column,
          location.Length);
      locationNode.Nodes.Add("Values", valuesString);
      return locationNode;
    }
      }
}
