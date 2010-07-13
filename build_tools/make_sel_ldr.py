#!/usr/bin/python
# Copyright 2010, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Build sel_ldr at a given revision."""

import optparse
import os
import shutil
import subprocess
import sys
import tempfile


def WorkingArea(options):
  # Pick work directory.
  options.work_dir = tempfile.mkdtemp(prefix='sel_ldr')
  # Go into working area.
  options.old_cwd = os.getcwd()
  os.chdir(options.work_dir)


def Checkout(options):
  # Setup client spec.
  p = subprocess.Popen(
      'gclient config '
      'http://nativeclient.googlecode.com/svn/trunk/src/native_client',
      shell=True)
  assert p.wait() == 0
  # Sync at the desired revision.
  p = subprocess.Popen(
      'gclient sync --rev %s' % options.revision, shell=True)
  assert p.wait() == 0


def Build(options):
  # Enter native_client.
  os.chdir('native_client')
  # Pick scons.
  if sys.platform == 'win32':
    scons = 'scons.bat'
  else:
    scons = './scons'
  # Pick variant.
  if sys.platform in ['win32', 'cygwin']:
    variant = 'opt-win'
  elif sys.platform == 'darwin':
    variant = 'opt-mac'
  elif sys.platform in ['linux', 'linux2']:
    variant = 'opt-linux'
  else:
    assert False
  options.variant = variant
  p = subprocess.Popen(
      '%s --mode=%s platform=x86-32 naclsdk_validate=0 sel_ldr' % (
          scons, variant), shell=True)
  assert p.wait() == 0
  # Leave it.
  os.chdir('..')


def Install(options):
  # Figure out where sel_ldr is.
  # TODO(bradnelson): add an 'install' alias to the main build for this.
  shutil.copy('native_client/scons-out/%s-x86-32/staging/sel_ldr%s' % (
      options.variant, options.exe_suffix), options.target)


def Cleanup(options):
  shutil.rmtree(options.work_dir, ignore_errors=True)


def BuildSelLdr(options):
  WorkingArea(options)
  Checkout(options)
  Build(options)
  Install(options)
  Cleanup(options)
  return 0


def main(argv):
  if sys.platform in ['win32', 'cygwin']:
    exe_suffix = '.exe'
  else:
    exe_suffix = ''

  parser = optparse.OptionParser()
  parser.add_option(
      '-r', '--revision', dest='revision',
      default='HEAD',
      help='which revision of native_client to sync')
  parser.add_option(
      '-t', '--target', dest='target',
      default='sel_ldr' + exe_suffix,
      help='which revision of native_client to sync')
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    print 'ERROR: invalid argument'
    sys.exit(1)

  options.target = os.path.abspath(options.target)
  options.exe_suffix = exe_suffix

  return BuildSelLdr(options)


if __name__ == '__main__':
  main(sys.argv[1:])
