// Copyright (c) 2013 The Chromium Authors. All rights reserved.
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
using NativeClientVSAddIn;

namespace UnitTests
{
    [TestClass]
    public class BaseCompileTest
    {
        /// <summary>
        /// The main visual studio object.
        /// </summary>
        protected DTE2 dte_;

        /// <summary>
        /// The path to a NaCl solution used in compile tests.
        /// </summary>
        protected static string SolutionName_;

        /// <summary>
        /// This is run one time before any test methods are called. Here we set-up test-copies of
        /// new NaCl solutions for use in the tests.
        /// </summary>
        /// <param name="testContext">Holds information about the current test run</param>
        public static void ClassSetUp(TestContext testContext, string solutionBaseName)
        {
            DTE2 dte = TestUtilities.StartVisualStudioInstance();
            try
            {
                SolutionName_ = TestUtilities.CreateBlankValidNaClSolution(
                    dte,
                    solutionBaseName,
                    Strings.PepperPlatformName,
                    Strings.NaCl64PlatformName,
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
                TestUtilities.AssertAddinLoaded(dte_, Strings.AddInName);
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
            TestUtilities.SetSolutionConfiguration(
                dte_, TestUtilities.NaClProjectUniqueName, configName, platformName);
            dte_.Solution.SolutionBuild.Build(true);

            string compileOutput = TestUtilities.GetPaneText(
                dte_.ToolWindows.OutputWindow.OutputWindowPanes.Item("Build"));
            Assert.IsTrue(
                compileOutput.Contains("Build succeeded.", ignoreCase),
                string.Format(failFormat, platformName, configName, compileOutput));
            Assert.IsFalse(
                compileOutput.Contains("MS-DOS style path detected", ignoreCase),
                string.Format(cygwinWarningFormat, platformName, configName));
        }

        /// <summary>
        /// Set the type of the project: Executable, DynamicLibrary, StaticLibrary
        /// </summary>
        private void SetProjectType(string projectType, string platformName)
        {
            Project project = dte_.Solution.Projects.Item(TestUtilities.NaClProjectUniqueName);
            TestUtilities.SetProjectType(project, projectType, platformName);
        }

        protected void CheckCompile(string platform, bool dll)
        {
            dte_.Solution.Open(SolutionName_);
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
            dte_.Solution.Close(true);
        }
    }
}
