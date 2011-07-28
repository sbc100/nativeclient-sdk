// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   Specializes the DictionaryLoader for loading the Source Files.
  /// </summary>
  public class SourceFileLoader : DictionaryLoader<SymbolDatabase.SourceFile> {
    protected override TreeNode GetTreeNode(ulong key,
                                            SymbolDatabase.SourceFile file) {
      var fileNode = new TreeNode();
      var keyString = GetString(key);
      fileNode.Name = keyString;
      fileNode.Text = @"File: " + keyString;
      var valuesString = string.Format(
          "Values: Key: {0} FileName: {1} RelativePath: {2} AbsolutePath: {3}",
          file.Key,
          file.Filename,
          file.RelativePath,
          file.CurrentAbsolutePath);
      fileNode.Nodes.Add("Values", valuesString);
      return fileNode;
    }
  }
}
