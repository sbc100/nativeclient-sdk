using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.Project;
using GuidAttribute=System.Runtime.InteropServices.GuidAttribute;
using System.Drawing;
using System.Windows.Forms;

namespace Google.NaClVsx.ProjectSupport
{
  [Guid("71BA2B25-4F44-4D94-91E9-7DC5FCF793AA")]
  class NaClProjectNode : ProjectNode
  {
    private static ImageList imageList;
    internal static int imageIndex;

    public override int ImageIndex
    {
      get { return imageIndex + 0; }
    }

    public NaClProjectNode() { 
      imageList =
        Utilities.GetImageList(
            typeof(NaClProjectNode).Assembly.GetManifestResourceStream(
                "Google.NaClProject.Templates.ProjectItems.NaClClass.NaClIcon.ico"));

    }

    public NaClProjectNode(NaClPackage package) {
      imageIndex = this.ImageHandler.ImageList.Images.Count;
      foreach (Image img in imageList.Images)
      {
        this.ImageHandler.AddImage(img);
      }
    }

    #region Overrides of ProjectNode

    /// <summary>
    /// This Guid must match the Guid you registered under
    /// HKLM\Software\Microsoft\VisualStudio\%version%\Projects.
    /// Among other things, the Project framework uses this 
    /// guid to find your project and item templates.
    /// </summary>
    public override Guid ProjectGuid {
      get { return typeof (NaClProjectFactory).GUID; }
    }

    /// <summary>
    /// Returns a caption for VSHPROPID_TypeName.
    /// </summary>
    /// <returns></returns>
    public override string ProjectType {
      get { return "NaCl Project"; }
    }

    protected override ConfigProvider CreateConfigProvider()
    {
      return new NaClConfigProvider(this);
    }

    public override MSBuildResult Build(uint vsopts, string config, Microsoft.VisualStudio.Shell.Interop.IVsOutputWindowPane output, string target)
    {
      return base.Build(vsopts, config, output, target);
    }

    protected override Guid[] GetConfigurationIndependentPropertyPages()
    {
      var result = new Guid[] {
          typeof (GeneralProperties).GUID,
          typeof (DebugProperties).GUID,
      };
      return result;
    }

    public override void AddFileFromTemplate(
        string source, string target)
    {
      this.FileTemplateProcessor.UntokenFile(source, target);
      this.FileTemplateProcessor.Reset(); 
    }

    /// <summary>
    /// Create a file node based on an msbuild item.
    /// </summary>
    /// <param name="item">msbuild item</param>
    /// <returns>FileNode added</returns>
    public override FileNode CreateFileNode(ProjectElement item)
    {
      FileNode newNode = new FileNode(this, item);
      return newNode;
    }

    /// <summary>
    /// Create a file node based on a string.
    /// </summary>
    /// <param name="file">filename of the new filenode</param>
    /// <returns>File node added</returns>
    public override FileNode CreateFileNode(string file)
    {
      ProjectElement item = this.AddFileToMsBuild(file);
      FileNode newNode = this.CreateFileNode(item);
      if (file.EndsWith(".cc") || file.EndsWith(".cpp") || 
          file.EndsWith(".c")) {
        // We added the file to the MsBuild (a few lines above), now change
        // it's ItemName to "Compile" instead of "Content" so it gets compiled.
        newNode.ItemNode.ItemName = "Compile";
      }
      return newNode;
    }

    #endregion
  }
}
