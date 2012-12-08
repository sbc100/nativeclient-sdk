// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace NaCl.Build.CPPTasks
{
    using System;
    using System.IO;
    using System.Collections.Generic;

    /// <summary>
    /// TODO: Update summary.
    /// </summary>
    public class SDKUtilities
    {
        // The first version of pepper with a known working PNaCl toolchain
        public const int MinPNaCLSDKVersion = 24;
        public const int MinPNaCLSDKRevision = 166868;

        /// <summary>
        /// Find python executable in user's PATH.
        /// </summary>
        public static bool FindPython()
        {
            string envvar = Environment.GetEnvironmentVariable("Path", EnvironmentVariableTarget.Process);
            List<string> pathList = new List<string>(envvar.Split(';'));
            foreach (string path in pathList)
            {
                string testPath = Path.Combine(path, "python.bat");
                if (File.Exists(testPath))
                {
                    return true;
                }
                testPath = Path.Combine(path, "python.exe");
                if (File.Exists(testPath))
                {
                    return true;

                }
            }
            return false;
        }

        /// <summary>
        /// Retrieve the version and revsion of the given NaCl SDK root.
        /// </summary>
        public static int GetSDKVersion(string root, out int revision)
        {
            // Determine version by parsing top level README file.
            string[] lines = File.ReadAllLines(Path.Combine(root, "README"));
            int version = -1;
            revision = -1;
            foreach (var line in lines)
            {
                if (line.StartsWith("Revision"))
                    revision = Convert.ToInt32(line.Split(':')[1]);
                if (line.StartsWith("Version"))
                    version = Convert.ToInt32(line.Split(':')[1]);
            }
            return version;
        }

        /// <summary>
        /// Retrun true if the NaCl SDK at the given location supports
        /// the PNaCl toolchain.
        /// </summary>
        public static bool SupportsPNaCl(string root)
        {
            int revision;
            int version = GetSDKVersion(root, out revision);
            if (version > MinPNaCLSDKVersion)
                return true;
            if (version == MinPNaCLSDKVersion && revision >= MinPNaCLSDKRevision)
                return true;
            return false;
        }
    }
}
