#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Takes the output of the build step and zips the distributable package.

This script assumes the build script has been run to compile the add-in.
It zips up all files required for the add-in installation and places the
result in out/NativeClientVSAddin.zip
"""

import os
import re
import fileinput
import win32api
import shutil
import zipfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Root output directory.
BUILD_OUTPUT_DIRECTORY = os.path.join(
    SCRIPT_DIR,
    "../../out/NativeClientVSAddIn")

# Directory that contains the build assemblies.
ASSEMBLY_DIRECTORY = os.path.join(BUILD_OUTPUT_DIRECTORY, "Debug")

# Directory containing static installer resources.
RESOURCE_DIRECTORY = os.path.join(SCRIPT_DIR, "InstallerResources")

# Base name of the final zip file.
OUTPUT_NAME = os.path.join(BUILD_OUTPUT_DIRECTORY, "NativeClientVSAddIn.zip")

# AddIn metadata file path. We will modify this with the version #.
ADDIN_METADATA = os.path.join(RESOURCE_DIRECTORY, "NativeClientVSAddIn.AddIn")

# AddIn dll file path. We will obtain our add-in version from this.
ADDIN_ASSEMBLY = os.path.join(ASSEMBLY_DIRECTORY, "NativeClientVSAddIn.dll")

# Regex list to exclude from the zip. If a file path matches any of the
# expressions during a call to AddFolderToZip it is excluded from the zip file.
EXCLUDES = [
    '.*\.svn.*', # Exclude .svn directories.
    # Exclude .AddIn file for now since we need to modify it with version info.
    re.escape(ADDIN_METADATA)]

# List of source/destination pairs to include in zip file.
FILE_LIST = [
  (ADDIN_ASSEMBLY, ''),
  (os.path.join(ASSEMBLY_DIRECTORY, "NaCl.Build.CPPTasks.dll"), 'NaCl')]


def AddFolderToZip(path, zip_file):
  """Adds an entire folder and sub folders to an open zipfile object.

  The zip_file must already be open and it is not closed by this function.

  Args:
    path: Folder to add.
    zipfile: Already open zip file.

  Returns:
    Nothing.
  """
  # Ensure the path ends in trailing slash.
  path = path.rstrip("/\\") + "\\"
  for dir_path, dir_names, files in os.walk(path):
    for file in files:
      read_path = os.path.join(dir_path, file)

      # If the file path matches an exclude, don't include it.
      if any(re.search(expr, read_path) is not None for expr in EXCLUDES):
        continue

      zip_based_dir = dir_path[len(path):]
      write_path = os.path.join(zip_based_dir, file)
      zip_file.write(read_path, write_path, zipfile.ZIP_DEFLATED)


def AddVersionModifiedAddinFile(zip_file):
  """Modifies the .AddIn file with the build version and adds to the zip.

  The version number is obtained from the NativeClientAddIn.dll assembly which
  is built during the build process.

  Args:
  zip_file: Already open zip file.
  """
  info = win32api.GetFileVersionInfo(ADDIN_ASSEMBLY, "\\")
  ms = info['FileVersionMS']
  ls = info['FileVersionLS']
  version = "[%i.%i.%i.%i]" % (
      win32api.HIWORD(ms), win32api.LOWORD(ms),
      win32api.HIWORD(ls), win32api.LOWORD(ls))
  print "\nNaCl VS Add-in Build version: %s\n" % (version)

  metadata_filename = os.path.basename(ADDIN_METADATA)
  modified_file = os.path.join(ASSEMBLY_DIRECTORY, metadata_filename)

  # Copy the metadata file to new location and modify the version info.
  with open(ADDIN_METADATA, 'r') as source_file:
    with open(modified_file, 'w') as dest_file:
      for line in source_file:
        dest_file.write(line.replace("[REPLACE_ADDIN_VERSION]", version))

  zip_file.write(modified_file, metadata_filename, zipfile.ZIP_DEFLATED)


def main():
  # Zip the package.
  out_file = zipfile.ZipFile(OUTPUT_NAME, 'w')
  for source_dest in FILE_LIST:
    file_name = os.path.basename(source_dest[0])
    dest = os.path.join(source_dest[1], file_name)
    out_file.write(source_dest[0], dest, zipfile.ZIP_DEFLATED)
  AddFolderToZip(RESOURCE_DIRECTORY, out_file)
  AddVersionModifiedAddinFile(out_file)
  out_file.close()


if __name__ == '__main__':
  main()
