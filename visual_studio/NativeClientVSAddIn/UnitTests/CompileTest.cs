// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.Text;
using System.Collections.Generic;
using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.VCProjectEngine;
using NaCl.Build.CPPTasks;

namespace UnitTests
{
    [TestClass]
    public class CompileTest
    {
        /// <summary>
        /// The main visual studio object.
        /// </summary>
        private DTE2 dte_;

        /// <summary>
        /// The path to a NaCl solution used in compile tests.
        /// </summary>
        private static string SolutionName_;

        /// <summary>
        /// This is run one time before any test methods are called. Here we set-up test-copies of
        /// new NaCl solutions for use in the tests.
        /// </summary>
        /// <param name="testContext">Holds information about the current test run</param>
        [ClassInitialize]
        public static void ClassSetUp(TestContext testContext)
        {
            DTE2 dte = TestUtilities.StartVisualStudioInstance();
            try
            {
                SolutionName_ = TestUtilities.CreateBlankValidNaClSolution(
                    dte,
                    "CompileTest",
                    NativeClientVSAddIn.Strings.PepperPlatformName,
                    NativeClientVSAddIn.Strings.NaCl64PlatformName,
                    testContext);
            }
            finally
            {
                TestUtilities.CleanUpVisualStudioInstance(dte);
            }
        }

        /// <summary>
        /// This is run after each test to clean up things created in TestSetup().
        /// </summary>
        [TestCleanup]
        public void ClassTearDown()
        {
            TestUtilities.CleanUpVisualStudioInstance(dte_);
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
        /// Helper function which opens the given solution, sets the configuration and platform and
        /// tries to compile, failing the test if the build does not succeed.
        /// </summary>
        /// <param name="solutionPath">Path to the solution to open.</param>
        /// <param name="configName">Solution Configuration name (Debug or Release).</param>
        /// <param name="platformName">Platform name.</param>
        private void TryCompile(string configName, string platformName)
        {
            string failFormat = "Project compile failed for {0} platform {1} config."
                              + "Build output: {2}";
            string cygwinWarningFormat = "Did not pass cygwin nodosfilewarning environment var to"
                                       + " tools Platform: {0}, configuration: {1}";
            StringComparison ignoreCase = StringComparison.InvariantCultureIgnoreCase;

            // Open Debug configuration and build.
            dte_.Solution.Open(SolutionName_);
            TestUtilities.SetSolutionConfiguration(
                dte_, TestUtilities.BlankNaClProjectUniqueName, configName, platformName);
            dte_.Solution.SolutionBuild.Build(true);

            string compileOutput = TestUtilities.GetPaneText(
                dte_.ToolWindows.OutputWindow.OutputWindowPanes.Item("Build"));
            Assert.IsTrue(
                compileOutput.Contains("Build succeeded.", ignoreCase),
                string.Format(failFormat, platformName, configName, compileOutput));
            Assert.IsFalse(
                compileOutput.Contains("MS-DOS style path detected", ignoreCase),
                string.Format(cygwinWarningFormat, platformName, configName));

            dte_.Solution.Close(true);
        }

        /// <summary>
        /// Set the type of the project: Executable, DynamicLibrary, StaticLibrary
        /// </summary>
        private void SetProjectType(string projectType, string platformName)
        {
            dte_.Solution.Open(SolutionName_);
            Project project = dte_.Solution.Projects.Item(TestUtilities.BlankNaClProjectUniqueName);
            VCConfiguration config;
            IVCRulePropertyStorage rule;

            config = TestUtilities.GetVCConfiguration(project, "Debug", platformName);
            rule = config.Rules.Item("ConfigurationGeneral");
            rule.SetPropertyValue("ConfigurationType", projectType);

            config = TestUtilities.GetVCConfiguration(project, "Release", platformName);
            rule = config.Rules.Item("ConfigurationGeneral");
            rule.SetPropertyValue("ConfigurationType", projectType);
            dte_.Solution.Close(true);
        }

        /// <summary>
        /// Test method to check that the NaCl platform compiles a test project.
        /// </summary>
        [TestMethod]
        public void CheckNaClCompile()
        {
            CheckCompile(NativeClientVSAddIn.Strings.NaCl64PlatformName, false);
        }

        /// <summary>
        /// Test method to check that the Pepper platform compiles a test project.
        /// </summary>
        [TestMethod]
        public void CheckPepperCompile()
        {
            CheckCompile(NativeClientVSAddIn.Strings.PepperPlatformName, true);
        }

        /// <summary>
        /// Test method to check that the NaCl platform compiles a test project.
        /// </summary>
        [TestMethod]
        public void CheckPNaClCompile()
        {
            string root = System.Environment.GetEnvironmentVariable("NACL_SDK_ROOT");
            if (!SDKUtilities.SupportsPNaCl(root))
            {
                Assert.Inconclusive();
            }
            CheckCompile(NativeClientVSAddIn.Strings.PNaClPlatformName, false);
        }

        private void CheckCompile(string platform, bool dll)
        {
            SetProjectType("Executable", platform);
            TryCompile("Debug", platform);
            TryCompile("Release", platform);
            SetProjectType("StaticLibrary", platform);
            TryCompile("Debug", platform);
            TryCompile("Release", platform);
            if (dll)
            {
                SetProjectType("DynamicLibrary", platform);
                TryCompile("Debug", platform);
                TryCompile("Release", platform);
            }
        }
    }
}
