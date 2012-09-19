// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;

  using EnvDTE;
  using EnvDTE80;
  using Microsoft.VisualStudio.TestTools.UnitTesting;

  using NativeClientVSAddIn;

  /// <summary>
  /// This is a test class for WebServerTest and is intended
  /// to contain all WebServer Unit Tests
  /// </summary>
  [TestClass]
  public class WebServerTest
  {
    /// <summary>
    /// The main visual studio object.
    /// </summary>
    private DTE2 dte_;

    /// <summary>
    /// Gets or sets the test context which provides
    /// information about and functionality for the current test run.
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
    /// A test for WebServer Constructor. Starts the web server.
    /// </summary>
    [TestMethod]
    public void WebServerConstructorTest()
    {
      OutputWindowPane outputWindowPane = dte_.ToolWindows.OutputWindow.OutputWindowPanes.Add(
          Strings.WebServerOutputWindowTitle);

      // Set up mock property manager to return the desired property values.
      MockPropertyManager properties = new MockPropertyManager(
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
                case "NaClWebServerPort": return "5105";
              }

              break;
            case "Property":
              switch (name)
              {
                case "ProjectDirectory": return TestContext.DeploymentDirectory;
              }

              break;
          }

          return null;
        },
        null);

      WebServer target = null;
      try
      {
        target = new WebServer(outputWindowPane, properties);

        TestUtilities.AssertTrueWithTimeout(
          () => !string.IsNullOrEmpty(TestUtilities.GetPaneText(outputWindowPane)),
          TimeSpan.FromMilliseconds(500),
          20,
          "Pane text never appeared");

        TestUtilities.AssertTrueWithTimeout(
            () => TestUtilities.DoesProcessExist("python.exe", "5105", "httpd.py"),
            TimeSpan.FromMilliseconds(500),
            20,
            "Web server failed to start.");

        target.Dispose();

        TestUtilities.AssertTrueWithTimeout(
            () => !TestUtilities.DoesProcessExist("python.exe", "5105", "httpd.py"),
            TimeSpan.FromMilliseconds(500),
            20,
            "Web server failed to shut down.");
      }
      finally
      {
        if (target != null)
        {
          target.Dispose();
        }
      }
    }
  }
}
