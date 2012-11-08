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
    public class NaClLib : TrackedVCToolTask
    {
        public bool BuildingInIDE { get; set; }

        [Required]
        public string LibrarianToolPath { get; set; }

        [Required]
        public string PropertiesFile { get; set; }

        [Required]
        public virtual string OutputFile { get; set; }

        [Required]
        public string OutputCommandLine { get; set; }

        [Required]
        public virtual ITaskItem[] Sources { get; set; }

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

        protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
        {
            if (OutputCommandLine == "true")
            {
                Log.LogMessage(MessageImportance.High, pathToTool + "  " + responseFileCommands);
            }

            return base.ExecuteTool(pathToTool, responseFileCommands, commandLineCommands);
        }



        public virtual string PlatformToolset
        {
            get
            {
                return "GCC";
            }
        }

        protected override bool MaintainCompositeRootingMarkers
        {
            get
            {
                return true;
            }
        }

        protected override ITaskItem[] TrackedInputFiles
        {
            get
            {
                return Sources;
            }
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

        protected override string TrackerIntermediateDirectory
        {
            get
            {
                if (this.TrackerLogDirectory != null)
                {
                    return this.TrackerLogDirectory;
                }
                else
                {
                    return string.Empty;
                }
            }
        }

        protected override string CommandTLogName
        {
            get
            {
                return "default.link.command.tlog";
            }
        }

        protected override string[] ReadTLogNames
        {
            get
            {
                return new string[]
                {
                    "default.link.read.tlog"
                };
            }
        }

        protected override string[] WriteTLogNames
        {
            get
            {
                return new string[]
                {
                    "default.link.write.tlog"
                };
            }
        }

        public string TrackerLogDirectory { get; set; }
    }


}
