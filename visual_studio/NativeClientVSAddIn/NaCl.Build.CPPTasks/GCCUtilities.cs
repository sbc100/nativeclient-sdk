using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.IO;
using System.Text.RegularExpressions;

namespace NaCl.Build.CPPTasks
{
    class GCCUtilities
    {
        public const int s_CommandLineLength = 256;

        public static string ConvertPathWindowsToPosix(string path)
        {
            return path.Replace('\\', '/');
        }

        public static string ConvertPathPosixToWindows(string path)
        {
            path = path.Replace('/', '\\');
            // also make double backslashes a single slash
            // TODO: speed this up by iterating instead of two replaces
            return path.Replace("\\\\", "\\");
        }

        // replace GCC error are warning output to Visual Studio format to support going to source code from error output.
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
