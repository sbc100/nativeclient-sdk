// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using System.IO;
using System.Reflection;
using System.Resources;
using System.Windows.Forms;
using Microsoft.Build.Framework;
using Microsoft.Win32;
using Microsoft.Build.Utilities;
using System.Collections.Specialized;

using System.Diagnostics;

namespace NaCl.Build.CPPTasks
{
    public class NaClCompile : NaClToolTask
    {
        public bool BuildingInIDE { get; set; }

        [Required]
        public string PropertiesFile { get; set; }

        [Required]
        public string NaCLCompilerPath { get; set; }

        [Required]
        public bool OutputCommandLine { get; set; }

        [Required]
        public string Platform { get; set; }

        public int ProcessorNumber { get; set; }

        public bool MultiProcessorCompilation { get; set; }

        [Required]
        public string ConfigurationType { get; set; }

        [Obsolete]
        protected override StringDictionary EnvironmentOverride
        {
            get {
                string show = OutputCommandLine ? "1" : "0";
                string cores = Convert.ToString(ProcessorNumber);
                return new StringDictionary() {
                      {"NACL_GCC_CORES", cores},
                      {"NACL_GCC_SHOW_COMMANDS", show }
                };
            }
        }

        protected override string GenerateFullPathToTool()
        {
            return ToolName;
        }

        public NaClCompile()
            : base(new ResourceManager("NaCl.Build.CPPTasks.Properties.Resources", Assembly.GetExecutingAssembly()))
        {
            this.EnvironmentVariables = new string[] { "CYGWIN=nodosfilewarning", "LC_CTYPE=C" };
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
                            Log.LogError("Invalid line in command tlog");
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
            base.LogEventsFromTextOutput(GCCUtilities.ConvertGCCOutput(singleLine), messageImportance);
        }

        static string GetObjectFile(ITaskItem source)
        {
            string objectFilePath = Path.GetFullPath(source.GetMetadata("ObjectFileName"));
            // cl.exe will accept a folder name as the ObjectFileName in which case
            // the objectfile is created as <ObjectFileName>/<basename>.obj.  Here
            // we mimic this behaviour.
            if ((File.GetAttributes(objectFilePath) & FileAttributes.Directory) != 0)
            {
                objectFilePath = Path.Combine(objectFilePath, Path.GetFileName(source.ItemSpec));
                objectFilePath = Path.ChangeExtension(objectFilePath, ".obj");
            }
            return objectFilePath;
        }

        protected override void OutputReadTLog(ITaskItem[] compiledSources, CanonicalTrackedOutputFiles outputs)
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
                    string objectFilePath = GetObjectFile(source);
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

        protected override CanonicalTrackedOutputFiles OutputWriteTLog(ITaskItem[] compiledSources)
        {
            string path = Path.Combine(TlogDirectory, WriteTLogFilename);
            TaskItem item = new TaskItem(path);
            CanonicalTrackedOutputFiles trackedFiles = new CanonicalTrackedOutputFiles(new TaskItem[] { item });

            foreach (ITaskItem sourceItem in compiledSources)
            {
                //remove this entry associated with compiled source which is about to be recomputed
                trackedFiles.RemoveEntriesForSource(sourceItem);

                //add entry with updated information
                trackedFiles.AddComputedOutputForSourceRoot(Path.GetFullPath(sourceItem.ItemSpec).ToUpperInvariant(),
                                                            Path.GetFullPath(GetObjectFile(sourceItem)).ToUpperInvariant());
            }

            //output tlog
            trackedFiles.SaveTlog();

            return trackedFiles;
        }

        protected override void OutputCommandTLog(ITaskItem[] compiledSources)
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

        protected string GenerateCommandLineFromProps(ITaskItem sourceFile,
                                                      bool fullOutputName=false)
        {
            StringBuilder commandLine = new StringBuilder(GCCUtilities.s_CommandLineLength);

            if (sourceFile != null)
            {
                //build command line from components and add required switches
                string props = xamlParser.Parse(sourceFile, fullOutputName);
                commandLine.Append(props);
                commandLine.Append(" -c ");

                // Remove rtti items as they are not relevant in C compilation and will 
                // produce warnings
                if (SourceIsC(sourceFile.ToString()))
                {
                    commandLine.Replace("-fno-rtti", "");
                    commandLine.Replace("-frtti", "");
                }

                if (ConfigurationType == "DynamicLibrary")
                {
                    commandLine.Append(" -fPIC ");
                }
            }

            return commandLine.ToString();
        }

        protected ITaskItem[] MergeOutOfDateSources(ITaskItem[] outOfDateSourcesFromTracking,
                List<ITaskItem> outOfDateSourcesFromCommandLineChanges)
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

