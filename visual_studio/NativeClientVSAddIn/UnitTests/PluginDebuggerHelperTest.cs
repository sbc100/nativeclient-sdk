// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;
  using System.Diagnostics;
  using System.IO;
  using System.Reflection;
  using System.Threading;

  using EnvDTE80;
  using Microsoft.VisualStudio.TestTools.UnitTesting;

  using NativeClientVSAddIn;

  /// <summary>
  /// This is a test class for PluginDebuggerHelperTest and is intended
  /// to contain all PluginDebuggerHelperTest Unit Tests.
  /// </summary>
  [TestClass]
  public class PluginDebuggerHelperTest
  {
    /// <summary>
    /// The dummy loop solution is a valid nacl/pepper plug-in VS solution.
    /// It is copied into the testing deployment directory and opened in some tests.
    /// Because unit-tests run in any order, the solution should not be written to
    /// in any tests.
    /// </summary>
    private const string DummyLoopSolution = @"\DummyLoop\DummyLoop.sln";

    /// <summary>
    /// The main visual studio object.
    /// </summary>
    private DTE2 dte_ = null;

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
    /// A test for PluginDebuggerHelper Constructor.
    /// </summary>
    [TestMethod]
    public void PluginDebuggerHelperConstructorTest()
    {
      // Check that a null dte fails.
      try
      {
        PluginDebuggerHelper nullDte = new PluginDebuggerHelper(null);
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

      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
      Assert.AreEqual(dte_, target.dte_);
      Assert.IsNull(target.webServerOutputPane_);
      Assert.IsFalse(target.isProperlyInitialized_);
    }

    /// <summary>
    /// This unit test verifies that the gdb init file is written correctly,
    /// and old-existing GDB processes are cleaned up.
    /// Verification of actual attachment is the responsibility of integration
    /// tests and NaCl-GDB itself.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void AttachNaClGDBTest()
    {
      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
      string existingGDB = "AttachNaClGDBTest_existingGDB";
      try
      {
        target.pluginProjectDirectory_ = TestContext.DeploymentDirectory;
        target.pluginAssembly_ = "fakeAssemblyString";
        target.irtPath_ = "fakeIrtPath";
        target.gdbPath_ = "python.exe";
        target.gdbProcess_ = TestUtilities.StartProcessForKilling(existingGDB, 20);
        string existingInitFileName = Path.GetTempFileName();
        target.gdbInitFileName_ = existingInitFileName;
        target.isProperlyInitialized_ = true;

        // Visual studio won't allow adding a breakpoint unless it is associated with
        // an existing file and valid line number, so use DummyLoopSolution.
        dte_.Solution.Open(TestContext.DeploymentDirectory + DummyLoopSolution);
        string fileName = "main.cpp";
        string functionName = "DummyInstance::HandleMessage";
        int lineNumber = 35;
        dte_.Debugger.Breakpoints.Add(Function: functionName);
        dte_.Debugger.Breakpoints.Add(Line: lineNumber, File: fileName);

        target.AttachNaClGDB(null, new PluginDebuggerHelper.PluginFoundEventArgs(0));

        Assert.IsTrue(File.Exists(target.gdbInitFileName_), "Init file not written");

        string[] gdbCommands = File.ReadAllLines(target.gdbInitFileName_);
        bool functionBP = false;
        bool lineBP = false;

        // Validate that the commands contain what we specified.
        // The syntax itself is not validated since this add-in is not responsible for
        // the syntax and it could change.
        foreach (string command in gdbCommands)
        {
          if (command.Contains(fileName) && command.Contains(lineNumber.ToString()))
          {
            lineBP = true;
          }

          if (command.Contains(functionName))
          {
            functionBP = true;
          }
        }

        Assert.IsFalse(
            TestUtilities.DoesProcessExist("python.exe", existingGDB),
            "Failed to kill existing GDB process");
        Assert.IsFalse(
            File.Exists(existingInitFileName),
            "Failed to delete existing temp gdb init file");
        Assert.IsTrue(lineBP, "Line breakpoint not properly set");
        Assert.IsTrue(functionBP, "Function breakpoint not properly set");
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

        if (!string.IsNullOrEmpty(target.gdbInitFileName_) && File.Exists(target.gdbInitFileName_))
        {
          File.Delete(target.gdbInitFileName_);
        }

        if (target.gdbProcess_ != null && !target.gdbProcess_.HasExited)
        {
          target.gdbProcess_.Kill();
          target.gdbProcess_.Dispose();
        }
      }
    }

    /// <summary>
    /// A test for FindAndAttachToPlugin.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void FindAndAttachToPluginTest()
    {
      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
      target.isProperlyInitialized_ = true;

      MockProcessSearcher processResults = new MockProcessSearcher();
      uint currentProcId = (uint)System.Diagnostics.Process.GetCurrentProcess().Id;
      string naclCommandLine = Strings.NaClProcessTypeFlag + " " + Strings.NaClDebugFlag;
      target.pluginAssembly_ = "testAssemblyPath";
      string pluginLoadFlag = string.Format(
          Strings.PepperProcessPluginFlagFormat, target.pluginAssembly_);
      string pepperCommandLine = string.Concat(
          pluginLoadFlag, " ", Strings.PepperProcessTypeFlag);
      string pluginFlagCommandLine =
          string.Format(Strings.PepperProcessPluginFlagFormat, target.pluginAssembly_);

      // Fake the list of processes on the system.
      processResults.ProcessList.Add(
          new ProcessInfo(currentProcId, currentProcId, string.Empty, string.Empty, "devenv.exe"));
      processResults.ProcessList.Add(
          new ProcessInfo(1, currentProcId, string.Empty, string.Empty, "MyParentProcess"));
      processResults.ProcessList.Add(
          new ProcessInfo(10, 1, string.Empty, pepperCommandLine, Strings.PepperProcessName));
      processResults.ProcessList.Add(
          new ProcessInfo(11, 1, string.Empty, naclCommandLine, Strings.NaClProcessName));
      processResults.ProcessList.Add(
          new ProcessInfo(12, 1, string.Empty, pluginFlagCommandLine, Strings.PepperProcessName));
      processResults.ProcessList.Add(
          new ProcessInfo(13, 1, string.Empty, Strings.NaClDebugFlag, Strings.NaClProcessName));

      // These two don't have this process as their parent, so they should not be attached to.
      processResults.ProcessList.Add(
          new ProcessInfo(14, 14, string.Empty, pepperCommandLine, Strings.PepperProcessName));
      processResults.ProcessList.Add(
          new ProcessInfo(15, 15, string.Empty, naclCommandLine, Strings.NaClProcessName));

      // Set the private value to the mock object (can't use accessor since no valid cast).
      typeof(PluginDebuggerHelper).GetField(
          "processSearcher_",
          BindingFlags.NonPublic | BindingFlags.Instance).SetValue(target.Target, processResults);

      // Test that the correct processes are attached to.
      bool goodNaCl = false;
      bool goodPepper = false;
      var handler = new EventHandler<NativeClientVSAddIn.PluginDebuggerHelper.PluginFoundEventArgs>(
        delegate(object unused, NativeClientVSAddIn.PluginDebuggerHelper.PluginFoundEventArgs args)
        {
          switch (args.ProcessID)
          {
            case 10:
              if (goodPepper)
              {
                Assert.Fail("Should not attach twice to same pepper process");
              }

              if (target.projectPlatformType_ ==
                  PluginDebuggerHelper_Accessor.ProjectPlatformType.NaCl)
              {
                Assert.Fail("Attached to pepper process when NaCl was the target");
              }

              goodPepper = true;
              break;
            case 11:
              if (goodNaCl)
              {
                Assert.Fail("Should not attach twice to same nacl process");
              }

              if (target.projectPlatformType_ ==
                  PluginDebuggerHelper_Accessor.ProjectPlatformType.Pepper)
              {
                Assert.Fail("Attached to nacl process when pepper was the target");
              }

              goodNaCl = true;
              break;
            case 12:
              Assert.Fail("Should not attach to pepper process with bad args");
              break;
            case 13:
              Assert.Fail("Should not attach to nacl process with bad args");
              break;
            case 14:
              Assert.Fail("Should not attach to pepper process that is not "
                        + "descendant of Visual Studio");
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

      target.add_PluginFoundEvent(handler);
      target.projectPlatformType_ = PluginDebuggerHelper_Accessor.ProjectPlatformType.Pepper;
      target.FindAndAttachToPlugin(null, null);
      target.projectPlatformType_ = PluginDebuggerHelper_Accessor.ProjectPlatformType.NaCl;
      target.FindAndAttachToPlugin(null, null);
      target.remove_PluginFoundEvent(handler);
      Assert.IsTrue(goodPepper, "Failed to attach to pepper process");
      Assert.IsTrue(goodNaCl, "Failed to attach to NaCl process");
    }

    /// <summary>
    /// A test for LoadProjectSettings.
    /// </summary>
    [TestMethod]
    public void LoadProjectSettingsTest()
    {
      string expectedSDKRootDir =
          Environment.GetEnvironmentVariable(Strings.SDKPathEnvironmentVariable);
      Assert.IsNotNull(expectedSDKRootDir, "SDK Path environment variable not set!");

      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
      target.isProperlyInitialized_ = false;
      try
      {
        target.LoadProjectSettings();
        Assert.Fail("Initializing with no loaded solution shouldn't succeed");
      }
      catch (ArgumentOutOfRangeException)
      {
        // This is expected for a correct implementation.
      }

      dte_.Solution.Open(TestContext.DeploymentDirectory + DummyLoopSolution);

      // Setting the start-up project to a non-cpp project should make loading fail.
      string badProjectUniqueName = @"NotNaCl\NotNaCl.csproj";
      object[] badStartupProj = { badProjectUniqueName };
      dte_.Solution.SolutionBuild.StartupProjects = badStartupProj;
      Assert.IsFalse(target.LoadProjectSettings());
      Assert.IsFalse(target.isProperlyInitialized_);

      // Setting the start-up project to correct C++ project, but also setting the platform
      // to non-nacl/pepper should make loading fail.
      string projectUniqueName = @"DummyLoop\DummyLoop.vcxproj";
      object[] startupProj = { projectUniqueName };
      dte_.Solution.SolutionBuild.StartupProjects = startupProj;
      TestUtilities.SetSolutionConfiguration(dte_, projectUniqueName, "Debug", "Win32");
      Assert.IsFalse(target.LoadProjectSettings());
      Assert.IsFalse(target.isProperlyInitialized_);

      // Setting the platform to NaCl should make loading succeed.
      TestUtilities.SetSolutionConfiguration(
          dte_, projectUniqueName, "Debug", Strings.NaClPlatformName);
      Assert.IsTrue(target.LoadProjectSettings());
      Assert.IsTrue(target.isProperlyInitialized_);
      Assert.AreEqual(
          target.projectPlatformType_,
          PluginDebuggerHelper_Accessor.ProjectPlatformType.NaCl);
      Assert.AreEqual(
          target.pluginProjectDirectory_,
          TestContext.DeploymentDirectory + @"\DummyLoop\DummyLoop\");
      Assert.AreEqual(
          target.pluginAssembly_,
          TestContext.DeploymentDirectory + @"\DummyLoop\DummyLoop\NaCl\Debug\DummyLoop.nexe");
      Assert.AreEqual(
          target.pluginOutputDirectory_,
          TestContext.DeploymentDirectory + @"\DummyLoop\DummyLoop\NaCl\Debug\");
      Assert.AreEqual(target.sdkRootDirectory_, expectedSDKRootDir);
      Assert.AreEqual(target.webServerExecutable_, "python.exe");
      ////Assert.AreEqual(target._webServerArguments, "");
      ////Assert.AreEqual(target._gdbPath, "");

      // Setting platform to Pepper should make succeed.
      TestUtilities.SetSolutionConfiguration(
          dte_, projectUniqueName, "Debug", Strings.PepperPlatformName);
      Assert.IsTrue(target.LoadProjectSettings());
      Assert.IsTrue(target.isProperlyInitialized_);
      Assert.AreEqual(
          target.projectPlatformType_,
          PluginDebuggerHelper_Accessor.ProjectPlatformType.Pepper);
      Assert.AreEqual(
          target.pluginProjectDirectory_,
          TestContext.DeploymentDirectory + @"\DummyLoop\DummyLoop\");
      Assert.AreEqual(
          target.pluginAssembly_,
          TestContext.DeploymentDirectory + @"\DummyLoop\Debug\PPAPI\DummyLoop.dll");
      Assert.AreEqual(
          target.pluginOutputDirectory_,
          TestContext.DeploymentDirectory + @"\DummyLoop\Debug\PPAPI\");
      Assert.AreEqual(target.sdkRootDirectory_, expectedSDKRootDir);
      Assert.AreEqual(target.webServerExecutable_, "python.exe");
      ////Assert.AreEqual(target._webServerArguments, "");
      ////Assert.AreEqual(target._gdbPath, "");
    }

    /// <summary>
    /// Checks that VS properly attaches debugger.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void AttachVSDebuggerTest()
    {
      using (Process dummyProc = TestUtilities.StartProcessForKilling("DummyProc", 20))
      {
        try
        {
          PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
          target.projectPlatformType_ = PluginDebuggerHelper_Accessor.ProjectPlatformType.Pepper;
          target.isProperlyInitialized_ = true;

          target.AttachVSDebugger(
              null,
              new NativeClientVSAddIn.PluginDebuggerHelper.PluginFoundEventArgs((uint)dummyProc.Id));

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
    /// A test for StartDebugging.
    /// </summary>
    [TestMethod]
    public void StartDebuggingTest()
    {
      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);

      // Neutralize StartWebServer by providing dummy executable settings.
      target.webServerExecutable_ = "python.exe";
      target.webServerArguments_ = "-c \"print 'test'\"";
      target.pluginProjectDirectory_ = TestContext.DeploymentDirectory;

      // Have the timer call a function to set success to true.
      ManualResetEvent finderSuccess = new ManualResetEvent(false);
      target.pluginFinderTimer_ = new System.Windows.Forms.Timer();
      target.pluginFinderTimer_.Tick += (a, b) => { finderSuccess.Set(); };

      // Check that an exception is thrown if not initialized properly.
      target.isProperlyInitialized_ = false;
      try
      {
        target.StartDebugging();
        Assert.Fail("Debugging started when not initialized");
      }
      catch (Exception)
      {
        // Expected in a proper implementation.
      }

      // Properly start debugging and wait for event signal.
      target.isProperlyInitialized_ = true;
      target.StartDebugging();

      // Pump events waiting for signal, time-out after 10 seconds.
      bool success = false;
      for (int i = 0; i < 20; i++)
      {
        System.Windows.Forms.Application.DoEvents();
        if (finderSuccess.WaitOne(500))
        {
          success = true;
          break;
        }
      }

      Assert.IsTrue(success, "Plug-in finder timer did not fire");
    }

    /// <summary>
    /// This tests that StartWebServer executes webServerExecutable_ with webServerArguments_
    /// as arguments, sets the working directory to the project directory, and hooks
    /// up stdout and stderr from the web server to the Web Server output panel in VS.
    /// This test implicitly covers WebServerMessageReceive.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void StartWebServerTest()
    {
      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
      try
      {
        string successMessage = "successful test!";
        string stderrMessage = "stderr test";
        target.webServerExecutable_ = "python.exe";

        // To save pain, if modifying this in the future avoid special characters,
        // or make sure to double escape them. Ex: \n --> \\n.
        string program =
          "import os;" +
          "import sys;" +
          string.Format("sys.stdout.write('{0}');", successMessage) +
          string.Format("sys.stderr.write('{0}');", stderrMessage) +
          "sys.stdout.write(os.getcwd());" +
          "sys.stdout.flush();" +
          "sys.stderr.flush()";
        target.webServerArguments_ = string.Format("-c \"{0}\"", program);
        target.pluginProjectDirectory_ = TestContext.DeploymentDirectory;
        target.isProperlyInitialized_ = true;
        target.StartWebServer();

        // Check that the output pane exists.
        EnvDTE.OutputWindowPane window = dte_.ToolWindows.OutputWindow.OutputWindowPanes.Item(
          Strings.WebServerOutputWindowTitle);
        Assert.IsNotNull(window, "Web server output pane failed to create");

        // Wait for results to arrive for up to 10 seconds, checking every 0.5 seconds.
        string result = TestUtilities.GetPaneText(target.webServerOutputPane_);
        for (int repeat = 0; repeat < 20; repeat++)
        {
          if (result != null &&
              result.Contains(successMessage) &&
              result.Contains(stderrMessage) &&
              result.Contains(TestContext.DeploymentDirectory))
          {
            break;
          }

          Thread.Sleep(500);
          result = TestUtilities.GetPaneText(target.webServerOutputPane_);
        }

        Assert.IsFalse(string.IsNullOrEmpty(result), "Nothing printed to output pane");
        StringAssert.Contains(
            result,
            successMessage,
            "Executable did not successfully run given arguments");
        StringAssert.Contains(
            result,
            TestContext.DeploymentDirectory,
            "Web server working directory was not set to project directory");
        StringAssert.Contains(result, stderrMessage, "Standard error message was not captured");
      }
      finally
      {
        if (!target.webServer_.WaitForExit(1000))
        {
          target.webServer_.Kill();
          target.webServer_.Dispose();
        }
      }
    }

    /// <summary>
    /// Ensures that StopDebugging() kills GDB and the web server, and resets the state of
    /// PluginDebuggerHelper to before debugging started.
    /// Implicitly tests KillGDBProcess().
    /// </summary>
    [TestMethod]
    public void StopDebuggingTest()
    {
      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
      string webServerIdentifier = "StartVisualStudioInstance_TestWebServer";
      string gdbIdentifier = "StartVisualStudioInstance_TestGDB";

      // Set up items that should exist given a successful calling of StartDebugging().
      target.gdbInitFileName_ = Path.GetTempFileName();
      target.pluginFinderForbiddenPids_.Add(123);
      target.webServer_ = TestUtilities.StartProcessForKilling(webServerIdentifier, 20);
      target.gdbProcess_ = TestUtilities.StartProcessForKilling(gdbIdentifier, 20);
      target.isProperlyInitialized_ = true;

      target.StopDebugging();

      Assert.IsFalse(target.isProperlyInitialized_, "Failed to set initialized state to false");
      Assert.IsFalse(target.pluginFinderTimer_.Enabled, "Plug-in finder timer not stopped");
      Assert.IsFalse(
          TestUtilities.DoesProcessExist("python.exe", webServerIdentifier),
          "Failed to kill web server process");
      Assert.IsFalse(
          TestUtilities.DoesProcessExist("python.exe", gdbIdentifier),
          "Failed to kill gdb process");
      Assert.IsFalse(
          File.Exists(target.gdbInitFileName_),
          "Failed to delete temp gdb init file");
      Assert.IsTrue(
          target.pluginFinderForbiddenPids_.Count == 0,
          "Plugin finder Process IDs not cleared");
    }

    /// <summary>
    /// A test for WebServerWriteLine.
    /// </summary>
    [TestMethod]
    [DeploymentItem("NativeClientVSAddIn.dll")]
    public void WebServerWriteLineTest()
    {
      PluginDebuggerHelper_Accessor target = new PluginDebuggerHelper_Accessor(dte_);
      string successMessage = "successful test!";
      target.webServerOutputPane_ = dte_.ToolWindows.OutputWindow.OutputWindowPanes.Add(
          Strings.WebServerOutputWindowTitle);
      target.isProperlyInitialized_ = true;
      target.WebServerWriteLine(successMessage);
      string result = TestUtilities.GetPaneText(target.webServerOutputPane_);

      // Wait for results to arrive for up to 10 seconds, checking every 0.5 seconds.
      for (int repeat = 0; repeat < 20; repeat++)
      {
        if (result != null &&
            result.Contains(successMessage))
        {
          break;
        }

        Thread.Sleep(500);
        result = TestUtilities.GetPaneText(target.webServerOutputPane_);
      }

      StringAssert.Contains(result, successMessage, "Message failed to print");
    }
  }
}
