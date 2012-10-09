# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file

"""Entry point for the AddIn build bot.

Perform build steps and output results using the buildbot
annootator syntax
"""

import os
import sys
import subprocess
import urllib2
import zipfile

GSURL = 'https://commondatastorage.googleapis.com'
GSPATH = 'nativeclient-mirror/nacl/nacl_sdk/sdk'
SDKROOT = os.path.join('..', '..', 'out', 'sdk')


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
  Log('@@@BUILD_STEP build AddIn@@@')
  RunCommand('build.bat')


def StepInstall():
  Log('@@@BUILD_STEP Install AddIn@@@')
  RunCommand('developer_deploy.bat')


def StepInstallSDK():
  Log('@@@BUILD_STEP Install SDK@@@')
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

  RunCommand([naclsdk, 'update', '--force', 'pepper_23'])
  RunCommand([naclsdk, 'update', '--force', 'pepper_canary'])


def StepTest():
  Log('@@@BUILD_STEP Testing AddIn@@@')
  # Don't actually test yet
  env = dict(os.environ)
  sdkroot = os.path.abspath(os.path.join(SDKROOT, 'nacl_sdk'))
  env['NACL_SDK_ROOT'] = os.path.join(sdkroot, 'pepper_23')
  RunCommand('test.bat', env)
  env['NACL_SDK_ROOT'] = os.path.join(sdkroot, 'pepper_canary')
  RunCommand('test.bat', env)


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
  Log('@@@BUILD_STEP Archiving %s@@@' % rev)
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
  StepBuild()
  StepInstall()
  StepInstallSDK()
  StepTest()
  StepArchive()


if __name__ == '__main__':
  main()
