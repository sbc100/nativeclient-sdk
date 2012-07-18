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

# Root output directory
BUILD_OUTPUT_DIRECTORY = "../../out/NativeClientVSAddIn/"

# Directory containing static installer resources
RESOURCE_DIRECTORY = "./InstallerResources"

# Directory that contains the build assemblies
ASSEMBLY_DIRECTORY = os.path.join(BUILD_OUTPUT_DIRECTORY, "Debug")

# Base name of the final zip file
OUTPUT_NAME = os.path.join(BUILD_OUTPUT_DIRECTORY, "NativeClientVSAddIn.zip")

# List of paths to files to include in the zip file
FILE_LIST = [
  os.path.join(RESOURCE_DIRECTORY, 'NativeClientVSAddIn.AddIn'),
  os.path.join(RESOURCE_DIRECTORY, 'install.py'),
  os.path.join(ASSEMBLY_DIRECTORY, 'NativeClientVSAddIn.dll')]

def main():
  # Zip the package
  out_file = zipfile.ZipFile(OUTPUT_NAME, 'w')
  for file_path in FILE_LIST:
    out_file.write(file_path, os.path.basename(file_path), zipfile.ZIP_DEFLATED)
  out_file.close()

if __name__ == '__main__':
  main()
