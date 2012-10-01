
using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using System.IO;
using System.Reflection;
using System.Resources;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

using System.Diagnostics;

namespace NaCl.Build.CPPTasks
{
    public class NaClCompile : ToolTask
    {
        private XamlParser m_XamlParser;
        private ITaskItem[] excludedInputPaths;
        private ITaskItem[] tlogReadFiles;
        private ITaskItem tlogCommandFile;
        private ITaskItem[] tlogWriteFiles;
        private CanonicalTrackedInputFiles trackedInputFiles;
        private bool skippedExecution;
        private ITaskItem[] compileSourceList;
        public bool BuildingInIDE { get; set; }
        private string m_toolname;
        private bool trackFileAccess;
        private bool minimalRebuildFromTracking;
        private string pathToLog;

        [Required]
        public string PropertiesFile { get; set; }

        [Required]
        public ITaskItem[] Sources { get; set; }

        [Required]
        public string NaCLCompilerPath { get; set; }

        [Required]
        public string OutputCommandLine { get; set; }

        [Required]
        public string TrackerLogDirectory { get; set; }


        protected override string GenerateFullPathToTool() { return ToolName; }

        public NaClCompile()
            : base(new ResourceManager("NaCl.Build.CPPTasks.Properties.Resources", Assembly.GetExecutingAssembly()))
        {
            this.pathToLog = string.Empty;
            this.EnvironmentVariables = new string []{"CYGWIN=nodosfilewarning", "LC_CTYPE=C"};
        }

        protected IDictionary<string, string> GenerateCommandLinesFromTlog()
        {
            IDictionary<string, string> cmdLineDictionary = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            string tlogFilename = this.TLogCommandFile.GetMetadata("FullPath");
            if (File.Exists(tlogFilename))
            {
                using (StreamReader reader = File.OpenText(tlogFilename))
                {
                    string filename = string.Empty;
                    for (string lineStr = reader.ReadLine(); lineStr != null; lineStr = reader.ReadLine())
                    {
                        if (lineStr.Length == 0 ||
                            (lineStr[0] == '^' && lineStr.Length == 1))
                        {
                            Log.LogMessage(MessageImportance.High, "Invalid line in command tlog");
                            break;
                        }
                        else if (lineStr[0] == '^')
                        {
                            filename = lineStr.Substring(1);
                        }
                        else
                        {
                            cmdLineDictionary[filename] = lineStr;
                        }
                    }
                }
            }
            return cmdLineDictionary;
        }

        protected override void LogEventsFromTextOutput(string singleLine, MessageImportance messageImportance)
        {
            base.LogEventsFromTextOutput(GCCUtilities.Convert_Output_GCC_to_VS(singleLine), messageImportance);
        }

        private void ConstructReadTLog(ITaskItem[] compiledSources, CanonicalTrackedOutputFiles outputs)
        {
            string trackerPath = Path.GetFullPath(TlogDirectory + ReadTLogFilenames[0]);

            //save tlog for sources not compiled during this execution
            TaskItem readTrackerItem = new TaskItem(trackerPath);
            CanonicalTrackedInputFiles files = new CanonicalTrackedInputFiles(new TaskItem[] { readTrackerItem }, Sources, outputs, false, false);
            files.RemoveEntriesForSource(compiledSources);
            files.SaveTlog();

            //add tlog information for compiled sources
            using (StreamWriter writer = new StreamWriter(trackerPath, true, Encoding.Unicode))
            {
                foreach (ITaskItem source in compiledSources)
                {
                    string sourcePath = Path.GetFullPath(source.ItemSpec).ToUpperInvariant();

                    string objectFilePath = Path.GetFullPath(source.GetMetadata("ObjectFileName"));
                    string depFilePath = Path.ChangeExtension(objectFilePath, ".d");

                    try
                    {
                        if (File.Exists(depFilePath) == false)
                        {
                            Log.LogMessage(MessageImportance.High, depFilePath + " not found");
                        }
                        else
                        {
                            writer.WriteLine("^" + sourcePath);
                            DependencyParser parser = new DependencyParser(depFilePath);

                            foreach (string filename in parser.Dependencies)
                            {
                                //source itself not required
                                if (filename == sourcePath)
                                    continue;

                                if (File.Exists(filename) == false)
                                {
                                    Log.LogMessage(MessageImportance.High, "File " + sourcePath + " is missing dependency " + filename);
                                }

                                writer.WriteLine(filename);
                            }

                            //remove d file
                            try
                            {
                                File.Delete(depFilePath);
                            }
                            finally
                            {

                            }
                        }

                    }
                    catch (Exception)
                    {
                        Log.LogError("Failed to update " + readTrackerItem + " for " + sourcePath);
                    }
                }
            }
        }

