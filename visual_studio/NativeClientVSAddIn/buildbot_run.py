#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Entry point for the AddIn build bot.

Perform build steps and output results using the buildbot
annootator syntax
"""

import os
import sys
import re
import shutil
import subprocess
import urllib2
import zipfile

GSURL = 'https://commondatastorage.googleapis.com'
GSPATH = 'nativeclient-mirror/nacl/nacl_sdk/sdk'
SDKROOT = os.path.join('..', '..', 'out', 'sdk')
SDK_VERSIONS = ['pepper_31', 'pepper_32', 'pepper_canary']


def Log(msg):
  sys.stdout.write(msg + '\n')
  sys.stdout.flush()


def RunCommand(cmd, env=None):
  Log("Running: %s" % cmd)
  Log("CWD: %s" % os.getcwd())
  if type(cmd) == str:
    cmd = cmd.split()

  if sys.platform == 'cygwin':
    # allow bat files in the current working directory to
    # be executed on cygwin as they are on win32
    if not os.path.dirname(cmd[0]) and os.path.exists(cmd[0]):
      cmd[0] = './' + cmd[0]

  rtn = subprocess.call(cmd, env=env)
  if rtn:
    Log("Command returned non-zero exit code: %s" % rtn)
    Log('@@@STEP_FAILURE@@@')
    sys.exit(1)


def StepBuild():
  Log('@@@BUILD_STEP build addin@@@')

  rev = os.environ.get('BUILDBOT_GOT_REVISION')
  if not rev:
    Log('No BUILDBOT_GOT_REVISION found in environ')
    Log('@@@STEP_FAILURE@@@')
    sys.exit(1)

  if rev[0] == 'r':
    rev = rev[1:]

  # make a backup of AssemblyInfo.cs before we modify it
  filename = os.path.join('NativeClientVSAddIn', 'AssemblyInfo.cs')
  backup = filename + '.orig'
  shutil.copyfile(filename, backup)

  try:
    # Before we do the build, insert the revsion information
    # info AssemblyInfo.cs.  Thie will then be reported as
    # the addin version in visual studio.
    with open(filename, 'rb') as f:
      contents = f.read()

    pattern = r'(\[assembly: AssemblyInformationalVersion\("\d+\.\d+\.).*"\)\]'
    contents = re.sub(pattern, r'\g<1>%s")]' % rev, contents)

    with open(filename, 'wb') as f:
      f.write(contents)

    RunCommand('build.bat')
  finally:
    # Once build is done restore original file
    os.remove(filename)
    os.rename(backup, filename)



def StepInstall():
  Log('@@@BUILD_STEP install addin@@@')
  RunCommand('developer_deploy.bat')


def StepInstallSDK():
  Log('@@@BUILD_STEP install sdk@@@')
  naclsdk = os.path.join(SDKROOT, 'nacl_sdk', 'naclsdk.bat')
  if not os.path.exists(naclsdk):
    if not os.path.exists(SDKROOT):
      os.makedirs(SDKROOT)
    filename = os.path.join(SDKROOT, 'nacl_sdk.zip')
    url = GSURL + "/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip"
    contents = urllib2.urlopen(url).read()
    with open(filename, 'wb') as zfileout:
      zfileout.write(contents)
    zfile = zipfile.ZipFile(filename)
    zfile.extractall(SDKROOT)

  for version in SDK_VERSIONS:
    RunCommand([naclsdk, 'update', '--force', version])


def StepTest():
  # Don't actually test yet
  env = dict(os.environ)
  sdkroot = os.path.abspath(os.path.join(SDKROOT, 'nacl_sdk'))
  if 'CHROME_PATH' not in os.environ:
    # TODO(sbc): Addin itself should autodetect chrome location
    # http://crbug.com/154911
    progfiles = os.environ.get('PROGRAMFILES')
    progfiles = os.environ.get('PROGRAMFILES(X86)', progfiles)
    chrome = os.path.join(progfiles, 'Google', 'Chrome', 'Application',
                          'chrome.exe')
    if not os.path.exists(chrome):
      Log('CHROME_PATH not found')
      Log('chrome not found in the default location: %s' % chrome)
      Log('@@@STEP_FAILURE@@@')
      sys.exit(1)
    env['CHROME_PATH'] = chrome
  for version in SDK_VERSIONS:
    Log('@@@BUILD_STEP test against %s@@@' % version)
    env['NACL_SDK_ROOT'] = os.path.join(sdkroot, version)
    RunCommand('test.bat', env)
    RunCommand('test_2012.bat', env)


def _FindInPath(filename):
  for path in os.environ['PATH'].split(os.pathsep):
    result = os.path.join(path, filename)
    if os.path.exists(result):
      return result

  Log('%s not found in PATH' % filename)
  Log('@@@STEP_FAILURE@@@')
  sys.exit(1)


def _GetGsutil():
  if os.environ.get('BUILDBOT_BUILDERNAME'):
    # When running in a buildbot slave use
    # gsutil from the slave scripts folder
    import slave
    slave_dir = os.path.dirname(slave.__file__)
    gsutil = os.path.join(slave_dir, 'gsutil')
    if os.name == 'nt':
      gsutil += '.bat'
    gsutil = [gsutil]
  else:
    if os.name == 'nt':
      gsutil = [sys.executable, _FindInPath('gsutil')]
    else:
      gsutil = ['gsutil']

  return gsutil


def StepArchive():
  rev = os.environ.get('BUILDBOT_GOT_REVISION')
  if not rev:
    Log('No BUILDBOT_GOT_REVISION found in environ')
    Log('@@@STEP_FAILURE@@@')
    sys.exit(1)
  Log('@@@BUILD_STEP archive build [r%s]@@@' % rev)
  basename = 'vs_addin.tgz'
  remote_name = '%s/%s/%s' % (GSPATH, rev, basename)
  local_filename = os.path.join('..', '..', 'out',
                                'vs_addin', basename)
  cmd = _GetGsutil()
  cmd += ['cp', '-a', 'public-read', local_filename,
          'gs://' + remote_name]
  RunCommand(cmd)
  url = "%s/%s" % (GSURL, remote_name)
  Log('@@@STEP_LINK@download@%s@@@' % url)


def main():
  # Remove BOTO_CONFIG from the environment -- we want to use the NaCl .boto
  # file that has access to gs://nativeclient-mirror.
  if 'BOTO_CONFIG' in os.environ:
    del os.environ['BOTO_CONFIG']
  if 'AWS_CREDENTIAL_FILE' in os.environ:
    del os.environ['AWS_CREDENTIAL_FILE']
  StepBuild()
  StepInstall()
  StepInstallSDK()
  StepTest()
  StepArchive()


if __name__ == '__main__':
  main()
