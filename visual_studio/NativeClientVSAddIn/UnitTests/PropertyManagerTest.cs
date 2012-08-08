// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace UnitTests
{
  using System;
  using System.IO;

  using EnvDTE;
  using EnvDTE80;
  using Microsoft.VisualStudio.TestTools.UnitTesting;

  using NativeClientVSAddIn;

  /// <summary>
  /// This is a test class for PropertyManagerTest and is intended
  /// to contain all PropertyManager Unit Tests
  /// </summary>
  [TestClass]
  public class PropertyManagerTest
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
          "PropertyManagerTest",
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
    /// Tests SetTarget() and SetTargetToActive().
    /// </summary>
    [TestMethod]
    public void SetTargetTest()
    {
      string expectedSDKRootDir =
          Environment.GetEnvironmentVariable(Strings.SDKPathEnvironmentVariable);
      Assert.IsNotNull(expectedSDKRootDir, "SDK Path environment variable not set!");

      PropertyManager target = new PropertyManager();
      dte_.Solution.Open(naclSolution);

      Project naclProject = dte_.Solution.Projects.Item(TestUtilities.BlankNaClProjectUniqueName);
      Project notNacl = dte_.Solution.Projects.Item(TestUtilities.NotNaClProjectUniqueName);

      // Invalid project.
      target.SetTarget(notNacl, Strings.PepperPlatformName, "Debug");
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.Other,
          target.ProjectPlatform,
          "SetTarget should not succeed with non-nacl/pepper project.");

      // Try valid project with different platforms.
      target.SetTarget(naclProject, Strings.NaClPlatformName, "Debug");
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.NaCl,
          target.ProjectPlatform,
          "SetTarget did not succeed with nacl platform on valid project.");
      Assert.AreEqual(expectedSDKRootDir, target.SDKRootDirectory, "SDK Root incorrect.");
      
      target.SetTarget(naclProject, "Win32", "Debug");
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.Other,
          target.ProjectPlatform,
          "SetTarget did not set 'other' platform on when Win32 platform of valid project.");

      target.SetTarget(naclProject, Strings.PepperPlatformName, "Debug");
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.Pepper,
          target.ProjectPlatform,
          "SetTarget did not succeed with pepper platform on valid project.");
      Assert.AreEqual(expectedSDKRootDir, target.SDKRootDirectory, "SDK Root incorrect.");

      // Setting the start-up project to a non-cpp project should make loading fail.
      object[] badStartupProj = { TestUtilities.NotNaClProjectUniqueName };
      dte_.Solution.SolutionBuild.StartupProjects = badStartupProj;
      target.SetTargetToActive(dte_);
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.Other,
          target.ProjectPlatform,
          "SetTargetToActive should not succeed with non-nacl/pepper project.");

      // Setting the start-up project to correct C++ project, but also setting the platform
      // to non-nacl/pepper should make loading fail.
      object[] startupProj = { TestUtilities.BlankNaClProjectUniqueName };
      dte_.Solution.SolutionBuild.StartupProjects = startupProj;
      TestUtilities.SetSolutionConfiguration(
          dte_, TestUtilities.BlankNaClProjectUniqueName, "Debug", "Win32");
      target.SetTargetToActive(dte_);
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.Other,
          target.ProjectPlatform,
          "SetTargetToActive should not succeed with Win32 platform.");

      // Now setting the platform to NaCl should make this succeed.
      TestUtilities.SetSolutionConfiguration(
          dte_, TestUtilities.BlankNaClProjectUniqueName, "Debug", Strings.NaClPlatformName);
      target.SetTargetToActive(dte_);
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.NaCl,
          target.ProjectPlatform,
          "SetTargetToActive should succeed with NaCl platform and valid project.");
      Assert.AreEqual(expectedSDKRootDir, target.SDKRootDirectory, "SDK Root incorrect.");
    }

    /// <summary>
    /// A test for GetProperty. Checks some non-trivial C# properties and the GetProperty method.
    /// </summary>
    [TestMethod]
    public void GetPropertyTest()
    {
      string expectedSDKRootDir =
          Environment.GetEnvironmentVariable(Strings.SDKPathEnvironmentVariable);
      Assert.IsNotNull(expectedSDKRootDir, "SDK Path environment variable not set!");

      // Set up the property manager to read the NaCl platform settings from BlankValidSolution.
      PropertyManager target = new PropertyManager();
      dte_.Solution.Open(naclSolution);
      Project naclProject = dte_.Solution.Projects.Item(TestUtilities.BlankNaClProjectUniqueName);
      target.SetTarget(naclProject, Strings.NaClPlatformName, "Debug");
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.NaCl,
          target.ProjectPlatform,
          "SetTarget did not succeed with nacl platform on valid project.");

      string projectDir = Path.Combine(
          Path.GetDirectoryName(naclSolution),
          Path.GetDirectoryName(TestUtilities.BlankNaClProjectUniqueName)) + @"\";
      string outputDir = Path.Combine(projectDir, "newlib") + @"\";
      string assembly = Path.Combine(outputDir, TestUtilities.BlankNaClProjectName + ".nexe");

      Assert.AreEqual(expectedSDKRootDir, target.SDKRootDirectory, "SDK Root.");
      Assert.AreEqual(projectDir, target.ProjectDirectory, "ProjectDirectory.");
      Assert.AreEqual(outputDir, target.OutputDirectory, "OutputDirectory.");
      Assert.AreEqual(assembly, target.PluginAssembly, "PluginAssembly.");
      Assert.AreEqual(
          @"win_x86_newlib",
          target.GetProperty("ConfigurationGeneral", "PlatformToolset"),
          "GetProperty() with PlatformToolset incorrect.");
    }

    /// <summary>
    /// A test for SetProperty.
    /// </summary>
    [TestMethod]
    public void SetPropertyTest()
    {
      string setTargetSolution = TestUtilities.CreateBlankValidNaClSolution(
          dte_,
          "PropertyManagerTestSetTarget",
          NativeClientVSAddIn.Strings.NaClPlatformName,
          NativeClientVSAddIn.Strings.NaClPlatformName,
          TestContext);

      // Set up the property manager to read the NaCl platform settings from BlankValidSolution.
      PropertyManager target = new PropertyManager();
      dte_.Solution.Open(setTargetSolution);
      Project naclProject = dte_.Solution.Projects.Item(TestUtilities.BlankNaClProjectUniqueName);
      target.SetTarget(naclProject, Strings.NaClPlatformName, "Debug");
      Assert.AreEqual(
          PropertyManager.ProjectPlatformType.NaCl,
          target.ProjectPlatform,
          "SetTarget did not succeed with nacl platform on valid project.");

      string newValue = "ThisIsNew";
      target.SetProperty("ConfigurationGeneral", "VSNaClSDKRoot", newValue);
      Assert.AreEqual(
          newValue,
          target.GetProperty("ConfigurationGeneral", "VSNaClSDKRoot"),
          "SetProperty() did not set property VSNaClSDKRoot.");
    }
  }
}
