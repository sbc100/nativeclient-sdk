
using System;
using System.IO;
using System.Resources;
using System.Reflection;
using System.Text;
using Microsoft.Build.Framework;
using Microsoft.Build.CPPTasks;


namespace NaCl.Build.CPPTasks
{
    public class NaClLink : TrackedVCToolTask
	{
        private XamlParser m_XamlParser;

        public bool BuildingInIDE { get; set; }

        [Required]
        public string OutputCommandLine { get; set; }

        [Required]
        public string NaClLinkerPath { get; set; }

        [Required]
        public virtual string OutputFile { get; set; }

        [Required]
        public string PropertiesFile { get; set; }

        [Required]
        public virtual ITaskItem[] Sources { get; set; }

        [Required]
        public string ConfigurationType { get; set; }

        public NaClLink()
            : base(new ResourceManager("NaCl.Build.CPPTasks.Properties.Resources", Assembly.GetExecutingAssembly()))
        {
          this.EnvironmentVariables = new string[] { "CYGWIN=nodosfilewarning", "LC_CTYPE=C" };
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
                return new string[] { 
                    "default.link.read.tlog"
                };
            }
        }

        protected override string[] WriteTLogNames
        {
            get
            {
                return new string[] { 
                    "default.link.write.tlog"
                };
            }
        }

        protected override void LogEventsFromTextOutput(string singleLine, MessageImportance messageImportance)
        {
            base.LogEventsFromTextOutput(GCCUtilities.ConvertGCCOutput(singleLine), messageImportance);
        }

        protected override string GenerateResponseFileCommands()
        {
            StringBuilder responseFileCmds = new StringBuilder(GCCUtilities.s_CommandLineLength);

            foreach (ITaskItem sourceFile in Sources)
            {
                responseFileCmds.Append(GCCUtilities.ConvertPathWindowsToPosix(sourceFile.GetMetadata("Identity")));
                responseFileCmds.Append(" ");
            }

            responseFileCmds.Append(m_XamlParser.Parse(Sources[0], false));

            return responseFileCmds.ToString();
        }

        public override bool Execute()
        {
            bool returnResult = false;

            try
            {
                m_XamlParser = new XamlParser(PropertiesFile);

                returnResult = base.Execute();
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

        protected override void PostProcessSwitchList()
        {
            //skip default behavior
        }

        protected override string GenerateFullPathToTool()
        {
            return this.ToolName;
        }

        protected override string TrackerIntermediateDirectory
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

        protected override Encoding ResponseFileEncoding
        {
            get
            {
                return Encoding.ASCII;
            }
        }

        protected override ITaskItem[] TrackedInputFiles
        {
            get
            {
                return this.Sources;
            }
        }

        protected override bool MaintainCompositeRootingMarkers
        {
            get
            {
                return true;
            }

        }

        protected override string ToolName
        {
            get
            {
                return NaClLinkerPath;
            }
        }

        public override bool AttributeFileTracking
        {
            get
            {
                return true;
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

        public string TrackerLogDirectory { get; set; }
	}
}
