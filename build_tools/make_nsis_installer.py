#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Create the NSIS installer for the SDK."""

import os
import subprocess

from build_tools import build_utils
from build_tools import install_nsis
from build_tools import nsis_script


def MakeNsisInstaller(installer_dir, sdk_version=None, cwd=None):
  '''Create the NSIS installer

  Args:
    installer_dir: The directory containing all the artifacts that get packaged
        in the NSIS installer.

    sdk_version: A string representing the SDK version.  The string is expected
        to be a '.'-separated triple representing <major>.<minor>.<build>.  If
         this argument is None, then the default is the value returned by
        build_utils.RawVersion()

    cwd: The current working directory.  Various artifacts (such as the NSIS
        installer) are expected to be in this directory.  Defaults to the
        script's directory.
  '''
  if not sdk_version:
    sdk_version = build_utils.RawVersion()
  sdk_full_name = 'native_client_sdk_%s' % sdk_version.replace('.', '_')

  if not cwd:
    cwd = os.path.abspath(os.path.dirname(__file__))

  install_nsis.Install(cwd)
  script = nsis_script.NsisScript(os.path.join(cwd, 'make_sdk_installer.nsi'))
  script.SetInstallDirectory(os.path.join('C:', sdk_full_name))
  script.CreateFromDirectory(installer_dir)
  script.Compile()
