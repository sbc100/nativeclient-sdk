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
import time

NACL32_PLATFORM = 'NaCl32'
NACL64_PLATFORM = 'NaCl64'
NACLARM_PLATFORM = 'NaClARM'
PNACL_PLATFORM = 'PNaCl'
NACL_PLATFORM_OLD = 'NaCl'
PEPPER_PLATFORM = 'PPAPI'

DEFAULT_VS_USER_DIRECTORY = os.path.expandvars(
    '%USERPROFILE%\\My Documents\\Visual Studio 2010')

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

ADDIN_FILES = ['NativeClientVSAddIn.AddIn', 'NativeClientVSAddIn.dll']


class InstallError(Exception):
  """Error class for this installer indicating a fatal but expected error."""
  pass


def UninstallDirectory(directory):
  if os.path.exists(directory):
    shutil.rmtree(directory)
    print 'Removed: %s' % (directory)


def UninstallFile(file_path):
  if os.path.exists(file_path):
    if not os.access(file_path, os.W_OK):
      raise InstallError("File is marked as read-only: %s.\n"
                         "Please correct and try again." % file_path)
    os.remove(file_path)
    print 'Removed: %s' % (file_path)


def Uninstall(platform_dirs, addin_dir):
  for dirname in platform_dirs:
    UninstallDirectory(dirname)
  for file_name in ADDIN_FILES:
    UninstallFile(os.path.join(addin_dir, file_name))


def CheckForRunningProgams():
  tasklist = os.popen('tasklist.exe').readlines()
  tasklist = [l for l in tasklist if 'MSBuild' in l or 'devenv' in l]
  return tasklist


def Ask(question):
  while True:
    print '\n'
    print question
    print "Continue? ((Yes))/((No))"
    answer = raw_input().strip().lower()
    if answer in ('y', 'yes'):
      return True
    if answer in ('n', 'no'):
      return False


