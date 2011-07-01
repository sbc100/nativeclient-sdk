#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build NaCl tools (e.g. sel_ldr and ncval) at a given revision."""

import build_utils
import optparse
import os
import shutil
import subprocess
import sys
import tempfile

bot = build_utils.BotAnnotator()


def MakeInstallDirs(options):
  install_dir = os.path.join(options.toolchain, 'bin');
  if not os.path.exists(install_dir):
    os.makedirs(install_dir)


def Build(options):
  '''Build 32-bit and 64-bit versions of both sel_ldr and ncval'''
  nacl_dir = os.path.join(options.nacl_dir, 'native_client')
  if sys.platform == 'win32':
    scons = os.path.join(nacl_dir, 'scons.bat')
    bits32 = 'vcvarsall.bat x86 && '
    bits64 = 'vcvarsall.bat x86_amd64 && '
  else:
    scons = os.path.join(nacl_dir, 'scons')
    bits32 = ''
    bits64 = ''

  def BuildTools(prefix, bits, target):
    cmd = '%s%s -j %s --mode=%s platform=x86-%s naclsdk_validate=0 %s' % (
        prefix, scons, options.jobs, options.variant, bits, target)
    bot.Run(cmd, shell=True, cwd=nacl_dir)

  BuildTools(bits32, '32', 'sdl=none sel_ldr ncval')
  BuildTools(bits64, '64', 'sdl=none sel_ldr ncval')

  def BuildAndInstallLibsAndHeaders(bits):
    cmd = '%s install --mode=nacl libdir=%s includedir=%s platform=x86-%s' % (
        scons,
        os.path.join(options.toolchain,
                     'nacl64',
                     'lib32' if bits == 32 else 'lib'),
        os.path.join(options.toolchain, 'nacl64', 'include'),
        bits)
    bot.Run(cmd, shell=True, cwd=nacl_dir)

  BuildAndInstallLibsAndHeaders(32)
  BuildAndInstallLibsAndHeaders(64)


def Install(options, tools):
  # Figure out where tools are and install the build artifacts into the
  # SDK tarball staging area.
  # TODO(bradnelson): add an 'install' alias to the main build for this.
  nacl_dir = os.path.join(options.nacl_dir, 'native_client')
  tool_build_path_32 = os.path.join(nacl_dir,
                                    'scons-out',
                                    '%s-x86-32' % (options.variant),
                                    'staging')
  tool_build_path_64 = os.path.join(nacl_dir,
                                    'scons-out',
                                    '%s-x86-64' % (options.variant),
                                    'staging')

  for nacl_tool in tools:
    shutil.copy(os.path.join(tool_build_path_32,
                             '%s%s' % (nacl_tool, options.exe_suffix)),
                os.path.join(options.toolchain,
                             'bin',
                             'nacl-%s%s' % (nacl_tool, options.exe_suffix)))
    shutil.copy(os.path.join(tool_build_path_64,
                             '%s%s' % (nacl_tool, options.exe_suffix)),
                os.path.join(options.toolchain,
                             'bin',
                             'nacl64-%s%s' % (nacl_tool, options.exe_suffix)))


#Cleans up the checkout directories if -c was provided as a command line arg.
def CleanUpCheckoutDirs(options):
  if(options.cleanup):
    bot.Print('Removing scons-out')
    scons_out = os.path.join(options.nacl_dir, 'native_client', 'scons-out')
    if sys.platform != 'win32':
      shutil.rmtree(scons_out, ignore_errors=True)
    else:
      # Intentionally ignore return value since a directory might be in use.
      subprocess.call(['rmdir', '/Q', '/S', scons_out],
                      env=os.environ.copy(),
                      shell=True)


def BuildNaClTools(options):
  bot.BuildStep('build NaCl tools')
  CleanUpCheckoutDirs(options)
  MakeInstallDirs(options)
  Build(options)
  Install(options, ['sel_ldr', 'ncval'])
  return 0


def main(argv):
  if sys.platform in ['win32', 'cygwin']:
    exe_suffix = '.exe'
  else:
    exe_suffix = ''

  script_dir = os.path.abspath(os.path.dirname(__file__))

  parser = optparse.OptionParser()
  parser.add_option(
      '-t', '--toolchain', dest='toolchain',
      default='toolchain',
      help='where to put the NaCl tool binaries')
  parser.add_option(
      '-c', '--cleanup', action='store_true', dest='cleanup',
      default=False,
      help='whether to clean up the checkout files')
  parser.add_option(
      '-j', '--jobs', dest='jobs', default='1',
      help='Number of parallel jobs to use while building nacl tools')
  parser.add_option(
      '-n', '--nacl_dir', dest='nacl_dir',
      default=os.path.join(script_dir, 'packages', 'native_client'),
      help='Location of Native Client repository used for building tools')
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    bot.Print('ERROR: invalid argument(s): %s' % args)
    sys.exit(1)

  options.toolchain = os.path.abspath(options.toolchain)
  options.exe_suffix = exe_suffix
  # Pick variant.
  if sys.platform in ['win32', 'cygwin']:
    variant = 'dbg-win'
  elif sys.platform == 'darwin':
    variant = 'dbg-mac'
  elif sys.platform in ['linux', 'linux2']:
    variant = 'dbg-linux'
  else:
    assert False
  options.variant = variant

  return BuildNaClTools(options)


if __name__ == '__main__':
  main(sys.argv[1:])
