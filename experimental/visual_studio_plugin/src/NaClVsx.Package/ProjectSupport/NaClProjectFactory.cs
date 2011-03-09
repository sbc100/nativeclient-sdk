// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using IOleServiceProvider = Microsoft.VisualStudio.OLE.Interop.IServiceProvider;
using Microsoft.VisualStudio.Project;

namespace Google.NaClVsx.ProjectSupport {
  [Guid(NaClGuids.kGuidNaClVsxProjectFactoryString)]
  class NaClProjectFactory : ProjectFactory {

    public NaClProjectFactory(Package package) : base(package) {
      Trace.WriteLine("Entered NaClProjectFactory constructor");
    }

    #region Implementation of IVsProjectFactory

    public int CanCreateProject(string pszFilename,
                                uint grfCreateFlags,
                                out int pfCanCreate) {
      pfCanCreate = 1;
      return VSConstants.S_OK;
      // TO DO: Under what circumstances should we
      // throw a NotImplementedException();
    }

    protected override ProjectNode CreateProject() {
      ProjectNode proj = new NaClProjectNode();
      proj.SetSite((IOleServiceProvider)((IServiceProvider)this.Package).GetService(typeof(IOleServiceProvider)));
      return proj;
    }

    public int SetSite(IServiceProvider psp) {
      throw new NotImplementedException();
    }

    public int Close() {
      throw new NotImplementedException();
    }

    #endregion
  }
}