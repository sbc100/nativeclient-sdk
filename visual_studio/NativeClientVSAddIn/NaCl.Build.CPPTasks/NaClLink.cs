
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
        public bool BuildingInIDE { get; set; }

        /// <summary>
        /// Property set only in PNaCl builds to signal that the translator
        /// should be run post-link.
        /// </summary>
        public string TranslateARM { get; set; }

        /// <summary>
        /// Property set only in PNaCl builds to signal that the translator
        /// should be run post-link.
        /// </summary>
        public string TranslateX86 { get; set; }

        /// <summary>
        /// Property set only in PNaCl builds to signal that the translator
        /// should be run post-link.
        /// </summary>
        public string TranslateX64 { get; set; }

        [Required]
        public string OutputCommandLine { get; set; }

        [Required]
        public string NaClLinkerPath { get; set; }

        [Required]
        public string Platform { get; set; }

        [Required]
        public virtual string OutputFile { get; set; }

        [Required]
        public string PropertiesFile { get; set; }

        [Required]
        public string ConfigurationType { get; set; }

        protected override CanonicalTrackedOutputFiles OutputWriteTLog(ITaskItem[] inputs)
        {
            string path = Path.Combine(TlogDirectory, WriteTLogFilename);
            TaskItem item = new TaskItem(path);
            CanonicalTrackedOutputFiles trackedFiles =
                new CanonicalTrackedOutputFiles(new TaskItem[] { item });

            foreach (ITaskItem sourceItem in Sources)
            {
                //remove this entry associated with compiled source which is about to be recomputed
                trackedFiles.RemoveEntriesForSource(sourceItem);

                //add entry with updated information
                string upper = Path.GetFullPath(sourceItem.ItemSpec).ToUpperInvariant();
                trackedFiles.AddComputedOutputForSourceRoot(upper, OutputFile);
            }

            //output tlog
            trackedFiles.SaveTlog();

            return trackedFiles;
        }

        protected override void OutputReadTLog(ITaskItem[] compiledSources, CanonicalTrackedOutputFiles outputs)
        {
            string trackerPath = Path.GetFullPath(TlogDirectory + ReadTLogFilenames[0]);

            using (StreamWriter writer = new StreamWriter(trackerPath, false, Encoding.Unicode))
            {
                string sourcePath = "";
                foreach (ITaskItem source in Sources)
                {
                    if (sourcePath != "")
                        sourcePath += "|";
                    sourcePath += Path.GetFullPath(source.ItemSpec).ToUpperInvariant();
                }

                writer.WriteLine("^" + sourcePath);
                foreach (ITaskItem source in Sources)
                {
                    writer.WriteLine(Path.GetFullPath(source.ItemSpec).ToUpperInvariant());
                }
                writer.WriteLine(Path.GetFullPath(OutputFile).ToUpperInvariant());
            }
        }

        protected override void OutputCommandTLog(ITaskItem[] compiledSources)
        {
            using (StreamWriter writer = new StreamWriter(TLogCommandFile.GetMetadata("FullPath"), false, Encoding.Unicode))
            {
                string cmds = GenerateResponseFileCommands();
                string sourcePath = "";
                foreach (ITaskItem source in Sources)
                {
                    if (sourcePath != "")
                        sourcePath += "|";
                    sourcePath += Path.GetFullPath(source.ItemSpec).ToUpperInvariant();
                }

                writer.WriteLine("^" + sourcePath);
                writer.WriteLine(cmds);
            }
        }

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

            responseFileCmds.Append(xamlParser.Parse(Sources[0], false));

            return responseFileCmds.ToString();
        }

        private bool Translate(string arch)
        {
            string nexeBase = Path.GetFileNameWithoutExtension(OutputFile) + "_" + arch + ".nexe";
            string outfile = Path.Combine(Path.GetDirectoryName(OutputFile), nexeBase);

            string commandLineCommands = String.Format("-arch {0} \"{1}\" -o \"{2}\"", arch, OutputFile, outfile);

            string translateTool = Path.Combine(Path.GetDirectoryName(GenerateFullPathToTool()), "pnacl-translate.bat");
            if (OutputCommandLine != "true")
                Log.LogMessage("pnacl-translate {0}", Path.GetFileName(nexeBase));

            if (ExecuteTool(translateTool, commandLineCommands, string.Empty) != 0)
            {
                return false;
            }

            return true;
        }

        public override bool Execute()
        {
            if (Platform.Equals("pnacl", StringComparison.OrdinalIgnoreCase))
            {
                if (!GCCUtilities.FindPython())
                {
                    Log.LogError("PNaCl linking requires python in your executable path.");
                    return false;
                }
            }

            bool returnResult = false;

            try
            {
                xamlParser = new XamlParser(PropertiesFile);
                if (!Setup())
                    return false;
                returnResult = base.Execute();

                if (TranslateX64 == "true" && !Translate("x86_64"))
                    return false;

                if (TranslateX86 == "true" && !Translate("i686"))
                    return false;

                if (TranslateARM == "true" && !Translate("arm"))
                    return false;
            }
            finally
            {

            }

            return returnResult;
        }

        protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
        {
            if (OutputCommandLine == "true")
            {
                Log.LogMessage(MessageImportance.High, pathToTool + "  " + responseFileCommands);
            }

            return base.ExecuteTool(pathToTool, responseFileCommands, commandLineCommands);
        }

        protected override string GenerateFullPathToTool()
        {
            return this.ToolName;
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

        public virtual string PlatformToolset
        {
            get
            {
                return "GCC";
            }
            set
            {}
        }
    }
}
