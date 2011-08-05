#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Class that represents an NSIS installer script."""

import os
import subprocess
import tarfile
import tempfile


class NsisScript:
  '''Container for a NSIS script file

  Use this class to create and manage an NSIS script.  You can construct this
  class with an existing script file.  You can Compile() the script to produce
  and NSIS installer.
  '''

  def __init__(self, script_file='nsis.nsi'):
    self._script_file = script_file
    self._install_dir = "C:\\nsis_install"
    self._relative_install_root = None
    self._file_list = []
    self._dir_list = []
    self._symlink_list = []

  def GetScriptFile(self):
    '''Accessor for |_script_file|.'''
    return self._script_file

  def GetInstallDirectory(self):
    '''The installation directory.
    Return:
      The value of the installation directory.  This is where the NSIS installer
      built from this script puts its contents when run.
    '''
    return self._install_dir

  def SetInstallDirectory(self, install_dir):
    '''Set the install name for the script.

    This sets the InstallDir property in the NSIS script.  When the final
    installer is run, the contents get put here.

    Args:
      install_dir: The target install directory.  Should be a full path
          (including a drive letter on Windows).
    '''
    self._install_dir = install_dir

  def GetFileList(self):
    '''Accessor for |_file_list|.'''
    return self._file_list

  def GetDirectoryList(self):
    '''Accessor for |_dir_list|.'''
    return self._dir_list

  def GetSymlinkList(self):
    '''Accessor for |_symlink_list|.'''
    return self._symlink_list

  def CreateFromDirectory(self,
                          artifact_dir,
                          dir_filter=None,
                          file_filter=None):
    '''Create the list of installer artifacts.

    Creates three lists:
      1. a list of directories that need to be generated by the installer
      2. a list of files that get installed into those directories
      3. a list of symbolic links that need to be created by the installer
    These lists are used later on when generating the section commands that are
    compiled into the final installer

    Args:
      artifact_dir: The directory containing the files to add to the NSIS
          installer script.  The NSIS installer will reproduce this directory
          structure.

      dir_filter: A filter function for directories.  This can be written as a
          list comprehension.  If dir_filter is not None, then it is called
          with |_dir_list|, and |_dir_list| is replaced with the filter's
          output.

      file_filter: A filter function for files.  This can be written as a
          list comprehension.  If file_filter is not None, then it is called
          with |_file_list|, and |_file_list| is replaced with the filter's
          output.
    '''
    self._relative_install_root = artifact_dir
    self._file_list = []
    self._dir_list = []
    self._symlink_list = []
    for root, dirs, files in os.walk(artifact_dir):
      self._dir_list.extend([os.path.join(root, d) for d in dirs])
      self._file_list.extend([os.path.join(root, f)
          for f in files if not os.path.islink(f)])
      self._symlink_list.extend([os.path.join(root, l)
          for l in files if os.path.islink(l)])
    if dir_filter:
      self._dir_list = dir_filter(self._dir_list)
      self._symlink_list = dir_filter(self._symlink_list)
    if file_filter:
      self._file_list = file_filter(self._file_list)


  def CreateInstallNameScript(self, cwd='.'):
    '''Write out the installer name script.

    The installer name script is in the special NSIS include file
    sdk_install_name.nsh.  This file is expected to be in |cwd|.  If
    sdk_install_name.nsh already exists, it is overwritten.

    Args:
      cwd: The directory where sdk_install_name.sdk is placed.
    '''
    with open(os.path.join(cwd, 'sdk_install_name.nsh'), 'wb') as script:
      script.write('InstallDir %s\n' % self._install_dir)

  def NormalizeInstallPath(self, path):
    '''Normalize |path| for installation.

    If |_relative_install_root| is set, then normalize |path| by making it
    relative to |_relative_install_root|.

    Args:
      path: The path to normalize.

    Returns: the normalized path.  If |_relative_install_root| is None, then
        the return value is the same as |path|.
    '''
    if (self._relative_install_root and
        path.startswith(self._relative_install_root)):
      return path[len(self._relative_install_root) + 1:]
    else:
      return path

  def CreateSectionNameScript(self, cwd='.'):
    '''Write out the section script.

    The section script is in the special NSIS include file sdk_section.nsh.
    This file is expected to be in |cwd|.  If sdk_section.nsh already exists,
    it is overwritten.

    If |_relative_install_root| is set, then that part of the path is stripped
    off of the installed file and directory names.  This will cause the files
    to be installed as if they were relative to |_relative_install_root| and
    not in an absolute location.

    Args:
      cwd: The directory where sdk_section.sdk is placed.
    '''
    with open(os.path.join(cwd, 'sdk_section.nsh'), 'wb') as script:
      script.write('Section "!Native Client SDK" NativeClientSDK\n')
      script.write('  SectionIn RO\n')
      script.write('  SetOutPath $INSTDIR\n')
      for d in self._dir_list:
        d = self.NormalizeInstallPath(d)
        script.write('  CreateDirectory "%s"\n' % os.path.join('$INSTDIR', d))
      for f in self._file_list:
        f_rel = self.NormalizeInstallPath(f)
        script.write('  File "/oname=%s" "%s"\n' % (f_rel, f))
      for l in self._symlink_list:
        l = self.NormalizeInstallPath(l)
        script.write('  MkLink::SoftD "%s" "%s"\n' % (
            os.path.join('$INSTDIR', l), os.path.realpath(l)))
      script.write('SectionEnd\n')

  def Compile(self):
    '''Compile the script.

    Compilation happens in a couple of steps: first, the install directory
    script is generated from |_install_dir|, then the section commands script
    is generated from |_file_list|.  Finally |_script_file| is compiled, which
    produces the NSIS installer specified by the OutFile property in
    |_script_file|.
    '''
    working_dir = os.path.dirname(self._script_file)
    self.CreateInstallNameScript(cwd=working_dir)
    self.CreateSectionNameScript(cwd=working_dir)
    # Run the NSIS compiler.
    subprocess.check_call([os.path.join('NSIS', 'makensis'),
                           '/V2',
                           self._script_file],
                          cwd=working_dir,
                          shell=True)
