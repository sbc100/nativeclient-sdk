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

"""Build NaCl tools (e.g. sel_ldr and ncval) at a given revision."""

import optparse
import os
import shutil
import subprocess
import sys
import tempfile


def MakeCheckoutDirs(options):
  # Pick work directory.
  options.work_dir = os.path.join('packages', 'native_client');
  if not os.path.exists(options.work_dir):
    os.mkdir(options.work_dir)
  # Go into working area.
  options.old_cwd = os.getcwd()
  os.chdir(options.work_dir)

def MakeInstallDirs(options, tools):
  # Make sure the toolchain directories exist
  toolchain_bin = os.path.join(options.toolchain, 'bin')
  if not os.path.exists(toolchain_bin):
    os.makedirs(toolchain_bin)


def Checkout(options):
  if not os.path.exists(".gclient"):
    # Setup client spec.
    p = subprocess.Popen(
        'gclient config '
        'http://src.chromium.org/native_client/trunk/src/native_client',
        shell=True)
    assert p.wait() == 0
  else:
    print(".gclient found, using existing configuration.")
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
  # Pick bit size prefix.
  if sys.platform == 'win32':
    bits32 = 'vcvarsall.bat x86 && '
    bits64 = 'vcvarsall.bat x86_amd64 && '
  else:
    bits32 = ''
    bits64 = ''
  # Build 32- and 64-bit sel_ldr.
  p = subprocess.Popen(
      '%s%s --mode=%s platform=x86-32 naclsdk_validate=0 sdl=none sel_ldr' % (
      bits32, scons, options.variant), shell=True)
  assert p.wait() == 0
  p = subprocess.Popen(
      '%s%s --mode=%s platform=x86-64 naclsdk_validate=0 sdl=none sel_ldr' % (
      bits64, scons, options.variant), shell=True)
  assert p.wait() == 0
  # Build 32- and 64-bit ncval.
  p = subprocess.Popen(
      '%s%s --mode=%s platform=x86-32 naclsdk_validate=0 ncval' % (
      bits32, scons, options.variant), shell=True)
  assert p.wait() == 0
  p = subprocess.Popen(
      '%s%s --mode=%s platform=x86-64 naclsdk_validate=0 ncval' % (
      bits64, scons, options.variant), shell=True)
  assert p.wait() == 0
  # Leave it.
  os.chdir('..')


def Install(options, tools):
  # Figure out where tools are and install the build artifacts into the
  # SDK tarball staging area.
  # TODO(bradnelson): add an 'install' alias to the main build for this.
  tool_build_path_32 = os.path.join('native_client',
                                    'scons-out',
                                    '%s-x86-32' % (options.variant),
                                    'staging')
  tool_build_path_64 = os.path.join('native_client',
                                    'scons-out',
                                    '%s-x86-64' % (options.variant),
                                    'staging')

  for nacl_tool in tools:
    shutil.copy(os.path.join(tool_build_path_32,
                             '%s%s' % (nacl_tool, options.exe_suffix)),
                os.path.join(options.toolchain,
                             'bin',
                             'nacl-%s%s' % (nacl_tool, options.exe_suffix)))
    if options.variant != 'opt-win':
    # TODO(dspringer): remove this check when 64-bit artifacts build on windows.
    # (see http://code.google.com/p/nativeclient/issues/detail?id=1228).
      shutil.copy(os.path.join(tool_build_path_64,
                               '%s%s' % (nacl_tool, options.exe_suffix)),
                  os.path.join(options.toolchain,
                               'bin',
                               'nacl64-%s%s' % (nacl_tool, options.exe_suffix)))

def BuildNaClTools(options):
  MakeCheckoutDirs(options)
  MakeInstallDirs(options, ['sel_ldr', 'ncval'])
  Checkout(options)
  Build(options)
  Install(options, ['sel_ldr', 'ncval'])
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
      '-t', '--toolchain', dest='toolchain',
      default='toolchain',
      help='where to put the NaCl tool binaries')
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    print 'ERROR: invalid argument'
    sys.exit(1)

  options.toolchain = os.path.abspath(options.toolchain)
  options.exe_suffix = exe_suffix
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

  return BuildNaClTools(options)


if __name__ == '__main__':
  main(sys.argv[1:])
