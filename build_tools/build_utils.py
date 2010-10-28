#!/usr/bin/python
# Copyright (c) 2010 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.

"""Small utility library of python functions used by the various package
installers.
"""

import errno
import os
import shutil
import subprocess
import sys

# Map the string stored in |sys.platform| into a toolchain platform specifier.
PLATFORM_MAPPING = {
    'win32': 'win_x86',
    'cygwin': 'win_x86',
    'linux': 'linux_x86',
    'linux2': 'linux_x86',
    'darwin': 'mac_x86',
    'macos': 'mac_x86',
}

TOOLCHAIN_AUTODETECT = "AUTODETECT"

# Make all the directories in |abs_path|.  If |abs_path| points to a regular
# file, it is removed before an attempt to make the directories.  If |abs_path|
# already points to a directory, this method does nothing.
def ForceMakeDirs(abs_path, mode=0755):
  try:
    # Remove an existing regular file; ignore errors (e.g. file doesn't exist).
    # If there are permission problems, they will be caught by the exception
    # handler around the os.makedirs call.
    os.remove(abs_path)
  except:
    pass
  try:
    os.makedirs(abs_path, mode)
  except OSError, (os_errno, os_strerr):
    # If the error is anything but EEXIST (file already exists), then print an
    # informative message and re-raise.  It is not and error if the directory
    # already exists.
    if os_errno != errno.EEXIST:
      print 'ForceMakeDirs(%s, 0%o) FAILED: %s' % (abs_path, mode, os_strerr)
      raise
    pass


# patch version 2.6 doesn't work.  Most of our Linux distros use patch 2.6.
# Returns |True| if the version of patch is usable (that is, not version 2.6).
def CheckPatchVersion():
  patch = subprocess.Popen("patch --version",
                            shell=True,
                            stdout=subprocess.PIPE)
  sed = subprocess.Popen("sed q",
                         shell=True,
                         stdin=patch.stdout,
                         stdout=subprocess.PIPE)
  sed_output = sed.communicate()[0]
  if sed_output.strip() == 'patch 2.6':
    print "patch 2.6 is incompatible with these scripts."
    print "Please install either version 2.5.9 (or earlier)"
    print "or version 2.6.1 (or later)."
    return False
  return True


# Build a toolchain path based on the platform type.  |base_dir| is the root
# directory which includes the platform-specific toolchain.  This could be
# something like "/usr/local/mydir/nacl_sdk/src".  If |base_dir| is None, then
# the environment variable NACL_SDK_ROOT is used (if it's set).
# This method assumes that the platform-specific toolchain is found under
# <base_dir>/toolchain/<platform_spec>.
def NormalizeToolchain(toolchain=TOOLCHAIN_AUTODETECT, base_dir=None):
  def AutoDetectToolchain(toochain_base):
    if sys.platform in PLATFORM_MAPPING:
      return os.path.join(toochain_base,
                          'toolchain',
                          PLATFORM_MAPPING[sys.platform])
    else:
      print 'ERROR: Unsupported platform "%s"!' % sys.platform
      return toochain_base

  if toolchain == TOOLCHAIN_AUTODETECT:
    if base_dir is None:
      base_dir = os.getenv('NACL_SDK_ROOT', '')
    normalized_toolchain = AutoDetectToolchain(base_dir)
  else:
    normalized_toolchain = os.path.abspath(toolchain)
  return normalized_toolchain
