/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VsSDK.IntegrationTestLibrary;
using Microsoft.VSSDK.Tools.VsIdeTesting;

namespace IntegrationTests {
  [TestClass]
  public class SolutionTests {
    #region fields

    private delegate void ThreadInvoker();

    #endregion

    #region properties

    /// <summary>
    ///Gets or sets the test context which provides
    ///information about and functionality for the current test run.
    ///</summary>
    public TestContext TestContext { get; set; }

    #endregion

    #region ctors

    #endregion

    [TestMethod]
    [HostType("VS IDE")]
    public void CreateEmptySolution() {
      UIThreadInvoker.Invoke((ThreadInvoker) delegate {
                                               var testUtils = new TestUtils();
                                               testUtils.CloseCurrentSolution(
                                                   __VSSLNSAVEOPTIONS.
                                                       SLNSAVEOPT_NoSave);
                                               testUtils.CreateEmptySolution(
                                                   TestContext.TestDir,
                                                   "CreateEmptySolution");
                                             });
    }
  }
}