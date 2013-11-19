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
    public class CompileParallelTest : BaseCompileTest
    {
        private static string SolutionBaseName_ = "CompileParallelTest";

        [ClassInitialize]
        public static void ClassSetUp(TestContext testContext)
        {
            BaseCompileTest.ClassSetUp(testContext, SolutionBaseName_);
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
        /// Test that parallel compilation works.
        /// </summary>
        [TestMethod]
        public void CheckParallelCompile()
        {
            string platform = Strings.NaCl64PlatformName;
            dte_.Solution.Open(SolutionName_);
            Project project = dte_.Solution.Projects.Item(TestUtilities.NaClProjectUniqueName);

            // Add property sheet that enables multiprocessing
            VCConfiguration config;
            config = TestUtilities.GetVCConfiguration(project, "Debug", platform);
            config.AddPropertySheet("multiprocess.props");
            config = TestUtilities.GetVCConfiguration(project, "Release", platform);
            config.AddPropertySheet("multiprocess.props");

            // Add a second file
            project.ProjectItems.AddFromFile("test_file.cpp");

            dte_.Solution.Close(true);

            CheckCompile(platform);
        }
    }
}
