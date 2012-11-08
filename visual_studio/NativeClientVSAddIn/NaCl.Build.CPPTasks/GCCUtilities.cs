// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.IO;
using System.Text.RegularExpressions;

namespace NaCl.Build.CPPTasks
{
    /// <summary>
    /// Utility functions for dealing with gcc toolchain on windows.
    /// </summary>
    class GCCUtilities
    {
        public const int s_CommandLineLength = 256;

        /// <summary>
        /// Add quotes around a string, if they are needed.
        /// </summary>
        /// <returns>The input arg surrounded by quotes as appropriate.</returns>
        public static string QuoteIfNeeded(string arg)
        {
            var match = arg.IndexOfAny(new char[] { ' ', '\t', ';', '&' }) != -1;
            if (!match)
                return arg;
            return "\"" + arg + "\"";
        }

        /// <summary>
        /// Convert windows path in to a cygwin path suitable for passing to gcc
        /// command line.
        /// </summary>
        public static string ConvertPathWindowsToPosix(string path)
        {
            string rtn = path.Replace('\\', '/');
            rtn = QuoteIfNeeded(rtn);
            return rtn;
        }

        public static string ConvertPathPosixToWindows(string path)
        {
            path = path.Replace('/', '\\');
            // also make double backslashes a single slash
            // TODO: speed this up by iterating instead of two replaces
            return path.Replace("\\\\", "\\");
        }

        /// <summary>
        /// Replace GCC error are warning output to Visual Studio format to
        /// support going to source code from error output.
        /// </summary>
        public static string ConvertGCCOutput(string line)
        {
            string result;
            foreach (GCCRegexLineConverter converter in s_RegexConverters)
            {
                result = converter.Convert(line);
                if (result.Length > 0)
                {
                    return result;
                }
            }

            return line;
        }

        private class GCCRegexLineConverter
        {
            public Regex OutputExpression { get; set; }

            //public string OutputIdentifier { get; set; }
            public string FilenameIdentifier { get; set; }
            public string RemainderIdentifier { get; set; }

            // returns empty string if cannot convert.
            public string Convert(string line)
            {
                //Regex expression = new Regex(OutputIdentifier);
                if (OutputExpression.IsMatch(line))
                {
                    string filename = OutputExpression.Replace(line, FilenameIdentifier);

                    // GetFullPath may throw an exception.
                    try
                    {
                        string fullPath = Path.GetFullPath(filename);
                        string remainder = OutputExpression.Replace(line, RemainderIdentifier);

                        string newLine = fullPath + remainder;

                        return newLine;
                    }
                    catch
                    {
                    }
                }

                return string.Empty;
            }
        } // GCCRegexLineConverter

        private static readonly List<GCCRegexLineConverter> s_RegexConverters = new List<GCCRegexLineConverter>
        {
            new GCCRegexLineConverter
            {
                OutputExpression =      new Regex(@"^\s*(.?.?[^:]*.*?):([1-9]\d*):([1-9]\d*):(.*$)"),
                FilenameIdentifier =    @"$1",
                RemainderIdentifier =   @"($2,$3):$4"
            },
            new GCCRegexLineConverter
            {
                OutputExpression =      new Regex(@"^\s*(.?.?[^:]*.*?):([1-9]\d*):(.*$)"),
                FilenameIdentifier =    @"$1",
                RemainderIdentifier =   @"($2):$3"
            },
            new GCCRegexLineConverter
            {
                OutputExpression =      new Regex(@"^\s*(.?.?[^:]*.*?):(.?.?[^:]*.*?):([1-9]\d*):(.*$)"),
                FilenameIdentifier =    @"$2",
                RemainderIdentifier =   @"($3):'$1' $4"
            }
        };
    } // GCCUtilities
} // namespace
