// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
using System;
using System.Diagnostics;
using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;
using Google.MsAd7.BaseImpl;
using Google.NaClVsx.DebugSupport;
using Google.NaClVsx.Installation;
using Google.NaClVsx.ProjectSupport;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Project;

namespace Google.NaClVsx {
  /// <summary>
  /// This is the class that implements the package exposed by this assembly.
  ///
  /// The minimum requirement for a class to be considered a valid package for Visual Studio
  /// is to implement the IVsPackage interface and register itself with the shell.
  /// This package uses the helper classes defined inside the Managed Package Framework (MPF)
  /// to do it: it derives from the Package class that provides the implementation of the 
  /// IVsPackage interface and uses the registration attributes defined in the framework to 
  /// register itself and its components with the shell.
  /// </summary>
  // This attribute tells the registration utility (regpkg.exe) that this class needs
  // to be registered as package.
  [PackageRegistration(UseManagedResourcesOnly = true)]
  // A Visual Studio component can be registered under different registry roots; for instance
  // when you debug your package you want to register it in the experimental hive. This
  // attribute specifies the registry root to use if none is provided to regpkg.exe with
  // the /root switch.
  [DefaultRegistryRoot("Software\\Microsoft\\VisualStudio\\9.0")]
  // This attribute is used to register the informations needed to show the this package
  // in the Help/About dialog of Visual Studio.
  [InstalledProductRegistration(true, "#110", "#112", "1.0",
      IconResourceID = 400)]
  // In order be loaded inside Visual Studio in a machine that has not the VS SDK installed, 
  // package needs to have a valid load key (it can be requested at 
  // http://msdn.microsoft.com/vstudio/extend/). This attributes tells the shell that this 
  // package has a load key embedded in its resources.
  [ProvideLoadKey("Standard", "1.0", "Google Native Client Support",
      "Google, Inc.", 101)]
  [Guid(NaClGuids.kGuidNaClVsxPackagePkgString)]
  [DebugEngineRegistration(
      DebugEngineId = Engine.kId,
      Disassembly = true,
      Name = Engine.kName,
      Attach = true,
      AlwaysLoadLocal = true,
      DebugEngineClsId = Engine.kClsId,
      LoadUnderWow64 = false,
      LoadProgramProviderUnderWow64 = true,
      ProgramProviderClsId = ProgramProvider.kClsId,
      PortSupplierClsIds = NaClPortSupplier.kClsId + ", " + NaClPortSupplier.kClsId
      )]
  [PortSupplierRegistration(typeof(NaClPortSupplier), Name = "Native Client")]
  [ProvideProjectFactory(
      typeof(NaClProjectFactory),
      "NaCl",
      "NaCl Project Files (*naclproj);*.naclproj",
      "naclproj",
      "naclproj",
      @"..\..\Templates\Projects",
      LanguageVsTemplate = "NaCl"
    )]
  [ProvideAutoLoad("adfc4e64-0397-11d1-9f4e-00a0c911004f")]
  [ProvideObject(typeof (Engine))]
  [ProvideObject(typeof (ProgramProvider))]
  [ProvideObject(typeof(GeneralProperties))]
  [ProvideObject(typeof(DebugProperties))]
  [ProvideObject(typeof(NaClPortSupplier))]
  [ProvideMSBuildTargets("NaCl_1.0", @"%ProgramFiles%\MSBuild\Google\NaCl\1.0\NaCl.Common.targets")]
  public sealed class NaClPackage : ProjectPackage, IVsInstalledProduct
  {
    /// <summary>
    /// Default constructor of the package.
    /// Inside this method you can place any initialization code that does not require 
    /// any Visual Studio service because at this point the package object is created but 
    /// not sited yet inside Visual Studio environment. The place to do all the other 
    /// initialization is the Initialize method.
    /// </summary>
    public NaClPackage() {
      Trace.WriteLine(string.Format(CultureInfo.CurrentCulture,
                                    "Entering constructor for: {0}",
                                    ToString()));
    }


    /////////////////////////////////////////////////////////////////////////////
    // Overriden Package Implementation

    /// <summary>
    /// Initialization of the package; this method is called right after the package is sited, so this is the place
    /// where you can put all the initialization code that rely on services provided by VisualStudio.
    /// </summary>
    protected override void Initialize() {
      Trace.WriteLine(string.Format(CultureInfo.CurrentCulture,
                                    "Entering Initialize() of: {0}",
                                    ToString()));
      base.Initialize();
      RegisterProjectFactory(new NaClProjectFactory(this));
    }

    #region Implementation of IVsInstalledProduct

    int IVsInstalledProduct.IdBmpSplash(out uint pIdBmp) {
      pIdBmp = 400;
      return VSConstants.S_OK;
    }

    int IVsInstalledProduct.OfficialName(out string pbstrName) {
      pbstrName = "Google Native Client Support";
      return VSConstants.S_OK;
    }

    int IVsInstalledProduct.ProductID(out string pbstrPID) {
      pbstrPID = "1.0";
      return VSConstants.S_OK;
    }

    int IVsInstalledProduct.ProductDetails(out string pbstrProductDetails) {
      pbstrProductDetails = "Adds support for Google's Native Client SDK, a runtime and toolchain for secure, "
        + "high-performance Web applications. For more information see http://code.google.com/p/nativeclient-sdk/";
      return VSConstants.S_OK;
    }

    int IVsInstalledProduct.IdIcoLogoForAboutbox(out uint pIdIco) {
      pIdIco = 400;
      return VSConstants.S_OK;
    }

    #endregion
  }
}