        private int Compile(string pathToTool)
        {
            if (Platform.Equals("pnacl", StringComparison.OrdinalIgnoreCase))
            {
                if (!SDKUtilities.FindPython())
                {
                    Log.LogError("PNaCl compilation requires python in your executable path.");
                    return -1;
                }

                // The SDK root is five levels up from the compiler binary.
                string sdkroot = Path.GetDirectoryName(Path.GetDirectoryName(pathToTool));
                sdkroot = Path.GetDirectoryName(Path.GetDirectoryName(sdkroot));
                sdkroot = Path.GetDirectoryName(sdkroot);

                int revision;
                int version = SDKUtilities.GetSDKVersion(sdkroot, out revision);
                if (revision < SDKUtilities.MinPNaCLSDKVersion)
                {
                    Log.LogError("The configured version of the NaCl SDK ({0} r{1}) is too old " +
                                 "for building PNaCl projects.\n" +
                                 "Please install at least 24 r{2} using the naclsdk command " +
                                 "line tool.", version, revision, SDKUtilities.MinPNaCLSDKVersion);
                    return -1;
                }
            }

            // If multiprocess compilation is enabled (not the VS default)
            // and the number of processors to use is not 1, then use the
            // compiler_wrapper python script to run multiple instances of
            // gcc
            if (MultiProcessorCompilation && ProcessorNumber != 1)
            {
                if (!SDKUtilities.FindPython())
                {
                    Log.LogWarning("Multi-processor Compilation with NaCl requires that python " +
                                   "be available in the visual studio executable path.\n" +
                                   "Please disable Multi-processor Compilation in the project " +
                                   "properties or add python to the your executable path.\n" +
                                   "Falling back to serial compilation.\n");
                }
                else
                {
                    return CompileParallel(pathToTool);
                }
            }
            return CompileSerial(pathToTool);
        }

        private int CompileSerial(string pathToTool)
        {
            int returnCode = 0;
            foreach (ITaskItem sourceItem in CompileSourceList)
            {
                try
                {
                    string commandLine = GenerateCommandLineFromProps(sourceItem, true);
                    commandLine += GCCUtilities.ConvertPathWindowsToPosix(sourceItem.ToString());

                    if (OutputCommandLine)
                    {
                        string logMessage = pathToTool + " " + commandLine;
                        Log.LogMessage(logMessage);
                    }
                    else
                    {
                        base.Log.LogMessage(Path.GetFileName(sourceItem.ToString()));
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

        private int CompileParallel(string pathToTool)
        {
            int returnCode = 0;

            // Compute sources that can be compiled together.
            Dictionary<string, List<ITaskItem>> srcGroups =
                    new Dictionary<string, List<ITaskItem>>();

            foreach (ITaskItem sourceItem in CompileSourceList)
            {
                string commandLine = GenerateCommandLineFromProps(sourceItem);
                if (srcGroups.ContainsKey(commandLine))
                {
                    srcGroups[commandLine].Add(sourceItem);
                }
                else
                {
                    srcGroups.Add(commandLine, new List<ITaskItem> {sourceItem});
                }
            }

            string pythonScript = Path.GetDirectoryName(Path.GetDirectoryName(PropertiesFile));
            pythonScript = Path.Combine(pythonScript, "compiler_wrapper.py");

            foreach (KeyValuePair<string, List<ITaskItem>> entry in srcGroups)
            {
                string commandLine = entry.Key;
                string cmd = "\"" + pathToTool + "\" " + commandLine + "--";
                List<ITaskItem> sources = entry.Value;

                foreach (ITaskItem sourceItem in sources)
                {
                    cmd += GCCUtilities.ConvertPathWindowsToPosix(sourceItem.ToString());
                }

                try
                {
                    // compile this group of sources
                    returnCode = base.ExecuteTool("python", cmd, "\"" + pythonScript + "\"");
                }
                catch (Exception e)
                {
                    Log.LogMessage("compiler exception: {0}", e);
                    returnCode = base.ExitCode;
                }

                //abort if an error was encountered
                if (returnCode != 0)
                    break;
            }

            Log.LogMessage(MessageImportance.Low, "compiler returned: {0}", returnCode);
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

        protected void CalcSourcesToCompile()
        {
            //check if full recompile is required otherwise perform incremental
            if (this.ForcedRebuildRequired() || this.MinimalRebuildFromTracking == false)
            {
                this.CompileSourceList = this.Sources;
                return;
            }

            //retrieve list of sources out of date due to command line changes
            List<ITaskItem> outOfDateSourcesFromCommandLine = GetOutOfDateSourcesFromCmdLineChanges();

            //retrieve sources out of date due to tracking
            CanonicalTrackedOutputFiles trackedOutputFiles = new CanonicalTrackedOutputFiles(this, this.TLogWriteFiles);
            this.TrackedInputFiles = new CanonicalTrackedInputFiles(this,
                                                                    this.TLogReadFiles,
                                                                    this.Sources,
                                                                    this.ExcludedInputPaths,
                                                                    trackedOutputFiles,
                                                                    true,
                                                                    false);
            ITaskItem[] outOfDateSourcesFromTracking = this.TrackedInputFiles.ComputeSourcesNeedingCompilation();

            //merge out of date lists
            CompileSourceList = MergeOutOfDateSources(outOfDateSourcesFromTracking, outOfDateSourcesFromCommandLine);

            if (this.CompileSourceList.Length == 0)
            {
                this.SkippedExecution = true;
                return;
            }

            //remove sources to compile from tracked file list
            this.TrackedInputFiles.RemoveEntriesForSource(this.CompileSourceList);
            trackedOutputFiles.RemoveEntriesForSource(this.CompileSourceList);
            this.TrackedInputFiles.SaveTlog();
            trackedOutputFiles.SaveTlog();
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
                xamlParser = new XamlParser(PropertiesFile);
                if (!Setup())
                    return false;
                CalcSourcesToCompile();
                returnResult = base.Execute();
            }
            finally
            {

            }

            return returnResult;
        }

        protected List<ITaskItem> GetOutOfDateSourcesFromCmdLineChanges()
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

    }
}
