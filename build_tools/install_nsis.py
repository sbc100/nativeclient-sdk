#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Install the NSIS compiler and its SDK."""

import os
import shutil
import subprocess
import zipfile


# The name of the archive that contains the AccessControl extensions.
ACCESS_CONTROL_ZIP = 'AccessControl.zip'
# The name of the NSIS installer.  This file is checked into the SDK repo.
NSIS_INSTALLER = 'nsis-2.46-Unicode-setup.exe'
# The default directory name for the NSIS installation.
NSIS_DIR = 'NSIS'

def InstallNsis(installer_exe, target_dir):
  '''Install NSIS into |target_dir|.

  Args:
    installer_exe: The full path to the NSIS self-extracting installer.

    target_dir: The target directory for NSIS.  The installer is run in this
        directory and produces a directory named NSIS that contains the NSIS
        compiler, etc. Must be defined.
  '''
  if not os.path.exists(installer_exe):
    raise IOError('%s not found' % installer_exe)

  if not os.path.isabs(installer_exe):
    raise ValueError('%s must be an absolute path' % installer_exe)
  if not os.path.isabs(target_dir):
    raise ValueError('%s must be an absolute path' % target_dir)

  try:
    os.makedirs(target_dir, mode=0755)
  except OSError:
    pass

  subprocess.check_call([installer_exe,
                         '/S',
                         '/D=%s' % target_dir],
                        cwd=os.path.dirname(installer_exe),
                        shell=True)


def InstallAccessControlExtensions(cwd, access_control_zip, target_dir):
  '''Install the AccessControl extensions into the NSIS directory.

  Args:
    cwd: The current working directory.

    access_control_zip: The full path of the AccessControl.zip file.  The
        contents of this file are extracted using python's zipfile package.

    target_dir: The full path of the target directory for the AccessControl
        extensions.
  '''
  if not os.path.exists(access_control_zip):
    raise IOError('%s not found' % access_control_zip)

  try:
    os.makedirs(os.path.join(target_dir, 'Plugins'), mode=0755)
  except OSError:
    pass

  zip_file = zipfile.ZipFile(access_control_zip, 'r')
  try:
    zip_file.extractall(target_dir)
  finally:
    zip_file.close()
  # Move the AccessControl plugin DLLs into the main NSIS Plugins directory.
  access_control_plugin_dir = os.path.join(target_dir,
                                           'AccessControl',
                                           'Plugins')
  access_control_plugins = [os.path.join(access_control_plugin_dir, p)
      for p in os.listdir(access_control_plugin_dir)]
  dst_plugin_dir = os.path.join(target_dir, 'Plugins')
  for plugin in access_control_plugins:
    shutil.copy(plugin, dst_plugin_dir)


def Install(cwd, target_dir=None, force=False):
  '''Install the entire NSIS compiler and SDK with extensions.

  Installs the NSIS SDK and extensions into |target_dir|.  By default, the
  target directory is NSIS_DIR under |cwd|.

  If NSIS is already installed and |force| is False, do nothing.  If |force|
  is True or NSIS is not already installed, then install NSIS and the necessary
  extensions.

  Args:
    cwd: The current working directory.

    target_dir: NSIS is installed here.  If |target_dir| is None, then NSIS is
        installed in its default location, which is NSIS_DIR under |cwd|.

    force: True means install NSIS whether it already exists or not.
  '''
  # If the NSIS compiler and SDK hasn't been installed, do so now.
  nsis_dir = target_dir or os.path.join(cwd, NSIS_DIR)
  if force or not os.path.exists(os.path.join(nsis_dir, 'makensis.exe')):
    InstallNsis(os.path.join(cwd, NSIS_INSTALLER), nsis_dir)
    InstallAccessControlExtensions(
        cwd, os.path.join(cwd, ACCESS_CONTROL_ZIP), nsis_dir)
