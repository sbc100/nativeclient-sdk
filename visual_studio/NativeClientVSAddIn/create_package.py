#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Takes the output of the build step and turns it into a compressed
archive ready for distribution.

This script assumes the build script has been run to compile the add-in.
It zips up all files required for the add-in installation and places the
result in out/vs_addin/vs_addin.tgz.
"""
from __future__ import print_function

import os
import re
import fileinput
import win32api
import shutil
import tarfile
import zipfile
import string
import sys
from os.path import join

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Checkout root
ROOT = os.path.dirname(os.path.dirname(SCRIPT_DIR))

# Root output directory.
BUILD_DIR = join(ROOT, 'out', 'vs_addin')
STAGING_DIR = join(BUILD_DIR, 'staging')

# Directory that contains the build assemblies.
ASSEMBLY_DIRECTORY_2010 = join(BUILD_DIR, '2010', 'Debug')
ASSEMBLY_DIRECTORY_2012 = join(BUILD_DIR, '2012', 'Debug')

# Directory containing static installer resources.
RESOURCE_DIRECTORY = join(SCRIPT_DIR, 'InstallerResources')

# Base name of the final zip file.
OUTPUT_NAME = join(BUILD_DIR, 'vs_addin.tgz')

# AddIn metadata file path. We will modify this with the version #.
ADDIN_METADATA = join(RESOURCE_DIRECTORY, 'NativeClientVSAddIn.AddIn')

# AddIn dll file path. We will obtain our add-in version from this.
ADDIN_ASSEMBLY_2010 = join(ASSEMBLY_DIRECTORY_2010, 'NativeClientVSAddIn.dll')
ADDIN_ASSEMBLY_2012 = join(ASSEMBLY_DIRECTORY_2012, 'NativeClientVSAddIn.dll')

# Regex list to exclude from the archive. If a file path matches any of the
# expressions during a call to AddFolderToArchive it is excluded from the
# archive file.
EXCLUDES = [
    r'\.svn', # Exclude .svn directories.
    r'\.swp$', # Exclude .swp files.
    r'\.pyc$', # Exclude .pyc files.
    r'examples\\.*\\chrome_data',
    r'examples\\.*\\Debug',
    r'examples\\.*\\newlib',
    r'examples\\.*\\glibc',
    r'examples\\.*\\PNaCl',
    r'examples\\.*\\win',
    r'examples\\.*\\ipch',
    r'examples\\.*\\*.sdf',
    r'examples\\.*\\*.suo',
    # Exclude .AddIn file for now since we need to modify it with version info.
    re.escape(ADDIN_METADATA)]

# List of source/destination pairs to include in archive file.
FILE_LIST = [
  (ADDIN_ASSEMBLY_2010, '2010'),
  (ADDIN_ASSEMBLY_2012, '2012'),
  (join(ASSEMBLY_DIRECTORY_2010, 'NativeClientVSAddIn.AddIn'), '2010'),
  (join(ASSEMBLY_DIRECTORY_2012, 'NativeClientVSAddIn.AddIn'), '2012'),
  (join(ASSEMBLY_DIRECTORY_2010, 'NaCl.Build.CPPTasks.dll'), 'NaCl')]


def MakeDir(dirname):
  """Create a directory if it doesn't already exist."""
  if not os.path.isdir(dirname):
    os.makedirs(dirname)


def StageDirectory(dir_to_copy):
  """Recursively add a directory to the staging directory.

  Args:
    dir_to_copy: Directory to add.
    basedir: The directory with the staging directory.
  """
  for root, _, files in os.walk(dir_to_copy):
    for filename in files:
      src_path = join(root, filename)

      # If the file path matches an exclude, don't include it.
      if any(re.search(expr, src_path) for expr in EXCLUDES):
        continue

      relative_root = os.path.relpath(root, dir_to_copy)
      dest_path = join(STAGING_DIR, relative_root, filename)

      MakeDir(os.path.dirname(dest_path))
      assert(not os.path.exists(dest_path))
      shutil.copy(src_path, dest_path)


def AddFolderToArchive(path, archive):
  """Recursively adds a directory to an open archive object.

  The archive must already be open and it is not closed by this function.

  Args:
    path: Direcotory to add.
    archive: Open archive file.
  """
  for root, _, files in os.walk(path):
    for filename in files:
      src_path = join(root, filename)
      archive_dir = os.path.join('vs_addin', os.path.relpath(root, path))
      archive_name = join(archive_dir, filename)
      print('Archiving: %s' % archive_name)
      archive.add(src_path, archive_name)


