#!/usr/bin/evn python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
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

  rev = os.environ.get('BUILDBOT_GOT_REVISION')
  if not rev:
    Log('No BUILDBOT_GOT_REVISION found in environ')
    Log('@@@STEP_FAILURE@@@')
    sys.exit(1)

  if rev[0] == 'r':
    rev = rev[1:]

  RunCommand('make -j8')
  return 0


def main():
  return StepBuild()


if __name__ == '__main__':
  sys.exit(main())
