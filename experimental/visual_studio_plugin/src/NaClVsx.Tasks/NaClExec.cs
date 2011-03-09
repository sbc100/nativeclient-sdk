using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.Build.Utilities;

namespace NaClVsx.Tasks
{
  public class NaClExec : ToolTask
  {
    public string Command
    {
      get { return command_; }
      set { command_ = value; }
    }

    public string Arguments
    {
      get { return arguments_; }
      set { arguments_ = value; }
    }

    public string NaclPath
    {
      get { return naclPath_; }
      set { naclPath_ = value; }
    }

    #region Overrides of Task

    protected override string GenerateFullPathToTool() {
      return Path.Combine(naclPath_, command_);
    }

    protected override string ToolName {
      get { return command_; }
    }

    protected override Encoding StandardErrorEncoding {
      // GDB uses Unicode characters for forward and backticks;
      // failing to set utf8 encoding causes the logger to emit
      // garbage characters instead.
      get { return Encoding.UTF8; }
    }

    protected override string GenerateCommandLineCommands()
    {
      return arguments_;
    }

    protected override System.Collections.Specialized.StringDictionary EnvironmentOverride
    {
      get
      {
        var result = new StringDictionary();
        result.Add("CYGWIN", "nodosfilewarning");
        return result;
      }
    }

    protected override void LogEventsFromTextOutput(string singleLine, Microsoft.Build.Framework.MessageImportance messageImportance)
    {
      var capture = errorTransform.Match(singleLine);
      string outputLine = errorTransform.Replace(singleLine, @"(${line}):");
      base.LogEventsFromTextOutput(outputLine, messageImportance);
    }

    #endregion
    private string command_;
    private string arguments_;
    private string naclPath_;
    private Regex errorTransform = new Regex(@":(?<line>\d+):", RegexOptions.Compiled);
  }
}