def CopyAddinFile(assembly, path, vs_version):
  """Copy the .AddIn file to the given path while making the necessary
  replacements.

  The version number is obtained from the NativeClientAddIn.dll assembly which
  is built during the build process.
  """
  infopath = '\\VarFileInfo\\Translation'
  pairs = win32api.GetFileVersionInfo(assembly, infopath)
  lang, codepage = pairs[0]
  infopath = u'\\StringFileInfo\\%04X%04X\\ProductVersion' % (lang, codepage)
  prodVersion = win32api.GetFileVersionInfo(assembly, infopath)
  version = '[%s]' % prodVersion
  print('NaCl VS Add-in %s version: %s' % (vs_version, version))

  metadata_filename = os.path.basename(ADDIN_METADATA)
  modified_file = join(path, metadata_filename)

  # Copy the metadata file to new location and modify the version info.
  with open(ADDIN_METADATA, 'r') as source_file:
    with open(modified_file, 'w') as dest_file:
      data = source_file.read()
      replacements = {'VS_VERSION': vs_version, 'ADDIN_VERSION': version}
      data = string.Template(data).substitute(replacements)
      dest_file.write(data)


def Error(msg):
  sys.stderr.write(msg + '\n')
  sys.exit(1)


def CopyWithReplacement(src, dest, replacements):
  MakeDir(dest)
  src_basename = os.path.basename(src)
  dest_basename = os.path.basename(dest)

  olddir = os.getcwd()
  try:
    os.chdir(src)
    for root, dirs, filenames in os.walk('.'):
      for filename in filenames:
        srcfile = join(root, filename)
        # skip non-files, in particular .svn folders.
        if not os.path.isfile(srcfile):
          continue

        destdir = join(dest, root.replace(src_basename, dest_basename))
        destdir = os.path.normpath(destdir)
        MakeDir(destdir)

        destfile = join(destdir, filename.replace(src_basename, dest_basename))
        if os.path.exists(destfile):
          print('Skipping: %s' % destfile)
          continue

        with open(srcfile, "rb") as f:
          data = f.read()
          for pat, subst in replacements.iteritems():
            data = data.replace(pat, subst)

        with open(destfile, "wb") as f:
          f.write(data)
  finally:
    os.chdir(olddir)


def main(args):
  if not os.path.exists(BUILD_DIR):
    Error("build dir not found: %s" % BUILD_DIR)

  CopyAddinFile(ADDIN_ASSEMBLY_2010, ASSEMBLY_DIRECTORY_2010, '10.0')
  CopyAddinFile(ADDIN_ASSEMBLY_2012, ASSEMBLY_DIRECTORY_2012, '11.0')

  print("Staging package in %s" % STAGING_DIR)
  if os.path.exists(STAGING_DIR):
    shutil.rmtree(STAGING_DIR)

  # Start by staging the entire resource tree
  StageDirectory(RESOURCE_DIRECTORY)

  # Then stage anything in the FILE_LIST
  for source, dest in FILE_LIST:
    file_name = os.path.basename(source)
    dest = join(STAGING_DIR, dest, file_name)
    assert(not os.path.exists(dest))
    MakeDir(os.path.dirname(dest))
    shutil.copy(source, dest)

  # Duplicate the NaCl64 platform but rename it to NaCl32
  src = join(RESOURCE_DIRECTORY, 'NaCl64')

  # Create NaCl32
  dest = join(STAGING_DIR, 'NaCl32')
  CopyWithReplacement(src, dest, {'x86_64': 'i686', '64': '32'})

  # Create NaClARM
  arm_replacements = {
      'x86_64': 'arm',
      '64': 'arm',
      'win_x86': 'win_arm'
  }

  dest = join(STAGING_DIR, 'NaClARM')
  CopyWithReplacement(src, dest, arm_replacements)

  # Create PNaCl
  pnacl_replacements = {
      'NaCl64': 'PNaCl',
      'x86_64': 'pnacl',
      '64': '32',
      '.nexe': '.pexe',
      'nacl_link.xml': 'pnacl_link.xml',
  }

  dest = join(STAGING_DIR, 'PNaCl')
  CopyWithReplacement(src, dest, pnacl_replacements)

  # Create archive
  archive = tarfile.open(OUTPUT_NAME, 'w:gz')
  AddFolderToArchive(STAGING_DIR, archive)
  archive.close()
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
