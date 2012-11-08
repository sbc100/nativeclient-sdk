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

        protected abstract CanonicalTrackedOutputFiles OutputWriteTLog(ITaskItem[] compiledSources);
        protected abstract void OutputReadTLog(ITaskItem[] compiledSources, CanonicalTrackedOutputFiles outputs);
        protected abstract void OutputCommandTLog(ITaskItem[] compiledSources);

        [Required]
        public string TrackerLogDirectory { get; set; }

        [Required]
        public virtual ITaskItem[] Sources { get; set; }


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

        protected bool Setup()
        {
            this.SkippedExecution = false;

            if (!ValidateParameters())
            {
                return false;
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
    }
}
