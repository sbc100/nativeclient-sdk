// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

using System::CLSCompliantAttribute;
using System::Reflection::AssemblyDescriptionAttribute;
using System::Reflection::AssemblyCompanyAttribute;
using System::Reflection::AssemblyConfigurationAttribute;
using System::Reflection::AssemblyCopyrightAttribute;
using System::Reflection::AssemblyCultureAttribute;
using System::Reflection::AssemblyProductAttribute;
using System::Reflection::AssemblyTitleAttribute;
using System::Reflection::AssemblyTrademarkAttribute;
using System::Reflection::AssemblyVersionAttribute;
using System::Runtime::InteropServices::ComVisibleAttribute;
using System::Security::Permissions::SecurityAction;
using System::Security::Permissions::SecurityPermissionAttribute;

//
// General Information about an assembly is controlled through the following
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
//
[assembly:AssemblyTitleAttribute("NaClVsxDebugHelpers")];
[assembly:AssemblyDescriptionAttribute("")];
[assembly:AssemblyConfigurationAttribute("")];
[assembly:AssemblyCompanyAttribute("Microsoft")];
[assembly:AssemblyProductAttribute("NaClVsxDebugHelpers")];
[assembly:AssemblyCopyrightAttribute("Copyright (c) Microsoft 2010")];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("")];

//
// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version
//      Build Number
//      Revision
//
// You can specify all the value or you can
// default the Revision and Build Numbers
// by using the '*' as shown below:

[assembly:AssemblyVersionAttribute("1.0.*")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];

[assembly:SecurityPermission(
                             SecurityAction::RequestMinimum,
                             UnmanagedCode = true)];

