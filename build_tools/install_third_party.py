#!/usr/bin/python
# Copyright (c) 2010 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.

"""Build and install all the third-party tools and libraries required to build
the SDK code.  To add a script, add it to the array |THIRD_PARTY_SCRIPTS|.
Before running the scripts, a couple of environment variables get set:
  PYTHONPATH - append this script's dir to the search path for module import.
  NACL_SDK_ROOT - forced to point to the root of this repo.
"""

import os
import subprocess
import sys

THIRD_PARTY_SCRIPTS = [
    os.path.join('install_gtest', 'install_gtest.py'),
    os.path.join('install_boost', 'install_boost.py'),
]


def main(argv):
  script_dir = os.path.abspath(os.path.dirname(__file__))
  os.putenv('PYTHONPATH', '%s:%s' %(os.getenv('PYTHONPATH'), script_dir))
  # Force NACL_SDK_ROOT to point to the toolchain in this repo.
  (nacl_sdk_root, _) = os.path.split(script_dir)
  os.putenv('NACL_SDK_ROOT', nacl_sdk_root)
  for script in THIRD_PARTY_SCRIPTS:
    print "Running install script: %s" % os.path.join(script_dir, script)
    py_command = ['%s %s' % (sys.executable, os.path.join(script_dir, script))]
    p = subprocess.Popen(py_command + argv, shell=True)
    assert p.wait() == 0


if __name__ == '__main__':
  main(sys.argv[1:])
