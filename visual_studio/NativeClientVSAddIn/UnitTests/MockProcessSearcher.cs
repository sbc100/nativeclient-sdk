// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;
  using System.Collections.Generic;
  using System.Management;

  /// <summary>
  /// A fake process searcher that allows the list of 'processes' on the system to be faked.
  /// </summary>
  public class MockProcessSearcher : NativeClientVSAddIn.ProcessSearcher
  {
    /// <summary>
    /// Constructs the fake process searcher.
    /// </summary>
    public MockProcessSearcher()
    {
      this.ProcessList = new List<NativeClientVSAddIn.ProcessInfo>();
    }

    /// <summary>
    /// Gets or sets the fake list of processes this MockProcessSearcher knows about.
    /// </summary>
    public List<NativeClientVSAddIn.ProcessInfo> ProcessList { get; set; }

    /// <summary>
    /// This method substitutes the fake process list for the list of real system processes.
    /// </summary>
    /// <returns>Fake list of processes</returns>
    protected override List<NativeClientVSAddIn.ProcessInfo> GetSystemProcesses()
    {
      return this.ProcessList;
    }
  }
}
