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

  params = [os.path.join(parentdir, command)] + argv
  def Run(parameters):
    print '\nRunning ', parameters
    sys.stdout.flush()
    subprocess.check_call(' '.join(parameters), shell=True, cwd=parentdir)

  Run(params + ['-c'])
  Run(params + ['bot'])
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
