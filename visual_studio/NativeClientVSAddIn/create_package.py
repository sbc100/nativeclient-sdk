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
import zipfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Root output directory.
BUILD_OUTPUT_DIRECTORY = os.path.join(
    SCRIPT_DIR,
    "../../out/NativeClientVSAddIn/")

# Directory containing static installer resources.
RESOURCE_DIRECTORY = os.path.join(SCRIPT_DIR, "InstallerResources/")

# Directory that contains the build assemblies.
ASSEMBLY_DIRECTORY = os.path.join(BUILD_OUTPUT_DIRECTORY, "Debug")

# Base name of the final zip file.
OUTPUT_NAME = os.path.join(BUILD_OUTPUT_DIRECTORY, "NativeClientVSAddIn.zip")

# List of source/destination pairs to include in zip file.
FILE_LIST = [
  (os.path.join(ASSEMBLY_DIRECTORY, "NativeClientVSAddIn.dll"), ''),
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
      zip_based_dir = dir_path[len(path):]
      write_path = os.path.join(zip_based_dir, file)
      zip_file.write(read_path, write_path, zipfile.ZIP_DEFLATED)


def main():
  # Zip the package.
  out_file = zipfile.ZipFile(OUTPUT_NAME, 'w')
  for source_dest in FILE_LIST:
    file_name = os.path.basename(source_dest[0])
    dest = os.path.join(source_dest[1], file_name)
    out_file.write(source_dest[0], dest, zipfile.ZIP_DEFLATED)
  AddFolderToZip(RESOURCE_DIRECTORY, out_file)
  out_file.close()


if __name__ == '__main__':
  main()
