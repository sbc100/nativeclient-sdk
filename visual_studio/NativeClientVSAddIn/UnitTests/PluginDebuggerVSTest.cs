// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;
  using System.Reflection;

  using EnvDTE80;
  using Microsoft.VisualStudio.TestTools.UnitTesting;

  using NativeClientVSAddIn;

  /// <summary>
  /// This is a test class for PluginDebuggerVSTest and is intended
  /// to contain all PluginDebuggerVS Unit Tests
  /// </summary>
  [TestClass]
  public class PluginDebuggerVSTest
  {
    /// <summary>
    /// The main visual studio object.
    /// </summary>
    private DTE2 dte_;

    /// <summary>
    /// Holds the default properties from property pages to use during tests.
    /// </summary>
    private MockPropertyManager properties_;

    /// <summary>
    /// Gets or sets the test context which provides information about,
    /// and functionality for the current test run.
    /// </summary>
    public TestContext TestContext { get; set; }

    /// <summary>
    /// This is run before each test to create test resources.
    /// </summary>
    [TestInitialize]
    public void TestSetup()
    {
      dte_ = TestUtilities.StartVisualStudioInstance();
      try
      {
        TestUtilities.AssertAddinLoaded(dte_, NativeClientVSAddIn.Strings.AddInName);
      }
      catch
      {
        TestUtilities.CleanUpVisualStudioInstance(dte_);
        throw;
      }

      // Set up mock property manager to return the desired property values.
      properties_ = new MockPropertyManager(
        PropertyManager.ProjectPlatformType.Pepper,
        delegate(string page, string name)
        {
          switch (page)
          {
            case "ConfigurationGeneral":
              switch (name)
              {
                case "VSNaClSDKRoot": return System.Environment.GetEnvironmentVariable(
                    NativeClientVSAddIn.Strings.SDKPathEnvironmentVariable);
              }

              break;
            case "Property":
              switch (name)
              {
                case "ProjectDirectory": return TestContext.DeploymentDirectory;
                case "PluginAssembly": return @"fake\Assembly\String";
              }

              break;
          }

          return null;
        },
        null);
    }

    /// <summary>
    /// This is run after each test to clean up things created in TestSetup().
    /// </summary>
    [TestCleanup]
    public void TestCleanup()
    {
      TestUtilities.CleanUpVisualStudioInstance(dte_);
    }

    /// <summary>
    /// A test for the constructor.
    /// </summary>
    [TestMethod]
    public void PluginDebuggerVSConstructorTest()
    {
      // Check that a null dte fails.
      try
      {
        PluginDebuggerBase nullDte = new PluginDebuggerVS(null, properties_);
        Assert.Fail("Using null DTE instance did not throw exception");
      }
      catch (ArgumentNullException)
      {
        // This is expected for a correct implementation.
      }
      catch
      {
        Assert.Fail("Using null DTE instance threw something other than ArgumentNullException");
      }

      // Check that a null PropertyManager fails.
      try
      {
        PluginDebuggerBase nullDte = new PluginDebuggerVS(dte_, null);
        Assert.Fail("Using null property manager did not throw exception");
      }
      catch (ArgumentNullException)
      {
        // This is expected for a correct implementation.
      }
      catch
      {
        Assert.Fail("Using null property manager threw something other than ArgumentNullException");
      }
    }

    /// <summary>
    /// A test for FindAndAttachToPlugin.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void FindAndAttachToPepperPluginTest()
    {
      MockProcessSearcher processResults = new MockProcessSearcher();

      using (PluginDebuggerVS target = new PluginDebuggerVS(dte_, properties_))
      {
        PluginDebuggerBase_Accessor targetBase = new PluginDebuggerBase_Accessor(
            new PrivateObject(target, new PrivateType(typeof(PluginDebuggerBase))));
        targetBase.debuggedChromeMainProcess_ = System.Diagnostics.Process.GetCurrentProcess();
        uint currentProcId = (uint)targetBase.debuggedChromeMainProcess_.Id;

        string pluginLoadFlag = string.Format(
            Strings.PepperProcessPluginFlagFormat, properties_.PluginAssembly);
        string pepperCommandLine = string.Concat(
            pluginLoadFlag, " ", Strings.ChromeRendererFlag);

        // Fake the list of processes on the system.
        processResults.ProcessList.Add(
            new ProcessInfo(
                currentProcId,
                currentProcId,
                string.Empty,
                Strings.NaClDebugFlag,
                Strings.ChromeProcessName));
        processResults.ProcessList.Add(
            new ProcessInfo(1, currentProcId, string.Empty, string.Empty, "MyParentProcess"));
        processResults.ProcessList.Add(
            new ProcessInfo(10, 1, string.Empty, pepperCommandLine, Strings.ChromeProcessName));

        // This is missing some relevant command line args, and should not be attached to.
        processResults.ProcessList.Add(
            new ProcessInfo(12, 1, string.Empty, pluginLoadFlag, Strings.ChromeProcessName));

        // This doesn't have this process as its parent, and should not be attached to.
        processResults.ProcessList.Add(
            new ProcessInfo(14, 14, string.Empty, pepperCommandLine, Strings.ChromeProcessName));

        // Set the private value to the mock object (can't use accessor since no valid cast).
        FieldInfo processSearcherField = typeof(PluginDebuggerBase).GetField(
            "processSearcher_",
            BindingFlags.NonPublic | BindingFlags.Instance);
        processSearcherField.SetValue(targetBase.Target, processResults);

        // Test that the correct processes are attached to.
        bool goodPepper = false;
        var handler = new EventHandler<NativeClientVSAddIn.PluginDebuggerBase.PluginFoundEventArgs>(
          delegate(object unused, NativeClientVSAddIn.PluginDebuggerBase.PluginFoundEventArgs args)
          {
            switch (args.ProcessID)
            {
              case 10:
                if (goodPepper)
                {
                  Assert.Fail("Should not attach twice to same pepper process");
                }

                goodPepper = true;
                break;
              case 12:
                Assert.Fail("Should not attach to pepper process with bad args");
                break;
              case 14:
                Assert.Fail("Should not attach to pepper process that is not "
                          + "descendant of Visual Studio");
                break;
              default:
                Assert.Fail("Should not attach to non-pepper/non-nacl process");
                break;
            }
          });

        target.PluginFoundEvent += handler;
        target.FindAndAttachToPlugin(null, null);
        target.PluginFoundEvent -= handler;

        Assert.IsTrue(goodPepper, "Failed to attach to pepper process");
      }
    }

    /// <summary>
    /// Checks that VS properly attaches debugger.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void AttachVSDebuggerTest()
    {
      using (System.Diagnostics.Process dummyProc = TestUtilities.StartProcessForKilling(
          "DummyProc", 20))
      {
        try
        {
          PluginDebuggerVS_Accessor target = new PluginDebuggerVS_Accessor(dte_, properties_);

          var pluginFoundArgs = new NativeClientVSAddIn.PluginDebuggerBase.PluginFoundEventArgs(
              (uint)dummyProc.Id);
          target.Attach(null, pluginFoundArgs);

          bool isBeingDebugged = false;
          foreach (EnvDTE.Process proc in dte_.Debugger.DebuggedProcesses)
          {
            if (proc.ProcessID == dummyProc.Id)
            {
              isBeingDebugged = true;
            }
          }

          Assert.IsTrue(isBeingDebugged, "Visual Studio debugger did not attach");
        }
        finally
        {
          if (dummyProc != null && !dummyProc.HasExited)
          {
            dummyProc.Kill();
            dummyProc.Dispose();
          }
        }
      }
    }

    /// <summary>
    /// A test for IsPluginProcess.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void IsPepperPluginProcessTest()
    {
      PluginDebuggerVS_Accessor target = new PluginDebuggerVS_Accessor(dte_, properties_);

      string identifierFlagTarget =
          string.Format(Strings.PepperProcessPluginFlagFormat, target.pluginAssembly_);
      string goodFlags = string.Concat(Strings.ChromeRendererFlag, ' ', identifierFlagTarget);

      ProcessInfo badProc1 = new ProcessInfo(
          1, 1, string.Empty, goodFlags, Strings.NaClProcessName);
      ProcessInfo badProc2 = new ProcessInfo(
          1, 1, string.Empty, Strings.NaClLoaderFlag, Strings.ChromeProcessName);
      ProcessInfo badProc3 = new ProcessInfo(
          1, 1, string.Empty, Strings.ChromeRendererFlag, Strings.ChromeProcessName);
      ProcessInfo badProc4 = new ProcessInfo(
          1, 1, string.Empty, identifierFlagTarget, Strings.ChromeProcessName);
      ProcessInfo goodProc = new ProcessInfo(
          1, 1, string.Empty, goodFlags, Strings.ChromeProcessName);

      Assert.IsTrue(target.IsPluginProcess(goodProc, string.Empty));
      Assert.IsFalse(target.IsPluginProcess(badProc1, string.Empty));
      Assert.IsFalse(target.IsPluginProcess(badProc2, string.Empty));
      Assert.IsFalse(target.IsPluginProcess(badProc3, string.Empty));
      Assert.IsFalse(target.IsPluginProcess(badProc4, string.Empty));
    }
  }
}
