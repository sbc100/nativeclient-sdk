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
    public abstract class NaClToolTask : ToolTask
    {
        protected NaClToolTask(ResourceManager taskResources) : base(taskResources) { }
        public bool BuildingInIDE { get; set; }
        protected ITaskItem[] excludedInputPaths;
        private ITaskItem[] tlogReadFiles;
        private ITaskItem tlogCommandFile;
        private ITaskItem[] tlogWriteFiles;
        private CanonicalTrackedInputFiles trackedInputFiles;
        private bool skippedExecution;
        private bool trackFileAccess;
        protected ITaskItem[] compileSourceList;
        protected XamlParser xamlParser;

        [Required]
        public string PropertiesFile { get; set; }

        [Required]
        public string TrackerLogDirectory { get; set; }

        [Required]
        public virtual ITaskItem[] Sources { get; set; }

        [Required]
        public bool OutputCommandLine { get; set; }

        [Required]
        public bool MinimalRebuildFromTracking { get; set; }

        [Required]
        public string Platform { get; set; }

        public virtual string OutputFile { get; set; }

        // Override default StandardOutputLoggingImportance so that we see the stdout from the
        // toolchain from within visual studio.
        protected override MessageImportance StandardOutputLoggingImportance
        {
            get { return MessageImportance.Normal; }
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

        protected override bool SkipTaskExecution()
        {
            return this.skippedExecution;
        }

        protected string BaseTool()
        {
            return Path.GetFileNameWithoutExtension(ToolName);
        }

        protected bool IsPNaCl()
        {
            return Platform.Equals("pnacl", StringComparison.OrdinalIgnoreCase);
        }

        protected bool Setup()
        {
            SkippedExecution = false;
            if (!ValidateParameters())
            {
                return false;
            }

            if (IsPNaCl())
            {
                if (!SDKUtilities.FindPython())
                {
                    Log.LogError("PNaCl linking requires python in your executable path.");
                    return false;
                }
            }

            if (TrackFileAccess || MinimalRebuildFromTracking)
            {
                SetTrackerLogPaths();
            }

            CalcSourcesToBuild();
            return true;
        }

        protected virtual CanonicalTrackedOutputFiles OutputWriteTLog(ITaskItem[] inputs)
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

        protected virtual void OutputReadTLog(ITaskItem[] compiledSources,
                                              CanonicalTrackedOutputFiles outputs)
        {
            string trackerPath = Path.GetFullPath(TlogDirectory + ReadTLogFilenames[0]);

            using (var writer = new StreamWriter(trackerPath, false, Encoding.Unicode))
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

        protected virtual string TLogCommandForSource(ITaskItem source)
        {
            return GenerateResponseFileCommands();
        }

        protected virtual void OutputCommandTLog(ITaskItem[] compiledSources)
        {
            string fullpath = TLogCommandFile.GetMetadata("FullPath");
            using (var writer = new StreamWriter(fullpath, false, Encoding.Unicode))
            {
                string cmds = TLogCommandForSource(Sources[0]);
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

        public override bool Execute()
        {
            xamlParser = new XamlParser(PropertiesFile);
            if (!Setup())
                return false;

            if (SkippedExecution)
                return true;

            bool res = base.Execute();
            // Update tracker log files if execution was successful
            if (res)
            {
                CanonicalTrackedOutputFiles outputs = OutputWriteTLog(compileSourceList);
                OutputReadTLog(compileSourceList, outputs);
                OutputCommandTLog(compileSourceList);
            }

            return res;
        }

        protected override Encoding ResponseFileEncoding
        {
            get
            {
                return Encoding.ASCII;
            }
        }

        protected virtual void SetTrackerLogPaths()
        {
            if (TLogCommandFile == null)
            {
                string commandFile = Path.Combine(TlogDirectory, CommandTLogFilename);
                TLogCommandFile = new TaskItem(commandFile);
            }

            if (TLogReadFiles == null)
            {
                TLogReadFiles = new ITaskItem[ReadTLogFilenames.Length];
                for (int n = 0; n < ReadTLogFilenames.Length; n++)
                {
                    string readFile = Path.Combine(TlogDirectory, ReadTLogFilenames[n]);
                    TLogReadFiles[n] = new TaskItem(readFile);
                }
            }

            if (this.TLogWriteFiles == null)
            {
                TLogWriteFiles = new ITaskItem[1];
                string writeFile = Path.Combine(TlogDirectory, WriteTLogFilename);
                TLogWriteFiles[0] = new TaskItem(writeFile);
            }
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

        protected IDictionary<string, string> GenerateCommandLinesFromTlog()
        {
            IDictionary<string, string> cmdLineDictionary = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            string tlogFilename = this.TLogCommandFile.GetMetadata("FullPath");
            if (!File.Exists(tlogFilename))
                return cmdLineDictionary;

            using (StreamReader reader = File.OpenText(tlogFilename))
            {
                string[] filenames = null;
                for (string lineStr = reader.ReadLine(); lineStr != null; lineStr = reader.ReadLine())
                {
                    if (lineStr.Length == 0 || (lineStr[0] == '^' && lineStr.Length == 1))
                    {
                        Log.LogError("Invalid line in command tlog");
                        break;
                    }
                    else if (lineStr[0] == '^')
                    {
                        filenames = lineStr.Substring(1).Split('|');
                    }
                    else
                    {
                        foreach (string filename in filenames)
                        {
                            cmdLineDictionary[filename] = lineStr;
                        }
                    }
                }
            }
            return cmdLineDictionary;
        }

        protected List<ITaskItem> GetOutOfDateSourcesFromCmdLineChanges()
        {
            //get dictionary of source + command lines
            IDictionary<string, string> dictionary = GenerateCommandLinesFromTlog();
            List<ITaskItem> outOfDateSources = new List<ITaskItem>();

            //add sources to out of date list if the tlog dictionary string do not match the generated command line string
            StringBuilder currentCommandLine = new StringBuilder(GCCUtilities.s_CommandLineLength);
            foreach (ITaskItem sourceItem in Sources)
            {
                currentCommandLine.Length = 0;
                currentCommandLine.Append(TLogCommandForSource(sourceItem));

                string tlogCommandLine = null;
                if (dictionary.TryGetValue(FileTracker.FormatRootingMarker(sourceItem), out tlogCommandLine))
                {
                    if (tlogCommandLine == null || !currentCommandLine.ToString().Equals(tlogCommandLine, StringComparison.Ordinal))
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

        protected void CalcSourcesToBuild()
        {
            //check if full recompile is required otherwise perform incremental
            if (ForcedRebuildRequired() || MinimalRebuildFromTracking == false)
            {
                CompileSourceList = Sources;
                return;
            }

            //retrieve list of sources out of date due to command line changes
            List<ITaskItem> outOfDateSourcesFromCommandLine = GetOutOfDateSourcesFromCmdLineChanges();

            //retrieve sources out of date due to tracking
            CanonicalTrackedOutputFiles outputs = new CanonicalTrackedOutputFiles(this, TLogWriteFiles);
            TrackedInputFiles = new CanonicalTrackedInputFiles(this,
                                                               TLogReadFiles,
                                                               Sources,
                                                               ExcludedInputPaths,
                                                               outputs,
                                                               true,
                                                               false);
            ITaskItem[] outOfDateSourcesFromTracking = TrackedInputFiles.ComputeSourcesNeedingCompilation();

            //merge out of date lists
            CompileSourceList = MergeOutOfDateSources(outOfDateSourcesFromTracking, outOfDateSourcesFromCommandLine);
            if (CompileSourceList.Length == 0)
            {
                SkippedExecution = true;
                return;
            }

            //remove sources to compile from tracked file list
            TrackedInputFiles.RemoveEntriesForSource(CompileSourceList);
            outputs.RemoveEntriesForSource(CompileSourceList);
            TrackedInputFiles.SaveTlog();
            outputs.SaveTlog();
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

        protected abstract string CommandTLogFilename { get; }

        protected abstract string WriteTLogFilename { get; }

        protected abstract string[] ReadTLogFilenames { get; }

        public virtual string PlatformToolset
        {
            get
            {
                return "GCC";
            }
        }

        protected override string GenerateFullPathToTool()
        {
            return this.ToolName;
        }
    }
}
