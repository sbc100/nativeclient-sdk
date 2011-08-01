#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Install the NSIS compiler and its SDK."""

import os
import subprocess


# The name of the NSIS installer.  This file is checked into the SDK repo.
NSIS_INSTALLER = 'nsis-2.46-Unicode-setup.exe'

def InstallNsis(installer_exe, target_dir):
  '''Install NSIS into |target_dir|.

  Args:
    installer_exe The full path to the NSIS self-extracting installer.

    target_dir The target directory for NSIS.  The installer is run in this
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
    os.makedirs(target_dir, mode=0777)
  except OSError:
    pass

  subprocess.check_call([installer_exe,
                         '/S',
                         '/D=%s' % target_dir],
                        cwd=os.path.dirname(installer_exe),
                        shell=True)
