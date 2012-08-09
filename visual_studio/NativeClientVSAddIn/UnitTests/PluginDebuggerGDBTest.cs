// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;
  using System.Collections.Generic;
  using System.IO;
  using System.Reflection;

  using EnvDTE80;
  using Microsoft.VisualStudio.TestTools.UnitTesting;

  using NativeClientVSAddIn;
    
  /// <summary>
  /// This is a test class for PluginDebuggerGDBTest and is intended
  /// to contain all PluginDebuggerGDB Unit Tests
  /// </summary>
  [TestClass]
  public class PluginDebuggerGDBTest
  {
    /// <summary>
    /// This holds the path to the NaCl solution used in these tests.
    /// The NaCl solution is a valid nacl/pepper plug-in VS solution.
    /// It is copied into the testing deployment directory and opened in some tests.
    /// Because unit-tests run in any order, the solution should not be written to
    /// in any tests.
    /// </summary>
    private static string naclSolution;

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
    /// This is run one time before any test methods are called. Here we set-up a test-copy of a
    /// new NaCl solution for use in the tests.
    /// </summary>
    /// <param name="testContext">Holds information about the current test run</param>
    [ClassInitialize]
    public static void ClassSetup(TestContext testContext)
    {
      DTE2 dte = TestUtilities.StartVisualStudioInstance();
      try
      {
        naclSolution = TestUtilities.CreateBlankValidNaClSolution(
          dte,
          "PluginDebuggerGDBTest",
          NativeClientVSAddIn.Strings.PepperPlatformName,
          NativeClientVSAddIn.Strings.NaClPlatformName,
          testContext);
      }
      finally
      {
        TestUtilities.CleanUpVisualStudioInstance(dte);
      }
    }

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
        PropertyManager.ProjectPlatformType.NaCl,
        delegate(string page, string name)
        {
          switch (page)
          {
            case "ConfigurationGeneral":
              switch (name)
              {
                case "VSNaClSDKRoot": return System.Environment.GetEnvironmentVariable(
                    NativeClientVSAddIn.Strings.SDKPathEnvironmentVariable);
                case "NaClIrtPath": return @"fake\Irt\Path";
                case "NaClManifestPath": return string.Empty;
                case "ToolchainName": return "newlib";
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
    public void PluginDebuggerGDBConstructorTest()
    {
      // Check that a null dte fails.
      try
      {
        PluginDebuggerBase nullDte = new PluginDebuggerGDB(null, properties_);
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
        PluginDebuggerBase nullDte = new PluginDebuggerGDB(dte_, null);
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
    /// Tests that a plugin can be found.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void FindAndAttachToNaClPluginTest()
    {
      MockProcessSearcher processResults = new MockProcessSearcher();

      using (PluginDebuggerGDB target = new PluginDebuggerGDB(dte_, properties_))
      {
        PluginDebuggerBase_Accessor targetBase = new PluginDebuggerBase_Accessor(
            new PrivateObject(target, new PrivateType(typeof(PluginDebuggerBase))));
        targetBase.debuggedChromeMainProcess_ = System.Diagnostics.Process.GetCurrentProcess();

        uint currentProcId = (uint)targetBase.debuggedChromeMainProcess_.Id;

        // Target nacl process flag.
        string naclCommandLine = Strings.NaClLoaderFlag;

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
            new ProcessInfo(11, 1, string.Empty, naclCommandLine, Strings.NaClProcessName));

        // This is missing some relevant command line args, should not be attached to.
        processResults.ProcessList.Add(
            new ProcessInfo(13, 1, string.Empty, string.Empty, Strings.NaClProcessName));

        // This doesn't have chrome process as their parent, so they should not be attached to.
        processResults.ProcessList.Add(
            new ProcessInfo(15, 15, string.Empty, naclCommandLine, Strings.NaClProcessName));

        // Set the private value to the mock object (can't use accessor since no valid cast).
        FieldInfo processSearcherField = typeof(PluginDebuggerBase).GetField(
            "processSearcher_",
            BindingFlags.NonPublic | BindingFlags.Instance);
        processSearcherField.SetValue(targetBase.Target, processResults);

        // Test that the correct processes are attached to.
        bool goodNaCl = false;
        var handler = new EventHandler<NativeClientVSAddIn.PluginDebuggerBase.PluginFoundEventArgs>(
          delegate(object unused, NativeClientVSAddIn.PluginDebuggerBase.PluginFoundEventArgs args)
          {
            switch (args.ProcessID)
            {
              case 11:
                if (goodNaCl)
                {
                  Assert.Fail("Should not attach twice to same nacl process");
                }

                goodNaCl = true;
                break;
              case 13:
                Assert.Fail("Should not attach to nacl process with bad args");
                break;
              case 15:
                Assert.Fail("Should not attach to nacl process that is not "
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

        Assert.IsTrue(goodNaCl, "Failed to attach to NaCl process");
      }
    }

    /// <summary>
    /// A test for Attach. Implicitly tests CleanUpGDBProcess().
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void AttachNaClGDBTest()
    {
      PluginDebuggerGDB_Accessor target = new PluginDebuggerGDB_Accessor(dte_, properties_);

      string existingGDB = "AttachNaClGDBTest_existingGDB";
      try
      {
        target.gdbProcess_ = TestUtilities.StartProcessForKilling(existingGDB, 20);
        string existingInitFileName = Path.GetTempFileName();
        target.gdbInitFileName_ = existingInitFileName;

        // Visual studio won't allow adding a breakpoint unless it is associated with
        // an existing file and valid line number, so use BlankValidSolution.
        dte_.Solution.Open(naclSolution);
        string fileName = "main.cpp";
        string functionName = "NaClProjectInstance::HandleMessage";
        int lineNumber = 39;
        dte_.Debugger.Breakpoints.Add(Function: functionName);
        dte_.Debugger.Breakpoints.Add(Line: lineNumber, File: fileName);

        target.Attach(null, new PluginDebuggerBase.PluginFoundEventArgs(0));

        Assert.IsTrue(File.Exists(target.gdbInitFileName_), "Init file not written");

        var gdbCommands = new List<string>(File.ReadAllLines(target.gdbInitFileName_));

        // Validate that the commands contain what we specified.
        // The syntax itself is not validated since this add-in is not responsible for
        // the syntax and it could change.
        Assert.IsTrue(
            gdbCommands.Exists(s => s.Contains(fileName) && s.Contains(lineNumber.ToString())),
            "Line breakpoint not properly set");
        Assert.IsTrue(
            gdbCommands.Exists(s => s.Contains(functionName)),
            "Function breakpoint not properly set");
        
        // Note fake assembly string should be double escaped when passed to gdb.
        Assert.IsTrue(
          gdbCommands.Exists(s => s.Contains(functionName)),
          @"fake\\Assembly\\String");

        // Check that the pre-existing gdb process was killed and its init file cleaned up.
        Assert.IsFalse(
            TestUtilities.DoesProcessExist("python.exe", existingGDB),
            "Failed to kill existing GDB process");
        Assert.IsFalse(
            File.Exists(existingInitFileName),
            "Failed to delete existing temp gdb init file");
      }
      finally
      {
        if (dte_.Debugger.Breakpoints != null)
        {
          // Remove all breakpoints.
          foreach (EnvDTE.Breakpoint bp in dte_.Debugger.Breakpoints)
          {
            bp.Delete();
          }
        }

        // Clean up file if not erased.
        if (!string.IsNullOrEmpty(target.gdbInitFileName_) && File.Exists(target.gdbInitFileName_))
        {
          File.Delete(target.gdbInitFileName_);
        }

        // Kill the gdb process if not killed.
        if (target.gdbProcess_ != null && !target.gdbProcess_.HasExited)
        {
          target.gdbProcess_.Kill();
          target.gdbProcess_.Dispose();
        }
      }
    }

    /// <summary>
    /// A test for Dispose. Implicitly tests CleanUpGDBProcess().
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void DisposeTest()
    {
      PluginDebuggerGDB_Accessor target = new PluginDebuggerGDB_Accessor(dte_, properties_);

      string existingGDB = "DisposeTest_GDB";
      try
      {
        target.gdbProcess_ = TestUtilities.StartProcessForKilling(existingGDB, 20);
        string existingInitFileName = Path.GetTempFileName();
        target.gdbInitFileName_ = existingInitFileName;

        target.Dispose();
        
        // Check that the pre-existing gdb process was killed and its init file cleaned up.
        Assert.IsFalse(
            TestUtilities.DoesProcessExist("python.exe", existingGDB),
            "Failed to kill existing GDB process");
        Assert.IsFalse(
            File.Exists(existingInitFileName),
            "Failed to delete existing temp gdb init file");
      }
      finally
      {
        // Clean up file if not erased.
        if (!string.IsNullOrEmpty(target.gdbInitFileName_) && File.Exists(target.gdbInitFileName_))
        {
          File.Delete(target.gdbInitFileName_);
        }

        // Kill the gdb process if not killed.
        if (target.gdbProcess_ != null && !target.gdbProcess_.HasExited)
        {
          target.gdbProcess_.Kill();
          target.gdbProcess_.Dispose();
        }
      }
    }

    /// <summary>
    /// A test for IsPluginProcess.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void IsNaClPluginProcessTest()
    {
      PluginDebuggerGDB_Accessor target = new PluginDebuggerGDB_Accessor(dte_, properties_);

      ProcessInfo badProc = new ProcessInfo(
          1, 1, string.Empty, Strings.ChromeRendererFlag, Strings.NaClProcessName);
      ProcessInfo goodProc = new ProcessInfo(
          1, 1, string.Empty, Strings.NaClLoaderFlag, Strings.NaClProcessName);
      
      string goodMainChromeFlags = Strings.NaClDebugFlag;
      string badMainChromeFlags = string.Format(
          Strings.PepperProcessPluginFlagFormat, target.pluginAssembly_);

      Assert.IsTrue(target.IsPluginProcess(goodProc, goodMainChromeFlags));
      Assert.IsFalse(target.IsPluginProcess(goodProc, badMainChromeFlags));
      Assert.IsFalse(target.IsPluginProcess(badProc, goodMainChromeFlags));
    }
  }
}
