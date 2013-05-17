#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Entry point for the Game build bot.

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
SDKROOT = os.path.abspath(os.path.join('out', 'sdk'))


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


def StepRunTests():
  Log('@@@BUILD_STEP Run tests@@@')
  RunCommand(['make', 'test'])
  RunCommand(['make', 'validate'])


def StepInstallSDK():
  Log('@@@BUILD_STEP Install SDK@@@')
  naclsdk = os.path.join(SDKROOT, 'nacl_sdk', 'naclsdk')
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

  os.chmod(naclsdk, 0750)
  RunCommand([naclsdk, 'update', '--force', 'pepper_canary'])


def StepBuild():
  env = dict(os.environ)
  env['NACL_SDK_ROOT'] = os.path.join(SDKROOT, 'nacl_sdk', 'pepper_canary')
  RunCommand(['make', '-j8'], env)


def main():
  StepRunTests()
  StepInstallSDK()
  StepBuild()
  return 0


if __name__ == '__main__':
  sys.exit(main())
