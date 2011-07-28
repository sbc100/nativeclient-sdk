// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#region

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using Google.NaClVsx.DebugSupport.DWARF;
using NaClVsx;

#endregion

namespace SymbolDBViewer {
  /// <summary>
  ///   So far, this is the only real class in this app.  It contains a few
  ///   controls and a TreeView of the contents of SymbolDatabase.
  ///   The display components are:
  ///   - A status display which is used to indicate load progress to the
  ///   user (Bottom).
  ///   - A fileName label which shows the name of the nexe file that is
  ///   currently loaded (Top Left).
  ///   The controls are:
  ///   - A "Load Nexe" button to pick a file to examine.
  ///   - A textfield and "Search" button to search through the Symbol Data.
  ///   - A "Close All" button to close any expanded nodes.
  ///   - A set of navigation controls to make iterating through search
  ///   results easier, consiting of a back button, forward button, and a
  ///   label to indicate the currently selected entry and the total number
  ///   of hits.
  /// </summary>
  public partial class NexeLoader : Form {
    /// <summary>
    ///   The constructor initializes the data fields for the loadStatus
    ///   indicator.
    /// </summary>
    public NexeLoader() {
      InitializeComponent();
      dieLoader_.DictionaryName = "DIEs";
      fileLoader_.DictionaryName = "Files";
      locationLoader_.DictionaryName = "Locations";
      scopeTransitionLoader_.DictionaryName = "Scope Transitions";
      locListLoader_.DictionaryName = "LocLists";
      callFrameLoader_.DictionaryName = "Call Frames";
    }

    #region Private Implementation

    /// <summary>
    ///   A Container used to store nodes that match the user's search.
    /// </summary>
    private List<TreeNode> matchingNodes_;

    /// <summary>
    ///   The index that the searchNav control cluster is currently on.
    /// </summary>
    private int searchIndex_ = -1;

    #endregion

    #region Private Implementation

    /// <summary>
    ///   Searches through the tree and expands and collects any nodes that 
    ///   match the search string.  The parents of any nodes that match are
    ///   also expanded to ensure that the search results are all visible.
    /// </summary>
    /// <param name = "nodes">The set of starting nodes to examine.</param>
    /// <param name = "searchString">The string to match against.</param>
    /// <returns>A collection of nodes that have already been expanded and
    ///   that match the search results.  In cases where parents and children
    ///   both match, only the matching children are returned.</returns>
    private static List<TreeNode> FindAndExpandMatchingNodes(
        TreeNodeCollection nodes, string searchString) {
      var matchingNodes = new List<TreeNode>();
      for (var i = 0; i < nodes.Count; i++) {
        var node = nodes[i];
        // Doesn't add itself if it already has a matching child.
        var matchingChildren = FindAndExpandMatchingNodes(
            node.Nodes, searchString);
        if (matchingChildren.Count > 0) {
          matchingNodes.AddRange(matchingChildren);
          node.Expand();
        } else if (node.Text.Contains(searchString)) {
          matchingNodes.Add(node);
          node.Expand();
        }
      }
      return matchingNodes;
    }

    /// <summary>
    ///   An event handler that can be called when the users wants to close
    ///   any open nodes in the TreeView.
    /// </summary>
    /// <param name = "sender">Unused.</param>
    /// <param name = "e">Unused.</param>
    private void CloseAllButtonClick(object sender, EventArgs e) {
      symbolDbTreeView_.CollapseAll();
    }

    /// <summary>
    ///   An event handler that can be called when the user wants to load a
    ///   new nexe into the SymbolDBViewer.
    /// </summary>
    /// <param name = "sender">Unused.</param>
    /// <param name = "e">Unused.</param>
    private void HandleLoadButtonClick(object sender, EventArgs e) {
      searchNavLabel_.Text = @"0/0";
      nexeChooserDialog_.InitialDirectory =
          Environment.GetEnvironmentVariable("NACL_VSX_ROOT");
      // Show the dialog and get result.
      var result = nexeChooserDialog_.ShowDialog();
      if (result == DialogResult.OK) {
        statusLabel_.Text = string.Format(
            "Loading :" + nexeChooserDialog_.SafeFileName);
        LoadNexeFile();
        symbolDbTreeView_.BeginUpdate();
        symbolDbTreeView_.Nodes.Clear();
        symbolDbTreeView_.EndUpdate();

        dieLoader_.Content = symbolDatabase_.Entries;
        fileLoader_.Content = symbolDatabase_.Files;
        locationLoader_.Content = symbolDatabase_.Locations;
        scopeTransitionLoader_.Content = symbolDatabase_.ScopeTransitions;
        callFrameLoader_.Content = symbolDatabase_.CallFrames;
        locListLoader_.Content = symbolDatabase_.LocLists;

        dieLoader_.RunWorkerAsync();
        fileLoader_.RunWorkerAsync();
        locationLoader_.RunWorkerAsync();
        scopeTransitionLoader_.RunWorkerAsync();
        callFrameLoader_.RunWorkerAsync();
        locListLoader_.RunWorkerAsync();
      }
      Console.WriteLine(result); // <-- For debugging use only.
    }

