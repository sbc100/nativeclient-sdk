#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Entry point for both build and try bots'''

import os
import subprocess
import sys


def main(argv):
  parentdir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
  if sys.platform in ['cygwin', 'win32']:
    # Windows build
    command = 'scons.bat'
  else:
    # Linux and Mac build
    command = 'scons'

  params = [os.path.join(parentdir, command), 'bot'] + argv
  print 'Running ', params
  sys.stdout.flush()
  return subprocess.call(' '.join(params), shell=True, cwd=parentdir)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
