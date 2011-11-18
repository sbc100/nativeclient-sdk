#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Entry point for both build and try bots'''

import os
import subprocess
import sys

# Add scons to the python path (as nacl_utils.py requires it).
PARENT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(os.path.join(PARENT_DIR, 'third_party/scons-2.0.1/engine'))

import build_utils


def Archive(revision, chrome_milestone):
  """Archive the sdk to google storage.

  Args:
    revision: SDK svn revision number.
    chrome_milestone: Chrome milestone (m14 etc), this is for.
  """
  if sys.platform in ['cygwin', 'win32']:
    src = 'nacl-sdk.exe'
    dst = 'naclsdk_win.exe'
  elif sys.platform in ['darwin']:
    src = 'nacl-sdk.tgz'
    dst = 'naclsdk_mac.tgz'
  else:
    src = 'nacl-sdk.tgz'
    dst = 'naclsdk_linux.tgz'
  bucket_path = 'nativeclient-mirror/nacl/nacl_sdk/pepper_%s_%s/%s' % (
      chrome_milestone, revision, dst)
  full_dst = 'gs://%s' % bucket_path
  subprocess.check_call(
      '/b/build/scripts/slave/gsutil cp -a public-read %s %s' % (
          src, full_dst), shell=True)
  url = 'https://commondatastorage.googleapis.com/%s' % bucket_path
  print '@@@STEP_LINK@download@%s@@@' % url
  sys.stdout.flush()


def main(argv):
  if sys.platform in ['cygwin', 'win32']:
    # Windows build
    command = 'scons.bat'
  else:
    # Linux and Mac build
    command = 'scons'

  parent_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
  params = [os.path.join(parent_dir, command)] + argv
  def Run(parameters):
    print '\nRunning ', parameters
    sys.stdout.flush()
    subprocess.check_call(' '.join(parameters), shell=True, cwd=parent_dir)

  print '@@@BUILD_STEP generate sdk@@@'
  sys.stdout.flush()

  Run(params + ['-c'])
  Run(params + ['bot'])

  # Archive on non-trybots.
  if '-sdk' in os.environ.get('BUILDBOT_BUILDERNAME', ''):
    print '@@@BUILD_STEP archive build@@@'
    sys.stdout.flush()
    Archive(revision=os.environ.get('BUILDBOT_GOT_REVISION'),
            chrome_milestone=build_utils.ChromeMilestone())

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
