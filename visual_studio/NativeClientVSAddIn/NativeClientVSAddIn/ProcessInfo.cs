// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NativeClientVSAddIn
{
  using System;
  using System.Globalization;
  using System.Management;

  /// <summary>
  /// Holds information about a process for a ProcessSearcher.
  /// </summary>
  public class ProcessInfo
  {
    /// <summary>
    /// Constructs a process entry.
    /// </summary>
    /// <param name="id">Process ID.</param>
    /// <param name="parentId">Process ID of the parent process.</param>
    /// <param name="creationDate">
    /// String date in format 'yyyyMMddHHmmss.ffffff', or if empty then current time used.
    /// </param>
    /// <param name="commandLine">Command line arguments to the process.</param>
    /// <param name="name">Process name.</param>
    public ProcessInfo(uint id, uint parentId, string creationDate, string commandLine, string name)
    {
      if (string.IsNullOrEmpty(creationDate))
      {
        // If creationDate string is empty, then use the current timestamp.
        CreationDate = DateTime.UtcNow;
      }
      else
      {
        // Example creationDate: "20120622150149.843021-420".
        CreationDate = DateTime.ParseExact(
            creationDate.Substring(0, 21),
            "yyyyMMddHHmmss.ffffff",
            CultureInfo.InvariantCulture);
        long timeZoneMinutes = long.Parse(creationDate.Substring(21));
        CreationDate = CreationDate.AddMinutes(-timeZoneMinutes);
      }

      ID = id;
      ParentID = parentId;
      CommandLine = commandLine;
      Name = name;
    }

    /// <summary>
    /// Gets or sets Process ID of the represented process.
    /// </summary>
    public uint ID { get; set; }

    /// <summary>
    /// Gets or sets Process ID of the parent process.
    /// </summary>
    public uint ParentID { get; set; }

    /// <summary>
    /// Gets or sets DateTime of the process creation.
    /// </summary>
    public DateTime CreationDate { get; set; }

    /// <summary>
    /// Gets or sets Command line arguments to the process.
    /// </summary>
    public string CommandLine { get; set; }

    /// <summary>
    /// Gets or sets Name of the process.
    /// </summary>
    public string Name { get; set; }

    /// <summary>
    /// Casts from a management object that is a Win32_Process underlying type to a ProcessInfo.
    /// </summary>
    /// <param name="from">A management object that is Win32_Process underneath.</param>
    /// <returns>A ProcessInfo object.</returns>
    public static explicit operator ProcessInfo(ManagementObject from)
    {
      return new ProcessInfo(
          id: (uint)from["ProcessID"],
          parentId: (uint)from["ParentProcessID"],
          creationDate: from["CreationDate"] as string,
          commandLine: from["CommandLine"] as string,
          name: from["Name"] as string);
    }
  }
}
