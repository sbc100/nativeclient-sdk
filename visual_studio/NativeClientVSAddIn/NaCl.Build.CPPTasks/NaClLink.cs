﻿// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.IO;
using System.Resources;
using System.Reflection;
using System.Text;
using Microsoft.Build.Framework;
using Microsoft.Build.CPPTasks;
using Microsoft.Build.Utilities;


namespace NaCl.Build.CPPTasks
{
    public class NaClLink : NaClToolTask
    {
        /// <summary>
        /// Property set only in PNaCl builds to signal that the translator
        /// should be run post-link.
        /// </summary>
        public bool TranslateARM { get; set; }

        /// <summary>
        /// Property set only in PNaCl builds to signal that the translator
        /// should be run post-link.
        /// </summary>
        public bool TranslateX86 { get; set; }

        /// <summary>
        /// Property set only in PNaCl builds to signal that the translator
        /// should be run post-link.
        /// </summary>
        public bool TranslateX64 { get; set; }

        [Required]
        public bool CreateNMF { get; set; }

        [Required]
        public string NaClLinkerPath { get; set; }

        [Required]
        public string ProjectName { get; set; }

        [Required]
        public string ToolchainName { get; set; }

        [Required]
        public string CreateNMFPath { get; set; }

        [Required]
        public string ConfigurationType { get; set; }

        public NaClLink()
            : base(new ResourceManager("NaCl.Build.CPPTasks.Properties.Resources", Assembly.GetExecutingAssembly()))
        {
            this.EnvironmentVariables = new string[] { "CYGWIN=nodosfilewarning", "LC_CTYPE=C" };
        }

        protected override void LogEventsFromTextOutput(string singleLine, MessageImportance messageImportance)
        {
            base.LogEventsFromTextOutput(GCCUtilities.ConvertGCCOutput(singleLine), messageImportance);
        }

        protected override string GenerateResponseFileCommands()
        {
            StringBuilder responseFileCmds = new StringBuilder(GCCUtilities.s_CommandLineLength);

            // We want GCC to behave more like visual studio in term of library dependencies
            // so we wrap all the inputs (libraries and object) into one group so they are
            // searched iteratively.
            responseFileCmds.Append("-Wl,--start-group ");
            foreach (ITaskItem sourceFile in Sources)
            {
                responseFileCmds.Append(GCCUtilities.ConvertPathWindowsToPosix(sourceFile.GetMetadata("Identity")));
                responseFileCmds.Append(" ");
            }
            responseFileCmds.Append("-Wl,--end-group ");

            responseFileCmds.Append(xamlParser.Parse(Sources[0], false, IsPNaCl() ? ".bc" : null));

            return responseFileCmds.ToString();
        }

        private static string PexeToBC(string pexe)
        {
            return Path.ChangeExtension(pexe, ".bc");
        }

        private static string PexeToNexe(string pexe, string arch)
        {
            string basename = Path.GetFileNameWithoutExtension(pexe) + "_" + arch + ".nexe";
            return Path.Combine(Path.GetDirectoryName(pexe), basename);
        }

        private bool Finalize()
        {
            string dirname = Path.GetDirectoryName(GenerateFullPathToTool());
            string finalize = Path.Combine(dirname, "pnacl-finalize.bat");
            string cmd = String.Format("\"{0}\" -o \"{1}\"", PexeToBC(OutputFile), OutputFile);
            if (!OutputCommandLine)
                Log.LogMessage("pnacl-finalize -> {0}", Path.GetFileName(OutputFile));

            return ExecuteTool(finalize, cmd, string.Empty) == 0;
        }

        private bool Translate(string arch, string pnacl_arch=null)
        {
            if (pnacl_arch == null)
                pnacl_arch = arch;
            string outfile = PexeToNexe(OutputFile, arch);
            string cmd = String.Format("-arch {0} \"{1}\" -o \"{2}\"",
                                       pnacl_arch, OutputFile, outfile);

            string dirname = Path.GetDirectoryName(GenerateFullPathToTool());
            string translateTool = Path.Combine(dirname, "pnacl-translate.bat");
            if (!OutputCommandLine)
                Log.LogMessage("pnacl-translate -> {0}", Path.GetFileName(outfile));

            return ExecuteTool(translateTool, cmd, string.Empty) == 0;
        }

        public override bool Execute()
        {
            if (!OutputCommandLine) {
                string filename = OutputFile;
                if (IsPNaCl())
                   filename = PexeToBC(OutputFile);
                Log.LogMessage("Linking: {0}", filename);
            }

            if (!base.Execute())
                return false;

            if (!SkippedExecution)
                if (!PostLink())
                    return false;

            return true;
        }

        protected bool PostLink()
        {
            if (IsPNaCl())
            {
                if (!Finalize())
                    return false;

                if (TranslateX64 && !Translate("64", "x86-64"))
                    return false;

                if (TranslateX86 && !Translate("32", "i686"))
                    return false;

                if (TranslateARM && !Translate("arm"))
                    return false;
            }

            if (CreateNMF)
            {
                if (!SDKUtilities.FindPython())
                {
                    Log.LogError("Automatic NMF creation requires python in your executable path.");
                    return false;
                }

                string outputRoot = ToolchainName;
                if (IsPNaCl())
                    outputRoot = "PNaCl";

                if (!Directory.Exists(outputRoot))
                    Directory.CreateDirectory(outputRoot);

                string nmfPath = Path.Combine(outputRoot, Path.ChangeExtension(ProjectName, ".nmf"));

                string cmd = "\"" + CreateNMFPath + "\" -o \"" + nmfPath + "\"";

                // The SDK root is one level up from create_nmf.py
                // Starting with 25.170267 the -t options to create_nmf is deprecated.
                string sdkroot = Path.GetDirectoryName(Path.GetDirectoryName(CreateNMFPath));
                if (!SDKUtilities.CheckVersionAtLeast(sdkroot, 25, 170267))
                    cmd += " -t " + ToolchainName;

                cmd += " -s " + ToolchainName;

                if (IsPNaCl())
                {
                    if (!TranslateARM && !TranslateX64 && !TranslateX86)
                        // Don't run create_nmf unless we actaully produced a nexe file.
                        return true;

                    foreach (var arch in new string []{ "arm", "32", "64" })
                    {
                        string nexe = PexeToNexe(OutputFile, arch);
                        if (File.Exists(nexe))
                            cmd += " \"" + nexe + "\"";
                    }
                }
                else
                {
                    cmd += " \"" + OutputFile + "\"";
                }

                if (!OutputCommandLine)
                    Log.LogMessage("CreateNMF -> {0}", Path.GetFileName(nmfPath));

                if (ExecuteTool("python", string.Empty, cmd) != 0)
                    return false;
            }

            return true;
        }

        protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
        {
            if (OutputCommandLine)
                Log.LogMessage(MessageImportance.High, pathToTool + "  " + responseFileCommands + " " + commandLineCommands);

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
                return NaClLinkerPath;
            }
        }

        protected override string CommandTLogFilename
        {
            get
            {
                return BaseTool() + ".link.command.1.tlog";
            }
        }

        protected override string[] ReadTLogFilenames
        {
            get
            {
                return new string[] { BaseTool() + ".link.read.1.tlog" };
            }
        }

        protected override string WriteTLogFilename
        {
            get
            {
                return BaseTool() + ".link.write.1.tlog";
            }
        }
    }
}
