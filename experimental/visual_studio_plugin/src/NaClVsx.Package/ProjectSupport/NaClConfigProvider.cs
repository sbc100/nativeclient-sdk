using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Project;

namespace Google.NaClVsx.ProjectSupport {
  class NaClConfigProvider : ConfigProvider {

    public static readonly string[] kPlatforms = {
                                                   "NaCl",
                                                 };

    public NaClConfigProvider(ProjectNode manager) : base(manager) {}

    public override int GetPlatformNames(uint celt,
                                         string[] names,
                                         uint[] actual) {
       /* From MSDN:
        * Typically two calls are made to GetPlatformNames.
        * With the first call, celt is set to zero, rgbstr
        * to nullNothingnullptra null reference (Nothing in
        * Visual Basic), and pcActual to a valid address.
        * GetPlatformNames returns with pcActual pointing to
        * the number of platform names available. The caller
        * uses this information to allocate rgbstr to the
        * appropriate size and call GetPlatformNames a second
        * time with celt set to the contents of pcActual.
        */
      if (celt == 0) {
        actual[0] = (uint)kPlatforms.Length;
      } else {
        Debug.Assert(celt <= kPlatforms.Length);
        kPlatforms.CopyTo(names,0);
        actual[0] = (uint)kPlatforms.Length;
      }
      return VSConstants.S_OK;
    }

    protected override ProjectConfig CreateProjectConfiguration(string configName)
    {
      return new NaClProjectConfig(this.ProjectMgr, configName);
    }
  }
}