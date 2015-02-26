// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.Text;
using System.Collections.Generic;
using EnvDTE80;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.VCProjectEngine;
using NaCl.Build.CPPTasks;
using NativeClientVSAddIn;

namespace UnitTests
{
    [TestClass]
    public class ComileTest : BaseCompileTest
    {
        private static string SolutionBaseName_ = "CompileTest";

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
        /// Test method to check that the NaCl platform compiles a test project.
        /// </summary>
        [TestMethod]
        public void CheckNaCl64Compile()
        {
            CheckCompile(Strings.NaCl64PlatformName);
        }

        /// <summary>
        /// Test method to check that the NaCl platform compiles a test project.
        /// </summary>
        [TestMethod]
        public void CheckNaClARMCompile()
        {
            string root = System.Environment.GetEnvironmentVariable("NACL_SDK_ROOT");
            if (!SDKUtilities.SupportsARM(root))
            {
                Assert.Inconclusive();
            }
            CheckCompile(Strings.NaClARMPlatformName);
        }

        /// <summary>
        /// Test method to check that the Pepper platform compiles a test project.
        /// </summary>
        [TestMethod]
        public void CheckPepperCompile()
        {
            if (!TestUtilities.IsVS2012())
            {
                // The pre-compiled libraries in the SDK (specifically
                // ppapi_cpp.lib) are not linkable except with 2012 so
                // this test will always fail with link errors under 2010
                // or any other Visual Studio version.
                Assert.Inconclusive();
            }
            CheckCompile(Strings.PepperPlatformName);
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
            CheckCompile(Strings.PNaClPlatformName);
        }

    }
}