def main():
  parser = optparse.OptionParser(usage='Usage: %prog [options]')
  parser.add_option('-b', '--msbuild-path', dest='msbuild_path',
      metavar='PATH', help='Provide the path to the MSBuild directory')
  parser.add_option('-a', '--vsuser-path', dest='vsuser_path',
      default=DEFAULT_VS_USER_DIRECTORY, metavar='PATH',
      help='Provide the path to the Visual Studio user directory')
  parser.add_option('-f', '--force', action="store_true",
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

  if platform.system() != 'Windows':
    raise InstallError('Must install to Windows system')

  if not options.msbuild_path:
    # Find the x86 program files folder.  If we are uing a 64-bit version of
    # python.exe then ProgramFiles(x86).  If we using a 32-bit python then
    # ProgramFiles will always be set to point the x86 program files even
    # under W0W64.
    if 'ProgramFiles(x86)' in os.environ:
      options.msbuild_path = os.path.expandvars('%ProgramFiles(x86)%\\MSBuild')
    else:
      options.msbuild_path = os.path.expandvars('%ProgramFiles%\\MSBuild')


  if CheckForRunningProgams():
    if not options.force:
      print "Visual Studio and MSBuild must be closed during installation"
      if not Ask("Kill all instances now?"):
        raise InstallError('Please close all Visual Studio or MSBuild '
                           'instances before installing')
    os.system("taskkill.exe /IM MSBuild.exe /f")
    os.system("taskkill.exe /IM devenv.exe")
    if CheckForRunningProgams():
      for i in xrange(10):
        if not CheckForRunningProgams():
          break
        time.sleep(1)
        if not CheckForRunningProgams():
          break
        if i == 5:
          print "Trying taskkill with /f"
          os.system("taskkill.exe /IM devenv.exe /f")
      if CheckForRunningProgams():
        raise InstallError('Failed to kill Visual Studio and MSBuild instances')

  if sys.version_info < (2, 6, 2):
    print "\n\nWARNING: Only python version 2.6.2 or greater is supported. " \
          "Current version is %s\n\n" % (sys.version_info[:3],)

  # Ensure install directories exist.
  if not os.path.exists(options.vsuser_path):
    raise InstallError("Could not find user Visual Studio directory: %s" % (
        options.vsuser_path))
  if not os.path.exists(options.msbuild_path):
    raise InstallError("Could not find MS Build directory: %s" % (
        options.msbuild_path))

  addin_dir = os.path.join(options.vsuser_path, 'Addins')
  platform_root = os.path.join(options.msbuild_path,
                               'Microsoft.Cpp', 'v4.0', 'Platforms')
  if not os.path.exists(platform_root):
    raise InstallError("Could not find path: %s" % platform_root)

  if not os.access(platform_root, os.W_OK):
    # Admin is needed to write to the default platform directory.
    if ctypes.windll.shell32.IsUserAnAdmin() != 1:
      raise InstallError("Not running as administrator. The install script "
                         "needs write access to protected Visual Studio "
                         "directories.")
    raise InstallError("install script needs write access to: %s"
                       % platform_root)

  nacl_dir_32 = os.path.join(platform_root, NACL32_PLATFORM)
  nacl_dir_64 = os.path.join(platform_root, NACL64_PLATFORM)
  nacl_dir_arm = os.path.join(platform_root, NACLARM_PLATFORM)
  pnacl_dir = os.path.join(platform_root, PNACL_PLATFORM)
  nacl_dir_old = os.path.join(platform_root, NACL_PLATFORM_OLD)
  nacl_common = os.path.join(os.path.dirname(platform_root), 'NaCl')
  pepper_dir = os.path.join(platform_root, PEPPER_PLATFORM)
  remove_dirs = (nacl_dir_32, nacl_dir_64, nacl_dir_arm,
                 pnacl_dir, nacl_dir_old, pepper_dir,
                 nacl_common)
  # If uninstalling then redirect to uninstall program.
  if options.uninstall:
    Uninstall(remove_dirs, addin_dir)
    print "\nUninstall complete!\n"
    sys.exit(0)

  if not os.path.exists(platform_root):
    raise InstallError("Could not find path: %s" % platform_root)
  if not os.path.exists(addin_dir):
    os.makedirs(addin_dir)

  # Ensure environment variables are set.
  if not options.force:
    nacl_sdk_root = os.getenv('NACL_SDK_ROOT', None)
    chrome_path = os.getenv('CHROME_PATH', None)
    if nacl_sdk_root is None:
      raise InstallError('Environment Variable NACL_SDK_ROOT is not set')
    if chrome_path is None:
      raise InstallError('Environment Variable CHROME_PATH is not set')

  # Remove existing installation.
  if any(os.path.exists(d) for d in remove_dirs):
    # If not forced then ask user permission.
    if not options.force:
      if not Ask("Warning: Pre-existing add-in installation "
                 "will be overwritten."):
        raise InstallError('User did not allow overwrite of existing install.')
    print "Removing existing install..."
    Uninstall(remove_dirs, addin_dir)

  # Ask user before installing PPAPI template.
  if options.install_ppapi is None:
    ppapi_answer = Ask("Set up configuration to enable Pepper development "
          "with Visual Studio?\n"
          "((Yes)) - I want to create and copy relevant files into a "
          "Pepper subdirectory\n"
          "((No)) - I am not interested or will set up the configuration later")
    if ppapi_answer:
      options.install_ppapi = True
      print "Confirmed installer will include PPAPI platform."
    else:
      options.install_ppapi = False
      print "Will not install PPAPI platform during installation."

  print "\nBegin installing components..."

  try:
    # Copy the necessary files into place.
    for file_name in ADDIN_FILES:
      shutil.copy(os.path.join(SCRIPT_DIR, file_name), addin_dir)
    print "Add-in installed."

    shutil.copytree(os.path.join(SCRIPT_DIR, 'NaCl'), nacl_common)
    print "NaCl common resources installed."

    shutil.copytree(os.path.join(SCRIPT_DIR, NACL32_PLATFORM), nacl_dir_32)
    print "%s platform installed." % NACL32_PLATFORM

    shutil.copytree(os.path.join(SCRIPT_DIR, NACL64_PLATFORM), nacl_dir_64)
    print "%s platform installed." % NACL64_PLATFORM

    shutil.copytree(os.path.join(SCRIPT_DIR, NACLARM_PLATFORM), nacl_dir_arm)
    print "%s platform installed." % NACLARM_PLATFORM

    shutil.copytree(os.path.join(SCRIPT_DIR, PNACL_PLATFORM), pnacl_dir)
    print "PNaCl platform installed."

    if options.install_ppapi:
      create_ppapi_platform.CreatePPAPI(options.msbuild_path)
      print "PPAPI platform installed."
  except:
    print "\nException occured! Rolling back install...\n"
    Uninstall(remove_dirs, addin_dir)
    raise
  else:
    print "\nInstallation complete!\n"

  return 0

if __name__ == '__main__':
  rtn = 1
  try:
    rtn = main()
  except InstallError as e:
    print
    print e
  except shutil.Error as e:
    print "Error while copying file. Please ensure file is not in use."
    print e
  except WindowsError as e:
    if e.winerror == 5:
      print e
      print("The install script failed to write to the files mentioned above")
      if ctypes.windll.shell32.IsUserAnAdmin() != 1:
        print("Please try running as administrator.")
      else:
        print("Please check for any running programs that might "
              "have them locked.")
    else:
      raise
  sys.exit(rtn)
