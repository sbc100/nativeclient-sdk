#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Copies necessary add-in files into place to install the add-in.

This script will copy the necessary files for the Visual Studio add-in
to where Visual Studio can find them. It assumes the current directory
contains the necessary files to copy.
"""

import create_ppapi_platform
import ctypes
import os
import optparse
import platform
import shutil
import sys

NACL_PLATFORM_NAME = 'NaCl'
PEPPER_PLATFORM_NAME = 'PPAPI'

DEFAULT_VS_USER_DIRECTORY = os.path.expandvars(
    '%USERPROFILE%\\My Documents\\Visual Studio 2010')

DEFAULT_MS_BUILD_DIRECTORY = os.path.expandvars('%ProgramFiles(x86)%\\MSBuild')

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

ADDIN_FILES = ['NativeClientVSAddIn.AddIn', 'NativeClientVSAddIn.dll']

class InstallError(Exception):
  """Error class for this installer indicating a fatal but expected error."""
  pass

def UninstallDirectory(directory):
  if os.path.exists(directory):
    shutil.rmtree(directory)
    print 'Removed: %s' % (directory)
  else:
    print 'Failed to remove non-existant directory: %s' % (directory)


def UninstallFile(file_path):
  if os.path.exists(file_path):
    os.remove(file_path)
    print 'Removed: %s' % (file_path)
  else:
    print 'Failed to remove non-existant file: %s' % (file_path)


def Uninstall(nacl_directory, pepper_directory, addin_directory):
  UninstallDirectory(nacl_directory)
  UninstallDirectory(pepper_directory)
  for file_name in ADDIN_FILES:
    UninstallFile(os.path.join(addin_directory, file_name))


def main():
  parser = optparse.OptionParser(usage='Usage: %prog [options]')
  parser.add_option('-b', '--msbuild-path', dest='msbuild_path',
      default=DEFAULT_MS_BUILD_DIRECTORY, metavar='PATH',
      help='Provide the path to the MSBuild directory')
  parser.add_option('-a', '--vsuser-path', dest='vsuser_path',
      default=DEFAULT_VS_USER_DIRECTORY, metavar='PATH',
      help='Provide the path to the Visual Studio user directory')
  parser.add_option('-f', '--force', action="store_true", dest='overwrite',
      default=False, help='Force an overwrite of existing files')
  parser.add_option('-p', '--ppapi', action="store_true", dest='install_ppapi',
      help='Install PPAPI template without asking.')
  parser.add_option('-n', '--no-ppapi', action="store_false",
      dest='install_ppapi', help='Do not install PPAPI template and do not ask')
  parser.add_option('-u', '--uninstall', action="store_true",
      dest='uninstall', help='Remove the add-in.')
  (options, args) = parser.parse_args()

  print "*************************************************"
  print "Native-Client Visual Studio 2010 Add-in Installer"
  print "*************************************************\n"
  print "Please ensure Visual Studio and MSBuild are closed " \
        "during installation.\n"

  if platform.system() != 'Windows':
    raise InstallError('Must install to Windows system')

  if sys.version_info < (2, 6, 2):
    print "\n\nWARNING: Only python version 2.6.2 or greater is supported. " \
          "Current version is %s\n\n" % (sys.version_info[:3],)

  # Admin is needed to write to the default platform directory.
  if ctypes.windll.shell32.IsUserAnAdmin() != 1:
    raise InstallError("Not running as administrator. The install script needs "
                       "write access to protected Visual Studio directories.")

  # Ensure install directories exist.
  if not os.path.exists(options.vsuser_path):
    raise InstallError("Could not find user Visual Studio directory: %s" % (
        options.vsuser_path))
  if not os.path.exists(options.msbuild_path):
    raise InstallError("Could not find MS Build directory: %s" % (
        options.msbuild_path))

  addin_directory = os.path.join(options.vsuser_path, 'Addins')
  platform_directory = os.path.join(
      options.msbuild_path, 'Microsoft.Cpp\\v4.0\\Platforms')
  nacl_directory = os.path.join(platform_directory, NACL_PLATFORM_NAME)
  pepper_directory = os.path.join(platform_directory, PEPPER_PLATFORM_NAME)

  # If uninstalling then redirect to uninstall program.
  if options.uninstall:
    Uninstall(nacl_directory, pepper_directory, addin_directory)
    print "\nUninstall complete!\n"
    exit(0)

  if not os.path.exists(platform_directory):
    raise InstallError("Could not find path: %s" % platform_directory)
  if not os.path.exists(addin_directory):
    os.mkdir(addin_directory)

  # Ensure environment variables are set.
  nacl_sdk_root = os.getenv('NACL_SDK_ROOT', None)
  chrome_path = os.getenv('CHROME_PATH', None)
  if nacl_sdk_root is None:
    raise InstallError('Environment Variable NACL_SDK_ROOT is not set')
  if chrome_path is None:
    raise InstallError('Environment Variable CHROME_PATH is not set')

  # Remove existing installation.
  if os.path.exists(nacl_directory) or os.path.exists(pepper_directory):
    # If not forced then ask user permission.
    if not options.overwrite:
      print "\nWarning: Pre-existing add-in installation will be overwritten."
      print "Continue? ((Yes))/((No))"
      remove_answer = raw_input().strip()
      if not (remove_answer.lower() == "yes" or remove_answer.lower() == "y"):
        raise InstallError('User did not allow overwrite of existing install.')
    print "Removing existing install..."
    Uninstall(nacl_directory, pepper_directory, addin_directory)

  # Ask user before installing PPAPI template.
  if options.install_ppapi is None:
    print "\n"
    print "Set up configuration to enable Pepper development " \
          "with Visual Studio?"
    print "((Yes)) - I want to create and copy relevant files into a " \
           "Pepper subdirectory"
    print "((No)) - I am not interested or will set up the configuration later"
    ppapi_answer = raw_input().strip()
    if ppapi_answer.lower() == "yes" or ppapi_answer.lower() == "y":
      options.install_ppapi = True
      print "Confirmed installer will include PPAPI platform."
    else:
      options.install_ppapi = False
      print "Will not install PPAPI platform during installation."

  print "\nBegin installing components..."

  try:
    # Copy the necessary files into place.
    for file_name in ADDIN_FILES:
      shutil.copy(os.path.join(SCRIPT_DIR, file_name), addin_directory)
    print "Add-in installed."

    shutil.copytree(os.path.join(SCRIPT_DIR, 'NaCl'), nacl_directory)
    print "NaCl platform installed."

    if options.install_ppapi:
      create_ppapi_platform.CreatePPAPI(options.msbuild_path)
      print "PPAPI platform installed."
  except:
    print "\nException occured! Rolling back install...\n"
    Uninstall(nacl_directory, pepper_directory, addin_directory)
    raise
  else:
    print "\nInstallation complete!\n"

if __name__ == '__main__':
  try:
    main()
  except InstallError as e:
    print
    print e
  except shutil.Error as e:
    print "Error while copying file. Please ensure file is not in use."
    print e
  except WindowsError as e:
    if e.winerror == 5:
      print "Access denied error.  Please ensure Visual Studio and MSBuild"
      print "processes are closed."
    else:
      raise