        private CanonicalTrackedOutputFiles OutputWriteTrackerLog(ITaskItem[] compiledSources)
        {
            string path = Path.Combine(TlogDirectory, WriteTLogFilename);
            TaskItem item = new TaskItem(path);
            CanonicalTrackedOutputFiles trackedFiles = new CanonicalTrackedOutputFiles(new TaskItem[] { item });

            foreach (ITaskItem sourceItem in compiledSources)
            {
                //remove this entry associated with compiled source which is about to be recomputed
                trackedFiles.RemoveEntriesForSource(sourceItem);

                //add entry with updated information
                trackedFiles.AddComputedOutputForSourceRoot( Path.GetFullPath(sourceItem.ItemSpec).ToUpperInvariant(),
                                                             Path.GetFullPath(sourceItem.GetMetadata("ObjectFileName")).ToUpperInvariant());
            }

            //output tlog
            trackedFiles.SaveTlog();

            return trackedFiles;
        }

        private void OutputCommandTrackerLog(ITaskItem[] compiledSources)
        {
            IDictionary<string, string> commandLines = GenerateCommandLinesFromTlog();

            //
            if (compiledSources != null)
            {
                foreach (ITaskItem source in compiledSources)
                {
                    string rmSource = FileTracker.FormatRootingMarker(source);
                    commandLines[rmSource] = GenerateCommandLineFromProps(source) + " " + source.GetMetadata("FullPath").ToUpperInvariant();
                }
            }

            //write tlog
            using (StreamWriter writer = new StreamWriter(this.TLogCommandFile.GetMetadata("FullPath"), false, Encoding.Unicode))
            {
                foreach (KeyValuePair<string, string> p in commandLines)
                {
                    string keyLine = "^" + p.Key;
                    writer.WriteLine(keyLine);
                    writer.WriteLine(p.Value);
                }
            }
        }

        protected string GenerateCommandLineFromProps(ITaskItem sourceFile)
        {
            StringBuilder commandLine = new StringBuilder(GCCUtilities.s_CommandLineLength);

            if (sourceFile != null)
            {
                string sourcePath = GCCUtilities.Convert_Path_Windows_To_Posix(sourceFile.ToString());

                // Remove rtti items as they are not relevant in C compilation and will produce warnings
                if (SourceIsC(sourceFile.ToString()))
                {
                    commandLine.Replace("-fno-rtti", "");
                    commandLine.Replace("-frtti", "");
                }

                //build command line from components and add required switches
                string props = m_XamlParser.Parse(sourceFile);
                commandLine.Append(props);
                commandLine.Append(" -MD -c ");
                commandLine.Append("\"" + sourcePath "\"");
            }

            return commandLine.ToString();
        }

        protected ITaskItem[] MergeOutOfDateSources(ITaskItem[] outOfDateSourcesFromTracking, List<ITaskItem> outOfDateSourcesFromCommandLineChanges)
        {
            List<ITaskItem> mergedSources = new List<ITaskItem>(outOfDateSourcesFromTracking);

            foreach (ITaskItem item in outOfDateSourcesFromCommandLineChanges)
            {
                if (!mergedSources.Contains(item))
                {
                    mergedSources.Add(item);
                }
            }

            return mergedSources.ToArray();
        }

