// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.IO;

namespace NaCl.Build.CPPTasks
{
    class DependencyParser
    {
        private static char[]   elementEndings = new char[] { ' ', '\n', '\\' };
        private List<string>    m_dependencies;
        public List<String> Dependencies
        {
            get
            {
                return m_dependencies;
            }
        }

        public DependencyParser(string filename)
        {
            m_dependencies = new List<string>();

            using (StreamReader reader = new StreamReader(filename, Encoding.ASCII))
            {
                while (!reader.EndOfStream)
                {
                    string str = reader.ReadLine();
                    ParseLine(str);
                }
                reader.Close();
            }
        }

        private void ParseLine(string line)
        {
            string[] paths = line.Split(elementEndings, StringSplitOptions.RemoveEmptyEntries);

            foreach (string path in paths)
            {
                // ignore the source object file
                // assumes .o file is only possible file ending with 'o'
                if (path.ElementAt(path.Length - 1) != 'o' && path.ElementAt(path.Length - 1) != ':')
                {
                    string newDependency = GCCUtilities.ConvertPathPosixToWindows(path);
                    m_dependencies.Add(newDependency);
                }
            }
        }
    }
}
