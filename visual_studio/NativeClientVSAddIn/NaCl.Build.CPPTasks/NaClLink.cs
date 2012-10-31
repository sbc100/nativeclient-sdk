
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
        public bool OutputCommandLine { get; set; }

        [Required]
        public bool CreateNMF { get; set; }

        [Required]
        public string NaClLinkerPath { get; set; }

        [Required]
        public string ProjectName { get; set; }

        [Required]
        public string ToolchainName { get; set; }

        [Required]
        public string Platform { get; set; }

        [Required]
        public string CreateNMFPath { get; set; }

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

        private static string PexeToNexe(string pexe, string arch)
        {
            string basename = Path.GetFileNameWithoutExtension(pexe) + "_" + arch + ".nexe";
            return Path.Combine(Path.GetDirectoryName(pexe), basename);
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
                Log.LogMessage("pnacl-translate {0}", Path.GetFileName(outfile));

            if (ExecuteTool(translateTool, cmd, string.Empty) != 0)
            {
                return false;
            }

            return true;
        }

        private bool IsPNaCl()
        {
            return Platform.Equals("pnacl", StringComparison.OrdinalIgnoreCase);
        }

        public override bool Execute()
        {
            if (IsPNaCl())
            {
                if (!SDKUtilities.FindPython())
                {
                    Log.LogError("PNaCl linking requires python in your executable path.");
                    return false;
                }
            }

            xamlParser = new XamlParser(PropertiesFile);
            if (!Setup())
                return false;

            if (!OutputCommandLine)
                Log.LogMessage("Linking: {0}", Path.GetFileName(OutputFile));

            if (!base.Execute())
                return false;

            if (!PostLink())
                return false;

            return true;
        }

        protected bool PostLink()
        {
            if (IsPNaCl())
            {
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
                cmd += " -t " + ToolchainName + " -s " + ToolchainName;

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
                    if (ToolchainName == "glibc")
                    {
                        string bindir = Path.GetDirectoryName(NaClLinkerPath);
                        string tcroot = Path.GetDirectoryName(bindir);
                        cmd += " -D \"" + Path.Combine(bindir, "x86_64-nacl-objdump.exe") + "\"";
                        cmd += " -L \"" + Path.Combine(tcroot, "x86_64-nacl", "lib") + "\"";
                        cmd += " -L \"" + Path.Combine(tcroot, "x86_64-nacl", "lib32") + "\"";
                    }
                    cmd += " \"" + OutputFile + "\"";
                }

                if (!OutputCommandLine)
                    Log.LogMessage("CreateNMF");

                if (ExecuteTool("python", string.Empty, cmd) != 0)
                {
                    return false;
                }
            }

            return true;
        }

        protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
        {
            if (OutputCommandLine)
            {
                Log.LogMessage(MessageImportance.High, pathToTool + "  " + responseFileCommands + " " + commandLineCommands);
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