        protected bool ForcedRebuildRequired()
        {
            string tlogCommandPath = null;

            try
            {
                tlogCommandPath = this.TLogCommandFile.GetMetadata("FullPath");
            }
            catch (Exception exception)
            {
                if (exception is InvalidOperationException || exception is NullReferenceException)
                    return true;
                else
                    throw;
            }

            //if command tlog file does not exist then force rebuild is required
            if (File.Exists(tlogCommandPath) == false)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        private int Compile(string pathToTool)
        {
            int returnCode = 0;

            foreach (ITaskItem sourceFileItem in CompileSourceList)
            {
                try
                {
                    string commandLine = GenerateCommandLineFromProps(sourceFileItem);

                    base.Log.LogMessageFromText(Path.GetFileName(sourceFileItem.ToString()), MessageImportance.High);

                    if (OutputCommandLine == "true")
                    {
                        string logMessage = pathToTool + " " + commandLine;
                        Log.LogMessageFromText(logMessage, MessageImportance.High);
                    }


                    // compile
                    returnCode = base.ExecuteTool(pathToTool, commandLine, string.Empty);
                }
                catch (Exception)
                {
                    returnCode = base.ExitCode;
                }

                //abort if an error was encountered
                if (returnCode != 0)
                {
                    return returnCode;
                }
            }
            return returnCode;
        }

        protected override int ExecuteTool(string pathToTool, string responseFileCommands, string commandLineCommands)
        {
            if (File.Exists(pathToTool) == false)
            {
                base.Log.LogMessageFromText("Unable to find NaCL compiler: " + pathToTool, MessageImportance.High);
                return -1;
            }

            int returnCode = -1;

            try
            {
                returnCode = Compile(pathToTool);
            }
            finally
            {

            }
            return returnCode;
        }

        protected override bool SkipTaskExecution()
        {
            return this.skippedExecution;
        }

        protected void CalcSourcesToCompile()
        {
            if (this.TrackFileAccess || this.MinimalRebuildFromTracking)
            {
                this.SetTrackerLogPaths();
            }

            //check if full recompile is required otherwise perform incremental
            if (this.ForcedRebuildRequired() || this.MinimalRebuildFromTracking == false)
            {
                this.CompileSourceList = this.Sources;
                if ((this.CompileSourceList == null) || (this.CompileSourceList.Length == 0))
                {
                    this.SkippedExecution = true;
                }
            }
            else
            {
                //retrieve list of sources out of date due to command line changes
                List<ITaskItem> outOfDateSourcesFromCommandLineChanges = this.GetOutOfDateSourcesFromCommandLineChanges();

                //retrieve sources out of date due to tracking
                CanonicalTrackedOutputFiles trackedOutputFiles = new CanonicalTrackedOutputFiles(this, this.TLogWriteFiles);
                this.TrackedInputFiles = new CanonicalTrackedInputFiles(this, this.TLogReadFiles, this.Sources, this.ExcludedInputPaths, trackedOutputFiles, true, false);
                ITaskItem[] outOfDateSourcesFromTracking = this.TrackedInputFiles.ComputeSourcesNeedingCompilation();

                //merge out of date lists
                this.CompileSourceList = this.MergeOutOfDateSources(outOfDateSourcesFromTracking, outOfDateSourcesFromCommandLineChanges);

                if (this.CompileSourceList.Length == 0)
                {
                    this.SkippedExecution = true;
                }
                else
                {
                    //remove sources to compile from tracked file list
                    this.TrackedInputFiles.RemoveEntriesForSource(this.CompileSourceList);
                    trackedOutputFiles.RemoveEntriesForSource(this.CompileSourceList);
                    this.TrackedInputFiles.SaveTlog();
                    trackedOutputFiles.SaveTlog();

                    this.SkippedExecution = false;
                }
            }
        }

        protected bool SourceIsC(string sourceFilename)
        {
            string fileExt = Path.GetExtension(sourceFilename.ToString());

            if (fileExt == ".c")
                return true;
            else
                return false;
        }

        public override bool Execute()
        {
            bool returnResult = false;

            try
            {
                m_XamlParser = new XamlParser(PropertiesFile);
                m_toolname = Path.GetFileNameWithoutExtension(ToolName);
                ValidateParameters();
                CalcSourcesToCompile();

                returnResult = base.Execute();

                // Update tracker log files if execution occurred
                //if (this.skippedExecution == false)
                {
                    CanonicalTrackedOutputFiles outputs = OutputWriteTrackerLog(CompileSourceList);
                    ConstructReadTLog(CompileSourceList, outputs);
                    OutputCommandTrackerLog(CompileSourceList);
                }
            }
            finally
            {

            }

            return returnResult;
        }

        protected List<ITaskItem> GetOutOfDateSourcesFromCommandLineChanges()
        {
            //get dictionary of source + command lines
            IDictionary<string, string> dictionary = this.GenerateCommandLinesFromTlog();
            List<ITaskItem> outOfDateSources = new List<ITaskItem>();

            //add sources to out of date list if the tlog dictionary string do not match the generated command line string
            StringBuilder currentCommandLine = new StringBuilder(GCCUtilities.s_CommandLineLength);
            foreach (ITaskItem sourceItem in Sources)
            {
                currentCommandLine.Length = 0;

                currentCommandLine.Append(GenerateCommandLineFromProps(sourceItem));
                currentCommandLine.Append(" ");
                currentCommandLine.Append(sourceItem.GetMetadata("FullPath").ToUpperInvariant());

                string tlogCommandLine = null;
                if (dictionary.TryGetValue(FileTracker.FormatRootingMarker(sourceItem), out tlogCommandLine))
                {
                    if ((tlogCommandLine == null) || !currentCommandLine.ToString().Equals(tlogCommandLine, StringComparison.Ordinal))
                    {
                        outOfDateSources.Add(sourceItem);
                    }
                }
                else
                {
                    outOfDateSources.Add(sourceItem);
                }
            }
            return outOfDateSources;
        }

        protected virtual void SetTrackerLogPaths()
        {
            if (this.TLogCommandFile == null)
            {
                string commandFile = Path.Combine(this.TlogDirectory, this.CommandTLogFilename);
                this.TLogCommandFile = new TaskItem(commandFile);
            }

            if (this.TLogReadFiles == null)
            {
                this.TLogReadFiles = new ITaskItem[this.ReadTLogFilenames.Length];
                for (int n = 0; n < this.ReadTLogFilenames.Length; n++)
                {
                    string readFile = Path.Combine(this.TlogDirectory, this.ReadTLogFilenames[n]);
                    this.TLogReadFiles[n] = new TaskItem(readFile);
                }
            }

            if (this.TLogWriteFiles == null)
            {
                this.TLogWriteFiles = new ITaskItem[1];
                string writeFile = Path.Combine(this.TlogDirectory, this.WriteTLogFilename);
                this.TLogWriteFiles[0] = new TaskItem(writeFile);
            }
        }


        //props
        protected string CommandTLogFilename
        {
            get
            {
                return m_toolname + ".compile.command.1.tlog";
            }
        }

        protected string[] ReadTLogFilenames
        {
            get
            {
                return new string[] { m_toolname + ".compile.read.1.tlog" };
            }
        }

        [Output]
        public bool SkippedExecution
        {
            get
            {
                return this.skippedExecution;
            }
            set
            {
                this.skippedExecution = value;
            }
        }

        public ITaskItem TLogCommandFile
        {
            get
            {
                return this.tlogCommandFile;
            }
            set
            {
                this.tlogCommandFile = value;
            }
        }

        protected string TlogDirectory
        {
            get
            {
                if (this.TrackerLogDirectory != null)
                {
                    return this.TrackerLogDirectory;
                }
                return string.Empty;
            }
        }

        public bool MinimalRebuildFromTracking
        {
            get
            {
                return this.minimalRebuildFromTracking;
            }
            set
            {
                this.minimalRebuildFromTracking = value;
            }
        }


        public ITaskItem[] TLogReadFiles
        {
            get
            {
                return this.tlogReadFiles;
            }
            set
            {
                this.tlogReadFiles = value;
            }
        }

        public ITaskItem[] ExcludedInputPaths
        {
            get
            {
                return this.excludedInputPaths;
            }
            set
            {
                this.excludedInputPaths = value;
            }
        }


        public ITaskItem[] TLogWriteFiles
        {
            get
            {
                return this.tlogWriteFiles;
            }
            set
            {
                this.tlogWriteFiles = value;
            }
        }

        protected string WriteTLogFilename
        {
            get
            {
                return m_toolname + ".compile.write.1.tlog";
            }
        }

        public bool TrackFileAccess
        {
            get
            {
                return this.trackFileAccess;
            }
            set
            {
                this.trackFileAccess = value;
            }
        }

        protected CanonicalTrackedInputFiles TrackedInputFiles
        {
            get
            {
                return this.trackedInputFiles;
            }
            set
            {
                this.trackedInputFiles = value;
            }
        }

        [Output]
        public ITaskItem[] CompileSourceList
        {
            get
            {
                return this.compileSourceList;
            }
            set
            {
                this.compileSourceList = value;
            }
        }

        protected override string ToolName
        {
            get
            {
                return NaCLCompilerPath;
            }
        }

        protected override Encoding ResponseFileEncoding
        {
            get
            {
                return Encoding.ASCII;
            }
        }
    }
}
