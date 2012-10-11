// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System.Reflection;
using System.Runtime.CompilerServices;

// General Information about an assembly is controlled through the following
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("Native Client Visual Studio Add-in")]
[assembly: AssemblyDescription("An add-in for Visual Studio aiding Native Client development")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany("Google Inc.")]
[assembly: AssemblyProduct("Native Client SDK Visual Studio Add-in")]
[assembly: AssemblyCopyright("Copyright © 2012 The Chromium Authors")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

// Version information for an assembly consists of the following four values:
//
//    Major Version
//    Minor Version
//    Revision
//    Build Number
//
// You can specify all the value or you can default the Revision and
// Build Numbers by using the '*' as shown below:
[assembly: AssemblyVersion("1.0")]
[assembly: AssemblyFileVersion("1.0.*")]

// The following version is what we display as the version within visual studio
[assembly: AssemblyInformationalVersion("1.0.svn")]

[assembly: System.Runtime.CompilerServices.InternalsVisibleTo("UnitTests")]
