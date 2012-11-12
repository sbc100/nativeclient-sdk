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
        private bool minimalRebuildFromTracking;
        private bool trackFileAccess;
        protected ITaskItem[] compileSourceList;
        protected XamlParser xamlParser;

        [Required]
        public string TrackerLogDirectory { get; set; }

        [Required]
        public virtual ITaskItem[] Sources { get; set; }

        [Required]
        public bool OutputCommandLine { get; set; }

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
            this.SkippedExecution = false;

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

            if (this.TrackFileAccess || this.MinimalRebuildFromTracking)
            {
                this.SetTrackerLogPaths();
            }

            if (this.ForcedRebuildRequired() || this.MinimalRebuildFromTracking == false)
            {
                if (this.Sources == null || this.Sources.Length == 0)
                {
                    this.SkippedExecution = true;
                }
            }

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

        protected virtual void OutputCommandTLog(ITaskItem[] compiledSources)
        {
            string fullpath = TLogCommandFile.GetMetadata("FullPath");
            using (var writer = new StreamWriter(fullpath, false, Encoding.Unicode))
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

        public override bool Execute()
        {
            bool  returnResult = base.Execute();

            // Update tracker log files if execution occurred
            //if (this.skippedExecution == false)
            {
                CanonicalTrackedOutputFiles outputs = OutputWriteTLog(compileSourceList);
                OutputReadTLog(compileSourceList, outputs);
                OutputCommandTLog(compileSourceList);
            }

            return returnResult;
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

        protected virtual string CommandTLogFilename
        {
            get
            {
                return BaseTool() + ".compile.command.1.tlog";
            }
        }

        protected virtual string[] ReadTLogFilenames
        {
            get
            {
                return new string[] { BaseTool() + ".compile.read.1.tlog" };
            }
        }

        protected virtual string WriteTLogFilename
        {
            get
            {
                return BaseTool() + ".compile.write.1.tlog";
            }
        }

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