    /// <summary>
    ///   Checks all statuses to see if the nexe has finished loading.
    /// </summary>
    /// <returns>True if the TreeView is complete.  False, otherwise.
    /// </returns>
    private bool IsLoadingComplete() {
      return (dieLoader_.DoneLoading() &&
              fileLoader_.DoneLoading() &&
              locationLoader_.DoneLoading() &&
              scopeTransitionLoader_.DoneLoading() &&
              callFrameLoader_.DoneLoading() &&
              locListLoader_.DoneLoading());
    }


    /// <summary>
    ///   Initializes a new SymbolDatabase with a given nexe.
    /// </summary>
    /// <param name = "filePath">Location of the nexe to use.</param>
    /// <returns>true unless the nexe failed to load for some reason.
    /// </returns>
    bool LoadNexe(string filePath) {
      try {
        symbolDatabase_ = new SymbolDatabase();
        DwarfParser.DwarfParseElf(
            filePath, new DwarfReaderImpl(symbolDatabase_));
        symbolDatabase_.BuildIndices();
        return true;
      }
      catch (Exception) {
        return false;
      }
    }

    /// <summary>
    ///   Loads the file most recently selected by the user in the
    ///   FileChooser.
    /// </summary>
    void LoadNexeFile() {
      var filePath = nexeChooserDialog_.FileName;
      var fileName = nexeChooserDialog_.SafeFileName;
      if (LoadNexe(filePath)) {
        fileNameLabel_.Text = fileName + @": ";
        Refresh();
      }
    }

    /// <summary>
    ///   Updates that status area on the bottom of the screen with the
    ///   current progress in loading the SymbolData into the TreeView.
    /// </summary>
    /// <param name = "sender">The BackgroundWorker that is reporting progress.
    /// </param>
    /// <param name = "e">The progress report.</param>
    private void PopulateNodesProgressChanged(object sender,
                                              ProgressChangedEventArgs e) {
      ReprintLoadProgress();
    }

    /// <summary>
    ///   Prints the current load status for each of the tables that is]
    ///   represented in our TreeView.
    /// </summary>
    private void ReprintLoadProgress() {
      statusLabel_.Text = string.Format(
          "Loading Progress: DIEs: {0}% Files: {1}% Locations: {2}% Scope Transitions: {3}% Call Frames: {4}% Loc Lists: {5}%",
          dieLoader_.LoadPercentage,
          fileLoader_.LoadPercentage,
          locationLoader_.LoadPercentage,
          scopeTransitionLoader_.LoadPercentage,
          callFrameLoader_.LoadPercentage,
          locListLoader_.LoadPercentage);
    }

    /// <summary>
    ///   Runs a search on the symbolDbTreeView_ for any text the user has
    ///   entered in the search text box.
    /// </summary>
    /// <param name = "sender">Unused.</param>
    /// <param name = "e">Unused.</param>
    private void SearchButtonClick(object sender, EventArgs e) {
      var searchString = searchTextBox_.Text;
      symbolDbTreeView_.BeginUpdate();
      matchingNodes_ = FindAndExpandMatchingNodes(
          symbolDbTreeView_.Nodes, searchString);
      symbolDbTreeView_.EndUpdate();
      if (matchingNodes_.Count > 0) {
        searchIndex_ = 0;
        UpdateSearchNavLabel();
      } else {
        searchNavLabel_.Text = @"0/0";
      }
    }

    /// <summary>
    ///   Navigates one step forward in the search results.
    /// </summary>
    /// <param name = "sender">Unused.</param>
    /// <param name = "e">Unused.</param>
    private void SearchNavBackButtonClick(object sender, EventArgs e) {
      if (searchIndex_ > 0) {
        --searchIndex_;
        UpdateSearchNavLabel();
      }
    }

    /// <summary>
    ///   Navigates one step backward in the search results.
    /// </summary>
    /// <param name = "sender">Unused.</param>
    /// <param name = "e">Unused.</param>
    private void SearchNavForwardButtonClick(object sender, EventArgs e) {
      if (searchIndex_ < matchingNodes_.Count) {
        ++searchIndex_;
        UpdateSearchNavLabel();
      }
    }

    /// <summary>
    ///   Updates the Display to indicate which search result is currently
    ///   selected.
    /// </summary>
    private void UpdateSearchNavLabel() {
      symbolDbTreeView_.SelectedNode = matchingNodes_[searchIndex_];
      searchNavLabel_.Text = string.Format(
          "{0}/{1}", searchIndex_ + 1, matchingNodes_.Count);
    }

    /// <summary>
    ///   Updates the TreeView when a DictionaryLoader worker finishes
    ///   creating the subtree it is responsible for.
    /// </summary>
    /// <param name = "sender">The DictionaryLoader that has just finished.
    /// </param>
    /// <param name = "e">The output of the DictionaryLoader, which contains
    ///   the finished subtree.</param>
    private void UpdateTreeView(object sender,
                                RunWorkerCompletedEventArgs e) {
      ReprintLoadProgress();
      var node = e.Result as TreeNode;
      if (node != null) {
        symbolDbTreeView_.BeginUpdate();
        symbolDbTreeView_.Nodes.Add(node);
        symbolDbTreeView_.EndUpdate();
        if (IsLoadingComplete()) {
          statusLabel_.Text = @"Loading Complete.";
        }
      } else {
        throw new Exception(
            "UpdateTreeView Failed because node was null!");
      }
    }

    #endregion
  }
}
