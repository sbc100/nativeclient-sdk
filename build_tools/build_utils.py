#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.

"""Small utility library of python functions used by the various package
installers.
"""

import errno
import os
import re
import shutil
import subprocess
import sys

#------------------------------------------------------------------------------
# Parameters

# Revision numbers for the SDK
MAJOR_REV = '0'
MINOR_REV = '3'

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

#------------------------------------------------------------------------------
# Functions

# Make all the directories in |abs_path|.  If |abs_path| points to a regular
# file, it is removed before an attempt to make the directories.  If |abs_path|
# already points to a directory, this method does nothing.
def ForceMakeDirs(abs_path, mode=0755):
  if os.path.isdir(abs_path):
    return
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


# Return a shell environment suitable for use by Mac, Linux and Windows.  On
# Mac and Linux, this is just a copy of os.environ.  On Windows, the PATH
# variable is extended to include the hermetic cygwin installation.
# |nacl_sdk_root| should point to the place where the hermetic cygwin was
# installed, this is typically something like C:/nacl_sdk/src.  If
# |nacl_sdk_root| is None, then the NACL_SDK_ROOT environment variable is used.
# if NACL_SDK_ROOT is not set, then the location of this script file is used.
def GetShellEnvironment(nacl_sdk_root=None):
  shell_env = os.environ.copy()
  if (sys.platform == 'win32'):
    # This adds the assumption that cygwin is installed in the default
    # location when building the SDK for windows.
    if nacl_sdk_root is None:
      toolchain_dir = os.path.dirname(os.path.dirname(
                                      os.path.abspath(__file__)))
      nacl_sdk_root = os.getenv('NACL_SDK_ROOT', toolchain_dir)
    cygwin_dir = os.path.join(nacl_sdk_root, 'third_party', 'cygwin', 'bin')
    shell_env['PATH'] = cygwin_dir + ';' + shell_env['PATH']

  return shell_env


# Return a "normalized" path that will work with both hermetic cygwin and *nix
# shell environments.  On *nix, this method just returns the original path; on
# Windows running hermetic cygwin, this alters the path.  |abs_path| must be
# a fully-qualified absolute path, on Windows it must include the drive letter.
# |shell_env| describes the environment used by any subprocess (this is
# normally obtained from GetShellEnvironment()
# TODO(dspringer,khim): make this work properly for hermetic cygwin (see bug
# http://code.google.com/p/nativeclient/issues/detail?id=1122)
def HermeticBuildPath(abs_path, shell_env):
  if (sys.platform == 'win32'):
    return abs_path

  return abs_path


# patch version 2.6 doesn't work.  Most of our Linux distros use patch 2.6.
# Returns |True| if the version of patch is usable (that is, not version 2.6).
# |shell_env| is the enviromnent used to run the subprocesses like patch and
# sed.  If |shell_env| is None, then os.environ is used.
def CheckPatchVersion(shell_env=None):
  if shell_env is None:
    shell_env = os.environ
  patch = subprocess.Popen("patch --version",
                            shell=True,
                            env=shell_env,
                            stdout=subprocess.PIPE)
  sed = subprocess.Popen("sed q",
                         shell=True,
                         env=shell_env,
                         stdin=patch.stdout,
                         stdout=subprocess.PIPE)
  sed_output = sed.communicate()[0]
  if sed_output.strip() == 'patch 2.6':
    print "patch 2.6 is incompatible with these scripts."
    print "Please install either version 2.5.9 (or earlier)"
    print "or version 2.6.1 (or later)."
    return False
  return True

  
def GetDepsValues(deps_loc):  
  """Returns a dict that contains all the variables defined in the DEPS file"""
  local_scope = { }
  
  # This is roughly taken from depot_tools/gclient.py, with unneeded functions
  # stubbed out.
  global_scope = {
    'File': lambda: '--File() Not Implemented--',
    'From': lambda: '--From() Not Implemented--',
    'Var': lambda var: 'Var(%s)' % var,
    'deps_os': {},
  }
  
  # This loads the contents from the DEPS file into the local_scope dict
  execfile(deps_loc, global_scope, local_scope)
  
  return local_scope
  

def GetNaClRevision(deps_loc):
  """Returns the Subversion Revision of the NaCL Repository"""
  return GetDepsValues(deps_loc)['NACL_REVISION']

  
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


def RawVersion():
  '''Returns the Raw version number of the SDK in a dotted format'''
  return '.'.join(GetVersionNumbers())


def GetVersionNumbers():
  '''Returns a list of 4 strings containing the version identifier'''
  rev = str(SVNRevision())
  build_number = os.environ.get('BUILD_NUMBER', '0')
  return [MAJOR_REV, MINOR_REV, rev, build_number]


# Note that this function has to be run from within a subversion working copy,
# or a git repository that is based on a subversion parent.
def SVNRevision():
  p = subprocess.Popen('svn info', shell=True, stdout=subprocess.PIPE)
  if p.wait() != 0:
    p = subprocess.Popen('git svn info', shell=True, stdout=subprocess.PIPE)
    if p.wait() != 0:
      raise AssertionError('Cannot determine SVN revision of this repository');

  svn_info = p.communicate()[0]
  m = re.search('Revision: ([0-9]+)', svn_info)
  if m:
    return int(m.group(1))
  else:
    return 0



def VersionString():
  '''Returns the version of native client based on the svn revision number.'''
  return 'native_client_sdk_%s' % '_'.join(GetVersionNumbers())
