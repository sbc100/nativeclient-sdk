// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.IO;
using System.Reflection;
using System.Resources;
using System.Text.RegularExpressions;
using System.Diagnostics;

using Microsoft.Build.Framework;
using Microsoft.Build.CPPTasks;
using Microsoft.Build.Utilities;

namespace NaCl.Build.CPPTasks
{
    public class NaClLib : NaClToolTask
    {
        [Required]
        public string LibrarianToolPath { get; set; }

        public NaClLib()
            : base(new ResourceManager("NaCl.Build.CPPTasks.Properties.Resources", Assembly.GetExecutingAssembly()))
        {
          this.EnvironmentVariables = new string[] { "CYGWIN=nodosfilewarning", "LC_CTYPE=C" };
        }

        protected override string GenerateResponseFileCommands()
        {
            StringBuilder responseFileCmds = new StringBuilder(GCCUtilities.s_CommandLineLength);
            responseFileCmds.Append("rcs ");
            responseFileCmds.Append(GCCUtilities.ConvertPathWindowsToPosix(OutputFile));

            foreach (ITaskItem item in Sources)
            {
                responseFileCmds.Append(" ");
                responseFileCmds.Append(GCCUtilities.ConvertPathWindowsToPosix(item.ToString()));
            }
            return responseFileCmds.ToString();
        }

        public override bool Execute()
        {
            if (!Setup())
                return false;

            return base.Execute();
        }

        protected override string CommandTLogFilename
        {
            get
            {
                return BaseTool() + ".lib.command.1.tlog";
            }
        }

        protected override string[] ReadTLogFilenames
        {
            get
            {
                return new string[] { BaseTool() + ".lib.read.1.tlog" };
            }
        }

        protected override string WriteTLogFilename
        {
            get
            {
                return BaseTool() + ".lib.write.1.tlog";
            }
        }

        protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
        {
            if (OutputCommandLine)
                Log.LogMessage(MessageImportance.High, pathToTool + "  " + responseFileCommands);

            return base.ExecuteTool(pathToTool, responseFileCommands, commandLineCommands);
        }

        protected override Encoding ResponseFileEncoding
        {
            get
            {
                return Encoding.ASCII;
            }
        }

        protected override string ToolName
        {
            get
            {
                return LibrarianToolPath;
            }
        }
    }
}
