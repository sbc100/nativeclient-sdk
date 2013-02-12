// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NaCl.Build.CPPTasks;

namespace UnitTests
{
    [TestClass]
    public class DependencyParserTest
    {
        [TestMethod]
        public void ParseEscapedSpacesTest()
        {
            DependencyParser parser = new DependencyParser();
            parser.Parse(@"test.o: test.c my\ test.h my\ \test.h");
            List<string> expected = new List<string>();
            expected.Add(@"test.c");
            expected.Add(@"my test.h");
            expected.Add(@"my \test.h");
            CollectionAssert.AreEquivalent(expected, parser.Dependencies);
        }

        [TestMethod]
        public void ParseMultiLineTest()
        {
            DependencyParser parser = new DependencyParser();
            parser.Parse("test.o: test.c \\\n foo.h\\\n bar.h   baz.h");
            List<string> expected = new List<string>();
            expected.Add(@"test.c");
            expected.Add(@"foo.h");
            expected.Add(@"bar.h");
            expected.Add(@"baz.h");
            CollectionAssert.AreEquivalent(expected, parser.Dependencies);
        }
    }
}
