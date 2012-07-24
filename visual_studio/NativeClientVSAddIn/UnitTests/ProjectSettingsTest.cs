// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;

  using EnvDTE;
  using EnvDTE80;
  using Microsoft.VisualStudio.TestTools.UnitTesting;
  using Microsoft.VisualStudio.VCProjectEngine;

  /// <summary>
  /// This test class contains tests related to the custom project settings
  /// and property pages for PPAPI and NaCl configurations.
  /// </summary>
  [TestClass]
  public class ProjectSettingsTest
  {
    /// <summary>
    /// The ProjectSettingsTest solution is a valid nacl/pepper plug-in VS solution
    /// that has not had the custom platforms added (PPAPI and NaCl). Immediately
    /// before these unit tests are run the project is copied into the testing
    /// deployment directory, and the custom platforms are added to this copy so that
    /// the project settings are based on the most recent template. Because unit-tests
    /// run in any order, the solution should not be written to in any test.
    /// </summary>
    private const string ProjectSettingsTestSolution =
        @"\ProjectSettingsTest\ProjectSettingsTest.sln";

    /// <summary>
    /// This is the project corresponding to ProjectSettingsTestSolution.
    /// </summary>
    private const string ProjectSettingsTestProject =
        @"ProjectSettingsTest\ProjectSettingsTest.vcxproj";

    /// <summary>
    /// The main visual studio object.
    /// </summary>
    private DTE2 dte_;

    /// <summary>
    /// The project configuration for debug settings of a test's platform.
    /// </summary>
    private VCConfiguration debug_;

    /// <summary>
    /// The project configuration for release settings of a test's platform
    /// </summary>
    private VCConfiguration release_;

    /// <summary>
    /// Gets or sets the test context which provides information about,
    /// and functionality for the current test run.
    /// </summary>
    public TestContext TestContext { get; set; }

    /// <summary>
    /// This is run one time before any test methods are called. Here we set-up the testing copy
    /// of ProjectSettingsTest to use the most up-to-date custom project settings.
    /// </summary>
    /// <param name="testContext">Holds information about the current test run</param>
    [ClassInitialize]
    public static void ClassSetup(TestContext testContext)
    {
      DTE2 dte = null;
      try
      {
        dte = TestUtilities.StartVisualStudioInstance();
        dte.Solution.Open(testContext.DeploymentDirectory + ProjectSettingsTestSolution);
        Project proj = dte.Solution.Projects.Item(ProjectSettingsTestProject);

        proj.ConfigurationManager.AddPlatform(
            NativeClientVSAddIn.Strings.PepperPlatformName,
            NativeClientVSAddIn.Strings.PepperPlatformName,
            true);

        proj.ConfigurationManager.AddPlatform(
            NativeClientVSAddIn.Strings.NaClPlatformName,
            NativeClientVSAddIn.Strings.NaClPlatformName,
            true);

        proj.Save();
        dte.Solution.SaveAs(testContext.DeploymentDirectory + ProjectSettingsTestSolution);
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
    /// Test method to check that the PPAPI platform template correctly sets default values.
    /// </summary>
    [TestMethod]
    public void VerifyDefaultPepperSettings()
    {
      string page;

      // Extract the debug and release configurations for Pepper from the project.
      dte_.Solution.Open(TestContext.DeploymentDirectory + ProjectSettingsTestSolution);
      Project project = dte_.Solution.Projects.Item(ProjectSettingsTestProject);
      Assert.IsNotNull(project, "Testing project was not found");
      string pepperPlatform = NativeClientVSAddIn.Strings.PepperPlatformName;
      debug_ = TestUtilities.GetVCConfiguration(project, "Debug", pepperPlatform);
      release_ = TestUtilities.GetVCConfiguration(project, "Release", pepperPlatform);

      // General
      page = "ConfigurationGeneral";
      AllConfigsAssertPropertyEquals(page, "OutDir", @"$(ProjectDir)Win\", true);
      AllConfigsAssertPropertyEquals(page, "IntDir", @"$(ProjectDir)Intermediate\Win\", true);
      AllConfigsAssertPropertyEquals(page, "TargetExt", ".dll", true);
      AllConfigsAssertPropertyEquals(page, "ConfigurationType", "DynamicLibrary", true);
      AllConfigsAssertPropertyEquals(page, "VSNaClSDKRoot", @"$(NACL_SDK_ROOT)\", false);
      
      // Debugging
      page = "WindowsLocalDebugger";
      AllConfigsAssertPropertyEquals(
          page,
          "LocalDebuggerCommand",
          @"$(CHROME_PATH)\chrome.exe",
          true);
      AllConfigsAssertPropertyEquals(
          page,
          "LocalDebuggerCommandArguments",
          "--register-pepper-plugins=\"$(TargetPath)\";application/x-nacl localhost:5103",
          true);

      // VC++ Directories
      page = "ConfigurationDirectories";
      AllConfigsAssertPropertyContains(page, "IncludePath", @"$(VSNaClSDKRoot)include;", true);
      AllConfigsAssertPropertyContains(page, "IncludePath", @"$(VCInstallDir)include", true);
      AllConfigsAssertPropertyContains(page, "LibraryPath", @"$(VSNaClSDKRoot)lib;", true);
      AllConfigsAssertPropertyContains(page, "LibraryPath", @"$(VCInstallDir)lib", true);

      // C/C++ Code Generation
      page = "CL";
      TestUtilities.AssertPropertyEquals(
          debug_,
          page,
          "RuntimeLibrary",
          "MultiThreadedDebug",
          false);
      TestUtilities.AssertPropertyEquals(release_, page, "RuntimeLibrary", "MultiThreaded", false);

      // Linker Input
      page = "Link";
      AllConfigsAssertPropertyContains(page, "AdditionalDependencies", "ppapi_cpp.lib", false);
      AllConfigsAssertPropertyContains(page, "AdditionalDependencies", "ppapi.lib", false);
    }

    /// <summary>
    /// Tests that a given property has a specific value for both Debug and Release
    /// configurations under the current test's platform.
    /// </summary>
    /// <param name="pageName">Property page name where property resides.</param>
    /// <param name="propertyName">Name of the property to check.</param>
    /// <param name="expectedValue">Expected value of the property.</param>
    /// <param name="ignoreCase">Ignore case when comparing the expected and actual values.</param>
    private void AllConfigsAssertPropertyEquals(
        string pageName,
        string propertyName,
        string expectedValue,
        bool ignoreCase)
    {
      TestUtilities.AssertPropertyEquals(
          debug_,
          pageName,
          propertyName,
          expectedValue,
          ignoreCase);
      TestUtilities.AssertPropertyEquals(
          release_,
          pageName,
          propertyName,
          expectedValue,
          ignoreCase);
    }

    /// <summary>
    /// Tests that a given property contains a specific string for both Debug and Release
    /// configurations under the NaCl platform.
    /// </summary>
    /// <param name="pageName">Property page name where property resides.</param>
    /// <param name="propertyName">Name of the property to check.</param>
    /// <param name="expectedValue">Expected value of the property.</param>
    /// <param name="ignoreCase">Ignore case when comparing the expected and actual values.</param>
    private void AllConfigsAssertPropertyContains(
        string pageName,
        string propertyName,
        string expectedValue,
        bool ignoreCase)
    {
      TestUtilities.AssertPropertyContains(
          debug_,
          pageName,
          propertyName,
          expectedValue,
          ignoreCase);
      TestUtilities.AssertPropertyContains(
          release_,
          pageName,
          propertyName,
          expectedValue,
          ignoreCase);
    }
  }
}
