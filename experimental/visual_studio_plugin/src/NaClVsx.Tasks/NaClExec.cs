#region

using System.Collections.Specialized;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

#endregion

namespace NaClVsx.Tasks {
  /// <summary>
  ///   The NaClExec task is used by the NaCl.Common.targets file as a build
  ///   rule. In NaCl.Common.targets, the 'Compile' step gets passed all the
  ///   source files as a list, but then NaClExec is called with each file to
  ///   compile it. When NaClExec is called/used in NaCl.Common.targets
  ///   arguments are supplied, which are reflected by the properties below
  ///   (which have get and/or set definitions). Here is a sample usage:
  ///   <NaClExec Command = "$(gcpp)" 
  ///       C_Command = "$(gcc)"
  ///       CFlags = "$(CFLAGS)"
  ///       CCFlags = "$(CCFLAGS)"
  ///       CXXFlags = "$(CXXFLAGS)"
  ///       FileExtension = "@(Compile -> '%(extension)')"
  ///       Arguments = "$(cflags) $(INCLUDES) $(OPT_FLAGS) $(archflag) -c
  ///         '%(FullPath)' -o @(Compile -> '$(IntermediatePath)%(filename).o')"
  ///       ToolPath = "$(NaClSDKRoot)\toolchain\win_x86_newlib\bin\" />
  /// </summary>
  /// Note that we pass in both Command and C_Command, since this task looks at
  /// FileExtension and then chooses the correct Command (nacl-gcc or nacl-g++).
  public class NaClExec : ToolTask {
    public string Command {
      get { return command_; }
      set { command_ = value; }
    }

    /// <summary>
    ///   Command to execute for C files (i.e. when extension is ".c"
    /// </summary>
    public string C_Command {
      get { return c_command_; }
      set { c_command_ = value; }
    }

    /// <summary>
    ///   Compilation arguments...except not CFLAGS, CCFLAGS, or CXXFLAGS
    /// </summary>
    public string Arguments {
      get { return arguments_; }
      set { arguments_ = value; }
    }

    public string CFlags {
      get { return c_flags_; }
      set { c_flags_ = value; }
    }

    public string CCFlags {
      get { return cc_flags_; }
      set { cc_flags_ = value; }
    }

    public string CXXFLags {
      get { return cxx_flags_; }
      set { cxx_flags_ = value; }
    }

    public string FileExtension {
      get { return file_extension_; }
      set { file_extension_ = value; }
    }

    public string NaclPath {
      get { return naclPath_; }
      set { naclPath_ = value; }
    }

    #region Overrides of Task

    /// <summary>
    ///   The ToolName depends on the extension of the file.  For .c files,
    ///   return the C command (<arch>-nacl-gcc);
    ///   otherwise, return command (<arch>-nacl-g++).
    /// </summary>
    protected override string ToolName {
      get {
        if (file_extension_ == ".c") {
          return c_command_;
        } else {
          return command_;
        }
      }
    }

    protected override Encoding StandardErrorEncoding {
      // GDB uses Unicode characters for forward and backticks;
      // failing to set utf8 encoding causes the logger to emit
      // garbage characters instead.
      get { return Encoding.UTF8; }
    }

    protected override StringDictionary EnvironmentOverride {
      get {
        var result = new StringDictionary();
        result.Add("CYGWIN", "nodosfilewarning");
        return result;
      }
    }

    protected override string GenerateFullPathToTool() {
      return Path.Combine(naclPath_, command_);
    }

    /// <summary>
    ///   CommandLineCommands is the concatenation of several values.
    ///   It is *either* CFLAGS or CXXFLAGS, followed by CCFLAGS, 
    ///   followed by arguments_.
    /// </summary>
    protected override string GenerateCommandLineCommands() {
      if (file_extension_ == ".c") {
        return c_flags_ + " " + cc_flags_ + " " + arguments_;
      } else {
        return cxx_flags_ + " " + cc_flags_ + " " + arguments_;
      }
    }

    protected override void LogEventsFromTextOutput(string singleLine,
                                                    MessageImportance
                                                        messageImportance) {
      var capture = errorTransform.Match(singleLine);
      var outputLine = errorTransform.Replace(singleLine, @"(${line}):");
      base.LogEventsFromTextOutput(outputLine, messageImportance);
    }

    #endregion

    #region Private Implementation

    private readonly Regex errorTransform = new Regex(
        @":(?<line>\d+):", RegexOptions.Compiled);

    private string arguments_;
    private string c_command_;
    private string c_flags_;
    private string cc_flags_;
    private string command_;
    private string cxx_flags_;
    private string file_extension_;
    private string naclPath_;

    #endregion
  }
}
