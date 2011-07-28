// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Windows.Forms;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   A slightly modified BackgroundWorker which adds an ID field to help
  ///   us tell them apart later.  As the name indicates, this worker is meant
  ///   to be used to load a table from the SymbolDatabase into the
  ///   SymbolDbTreeView_.
  /// </summary>
  public class DictionaryLoader<TEntryType> : BackgroundWorker {
    /// <summary>
    ///   Simple Constructor.  -1 is used here to indicate an uninitialized
    ///   ID.
    /// </summary>
    public DictionaryLoader() {
      LoadPercentage = 0;
      DictionaryName = "";
      Content = null;
    }

    /// <summary>
    ///   Can't be read-only because of constraints that C# places on the
    ///   constructor signature.
    /// </summary>
    public int LoadPercentage { get; set; }

    public string DictionaryName { get; set; }
    public Dictionary<ulong, TEntryType> Content { get; set; }

    public bool DoneLoading() {
      return (LoadPercentage == 100);
    }

    /// <summary>
    ///   Creates a Tree to represent the entries from a Dictionary stored in
    ///   the SymbolDatabase.
    /// </summary>
    /// <param name = "sender">Used to report Progress.</param>
    /// <param name = "e">Used to store and return the resulting Tree.</param>
    public void PopulateTree(object sender, DoWorkEventArgs e) {
      if (Content != null) {
        var rootNode = new TreeNode(DictionaryName);
        var nodes = rootNode.Nodes;
        for (var i = 0; i < Content.Count; ++i) {
          var entry = Content.ElementAt(i);
          nodes.Add(GetTreeNode(entry.Key, entry.Value));

          if (i % 100 == 0) {
            LoadPercentage = (i * 100) / Content.Count;
            ReportProgress(LoadPercentage);
          }
        }
        LoadPercentage = 100;
        e.Result = rootNode;
      }
    }

    /// <summary>
    ///   Implements the TreeNode generator for EntryType.  This function is
    ///   to be implemented by the child class.
    /// </summary>
    /// <param name = "die">The object which should be represented as a tree
    ///   node.</param>
    /// <returns>A new TreeNode; an empty one if something went wrong.
    /// </returns>
    protected virtual TreeNode GetTreeNode(ulong key, TEntryType entry) {
      return new TreeNode();
    }


    /// <summary>
    ///   Formats a single value of any type into a string.
    /// </summary>
    /// <typeparam name = "TArgType">The type.</typeparam>
    /// <param name = "arg">The value.</param>
    /// <returns>The string.</returns>
    protected static string GetString<TArgType>(TArgType arg) {
      return string.Format("{0}", arg);
    }
  }
}
