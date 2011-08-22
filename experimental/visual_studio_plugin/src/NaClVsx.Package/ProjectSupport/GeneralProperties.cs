#region

using System;
using System.ComponentModel;
using System.Drawing.Design;
using System.Runtime.InteropServices;
using System.Windows.Forms.Design;

#endregion

namespace Google.NaClVsx.ProjectSupport {
  public enum TargetArchitecture {
    x86_32,
    x86_64,
    arm,
  }

  [ComVisible(true)]
  [Guid("22F5ABBB-A1B5-4d21-85E5-02C86535E9ED")]
  class GeneralProperties : AutoSettingsPage {
    #region Tags enum

    public enum Tags {
      TargetArch,
      OutputDir,
      IntermediateDir,
    }

    #endregion

    public GeneralProperties() {
      Name = "General";
    }

    [Category("Compiler")]
    [DisplayName("Architecture")]
    [ProjectProperty("TargetArch", true)]
    public TargetArchitecture Arch {
      get { return arch_; }
      set {
        arch_ = value;
        IsDirty = true;
      }
    }

    [Category("Compiler")]
    [DisplayName("Native Client SDK Root")]
    [ProjectProperty("NaClSDKRoot", true)]
    [Description(
        "The base location of the Native Client SDK installation.  This sets \"NaClSDKRoot\""
        )]
    public string NaClSdkRoot {
      get { return naclSdkRoot_; }
      set {
        naclSdkRoot_ = value;
        IsDirty = true;
      }
    }

    [Category("Compiler")]
    [DisplayName("CCFLAGS")]
    [ProjectProperty("CCFLAGS", true)]
    [Description(
        "CCFLAGS: passed on to nacl-gcc or nacl-g++"
        )]
    public string CCFLAGS {
      get { return naclCcFlags_; }
      set {
        naclCcFlags_ = value;
        IsDirty = true;
      }
    }

    [Category("Compiler")]
    [DisplayName("CFLAGS")]
    [ProjectProperty("CFLAGS", true)]
    [Description(
        "CFLAGS: passed on to nacl-gcc (not nacl-g++)"
        )]
    public string CFLAGS {
      get { return naclCFlags_; }
      set {
        naclCFlags_ = value;
        IsDirty = true;
      }
    }

    [Category("Compiler")]
    [DisplayName("CXXFLAGS")]
    [ProjectProperty("CXXFLAGS", true)]
    [Description(
        "CXXFLAGS: passed on to nacl-g++ (not nacl-gcc)"
        )]
    public string CXXFLAGS {
      get { return naclCxxFlags_; }
      set {
        naclCxxFlags_ = value;
        IsDirty = true;
      }
    }


    [Category("Compiler")]
    [DisplayName("INCLUDES")]
    [ProjectProperty("INCLUDES", true)]
    [Description(
        "Include Paths: passed on to nacl-gcc or nacl-g++"
        )]
    public string INCLUDES {
      get { return naclIncludes_; }
      set {
        naclIncludes_ = value;
        IsDirty = true;
      }
    }

    [Category("Compiler")]
    [DisplayName("OPT_FLAGS")]
    [ProjectProperty("OPT_FLAGS", true)]
    [Description(
        "Optimization flags (such as -O2): passed on to nacl-gcc or nacl-g++"
        )]
    public string OPT_FLAGS {
      get { return naclOptFlags_; }
      set {
        naclOptFlags_ = value;
        IsDirty = true;
      }
    }

    [Category("Linker")]
    [DisplayName("Library Dependencies")]
    [ProjectProperty("LinkLibs", true)]
    [Description(
        "List of libraries to link, separated by semicolons. Omit 'lib prefix and '.a' suffix (e.g. 'iberty' not 'libiberty.a')."
        )]
    public string Libs {
      get { return libs_; }
      // MSVS allows libs to be space-delimited. But in order for our
      // msbuild rules to work, they need to be semicolon-delimited.
      // Help the user by replacing spaces.
      // There doesn't seem to be a good reason to go the other way,
      // that is, convert spaces to semicolons. The user should see the
      // changes immediately.
      set {
        libs_ = !String.IsNullOrEmpty(value)
                    ? value.Trim().Replace(' ', ';')
                    : value;
        IsDirty = true;
      }
    }


    [Category("Linker")]
    [DisplayName("Additional Library Directories")]
    [ProjectProperty("Lib", true)]
    [Description("Additional library paths to search")]
    public string LibPath {
      get { return libPath_; }
      set {
        libPath_ = value;
        IsDirty = true;
      }
    }


    [Category("Paths")]
    [DisplayName("Output Directory")]
    [ProjectProperty("OutputPath", true)]
    [Editor(typeof (FolderNameEditor), typeof (UITypeEditor))]
    public string OutputDir {
      get { return outputDir_; }
      set {
        outputDir_ = value;
        IsDirty = true;
      }
    }

    [Category("Paths")]
    [DisplayName("Intermediate Directory")]
    [ProjectProperty("IntermediatePath", true)]
    [Editor(typeof (FolderNameEditor), typeof (UITypeEditor))]
    public string IntermediateDir {
      get { return intermediateDir_; }
      set {
        intermediateDir_ = value;
        IsDirty = true;
      }
    }

    [Category("Paths")]
    [DisplayName("Output File Name")]
    [ProjectProperty("OutputFile", true)]
    public string OutputFileName {
      get { return outputFileName_; }
      set {
        outputFileName_ = value;
        IsDirty = true;
      }
    }

    #region Private Implementation

    private TargetArchitecture arch_;
    private string intermediateDir_;
    private string libPath_;
    private string libs_;
    private string naclCFlags_;
    private string naclCcFlags_;
    private string naclCxxFlags_;
    private string naclIncludes_;
    private string naclOptFlags_;
    private string naclSdkRoot_;
    private string outputDir_;
    private string outputFileName_;

    #endregion
  }
}
