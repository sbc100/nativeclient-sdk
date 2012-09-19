// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;
  using System.Collections.Generic;
  using System.Linq;
  using System.Management;

  /// <summary>
  /// Queries the system for the list of running processes.
  /// </summary>
  public class ProcessSearcher
  {
    /// <summary>
    /// Returns results of a process search subject to given constraints.
    /// </summary>
    /// <param name="constraints">
    /// A function taking a ProcessInfo object and returning true if the
    /// ProcessInfo object satisfies the constraints.
    /// </param>
    /// <returns>List of matching processes.</returns>
    public List<ProcessInfo> GetResults(Func<ProcessInfo, bool> constraints)
    {
      return GetSystemProcesses().Where(constraints).ToList();
    }

    /// <summary>
    /// Searches the system for all processes of a given name.
    /// </summary>
    /// <param name="name">Name to search for.</param>
    /// <returns>List of matching processes.</returns>
    public List<ProcessInfo> GetResultsByName(string name)
    {
      return GetResults(p => name.Equals(p.Name, StringComparison.OrdinalIgnoreCase));
    }

    /// <summary>
    /// Searches the system for all processes of a given process ID.
    /// </summary>
    /// <param name="procID">ID to search for.</param>
    /// <returns>List of matching processes.</returns>
    public List<ProcessInfo> GetResultsByID(uint procID)
    {
      return GetResults(p => procID == p.ID);
    }

    /// <summary>
    /// Returns a list of all descendant processes of a process.
    /// </summary>
    /// <param name="root">Process ID of the process to get all descendants from.</param>
    /// <returns>All descendants of the given root process.</returns>
    public List<ProcessInfo> GetDescendants(uint root)
    {
      List<ProcessInfo> processes = GetSystemProcesses();
      HashSet<ProcessInfo> descendants = new HashSet<ProcessInfo>();
      descendants.UnionWith(processes.Where(p => p.ID == root));
      int lastCount = 0;
      while (descendants.Count > lastCount)
      {
        lastCount = descendants.Count;

        // Add any processes who have a parent that is in the descendant list already.
        // Note we check the creation date to prevent cycles caused by recycled process IDs.
        descendants.UnionWith(processes.Where(p => descendants.Any(
            parent => parent.ID == p.ParentID && parent.CreationDate <= p.CreationDate)));
      }

      return descendants.ToList();
    }

    /// <summary>
    /// Queries the system for the full list of processes.
    /// </summary>
    /// <returns>List of processes on the system.</returns>
    protected virtual List<ProcessInfo> GetSystemProcesses()
    {
      var processList = new List<ProcessInfo>();
      string query = "select * from Win32_Process";
      using (ManagementObjectSearcher searcher = new ManagementObjectSearcher(query))
      {
        using (ManagementObjectCollection results = searcher.Get())
        {
          foreach (ManagementObject process in results)
          {
            processList.Add((ProcessInfo)process);
          }
        }
      }

      return processList;
    }
  }
}
