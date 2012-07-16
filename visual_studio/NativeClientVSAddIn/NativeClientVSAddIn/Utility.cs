// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;
  using System.Collections.Generic;

  using EnvDTE;
  using Microsoft.VisualStudio.VCProjectEngine;

  /// <summary>
  /// Contains helper functions for this add-in.
  /// </summary>
  public static class Utility
  {
    /// <summary>
    /// Tells us if the given project is a Visual C/C++ project.
    /// </summary>
    /// <param name="proj">Project to check.</param>
    /// <returns>True if project is a Visual C/C++ project.</returns>
    public static bool IsVisualCProject(Project proj)
    {
      string projectType = proj.Properties.Item("Kind").Value as string;
      return projectType == "VCProject";
    }

    /// <summary>
    /// Given a generic project, checks that it is a Visual C project, and
    /// extracts the active VCConfiguration object.
    /// </summary>
    /// <param name="proj">Generic project object.</param>
    /// <returns>The active configuration, or null if failure.</returns>
    public static VCConfiguration GetActiveVCConfiguration(Project proj)
    {
      if (!IsVisualCProject(proj))
      {
        return null;
      }

      VCProject vcproj = (VCProject)proj.Object;
      IVCCollection configs = vcproj.Configurations;
      Configuration active = proj.ConfigurationManager.ActiveConfiguration;

      foreach (VCConfiguration config in configs)
      {
        if (config.ConfigurationName == active.ConfigurationName &&
            config.Platform.Name == active.PlatformName)
        {
          return config;
        }
      }

      return null;
    }

    /// <summary>
    /// Extends the string class to allow checking if a string contains another string
    /// allowing a comparison type (such as case-insensitivity).
    /// </summary>
    /// <param name="source">Base string to search.</param>
    /// <param name="toCheck">String to check if contained within base string.</param>
    /// <param name="comparison">Comparison type.</param>
    /// <returns>True if toCheck is contained in source.</returns>
    public static bool Contains(this string source, string toCheck, StringComparison comparison)
    {
      return source.IndexOf(toCheck, comparison) != -1;
    }

    /// <summary>
    /// This checks if the first argument is a descendant of the second, where
    /// both arguments are process IDs of two processes.
    /// </summary>
    /// <param name="processSearcher">Process searcher object.</param>
    /// <param name="descendant">Process ID of the descendant.</param>
    /// <param name="ancestor">Process ID of ancestor.</param>
    /// <returns>True if descendant is a descendant of ancestor.</returns>
    public static bool IsDescendantOfProcess(
      ProcessSearcher processSearcher,
      uint descendant,
      uint ancestor)
    {
      return IsDescendantOfProcessHelper(
          processSearcher,
          descendant,
          ancestor,
          DateTime.UtcNow);
    }

    /// <summary>
    /// Helper function for IsDescendantOfProcessHelper().
    /// This function prevents an edge case where a process has a parent process ID
    /// that refers to a descendant of itself. This can occur when the parent of a process
    /// is destroyed and the parent's pid is recycled and reused on a descendant.  The
    /// parent process ID value is never updated when the parent is destroyed. The solution
    /// is to make sure that parents are created before children, otherwise it is a cycle.
    /// </summary>
    /// <param name="processSearcher">Process searcher object.</param>
    /// <param name="descendant">Process ID of the descendant.</param>
    /// <param name="anscestor">Process ID of the ancestor.</param>
    /// <param name="previousCreationTime">Creation time of the previous call's descendant.</param>
    /// <returns>True if descendant is a descendant of ancestor.</returns>
    private static bool IsDescendantOfProcessHelper(
        ProcessSearcher processSearcher,
        uint descendant,
        uint anscestor,
        DateTime previousCreationTime)
    {
      List<ProcessInfo> results = processSearcher.GetResultsByID(descendant);
      foreach (ProcessInfo proc in results)
      {
        // Ensure this parent relationship is valid.
        if (proc.CreationDate <= previousCreationTime)
        {
          if (descendant == anscestor)
          {
            return true;
          }
          else if (descendant == proc.ParentID)
          {
            // If process is its own parent then we have a cycle, return false.
            return false;
          }

          return IsDescendantOfProcessHelper(
              processSearcher,
              proc.ParentID,
              anscestor,
              proc.CreationDate);
        }
      }

      return false;
    }
  }
}
