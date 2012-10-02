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

GSURL = 'https://commondatastorage.googleapis.com/'
GSPATH = 'nativeclient-mirror/nacl/nacl_sdk/sdk'

def Log(msg):
  sys.stdout.write(msg + '\n')
  sys.stdout.flush()


def RunCommand(cmd):
  Log("Running: %s" % cmd)
  Log("CWD: %s" % os.getcwd())
  rtn = subprocess.call(cmd, shell=True)
  if rtn:
    Log('@@@STEP_FAILURE@@@')
    sys.exit(1)


def StepBuild():
  Log('@@@BUILD_STEP build AddIn@@@')
  if sys.platform == 'cygwin':
    RunCommand(['./build.bat'])
  else:
    RunCommand(['build.bat'])


def StepTest():
  Log('@@@BUILD_STEP Testing AddIn@@@')
  # Don't actually test yet
  #RunCommand(['test.bat'])


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
  StepTest()
  StepArchive()


if __name__ == '__main__':
  main()
