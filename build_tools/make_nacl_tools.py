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


def MakeCheckoutDirs(options):
  script_dir = os.path.abspath(os.path.dirname(__file__))
  # Pick work directory.
  options.work_dir = os.path.join(script_dir, 'packages', 'native_client');
  if not os.path.exists(options.work_dir):
    os.makedirs(options.work_dir)

def MakeInstallDirs(options):
  install_dir = os.path.join(options.toolchain, 'bin');
  if not os.path.exists(install_dir):
    os.makedirs(install_dir)

def Checkout(options):
  os.chdir(options.work_dir)
  if not os.path.exists(".gclient"):
    # Setup client spec.
    subprocess.check_call(
        'gclient config '
        'http://src.chromium.org/native_client/trunk/src/native_client',
        shell=True)
  else:
    bot.Print(".gclient found, using existing configuration.")
  # Sync at the desired revision.
  subprocess.check_call(
      'gclient sync --rev %s' % options.revision, shell=True)


def Build(options):
  '''Build 32-bit and 64-bit versions of both sel_ldr and ncval'''
  if sys.platform == 'win32':
    scons = 'scons.bat'
    bits32 = 'vcvarsall.bat x86 &&'
    bits64 = 'vcvarsall.bat x86_amd64 &&'
  else:
    scons = './scons'
    bits32 = ''
    bits64 = ''

  def build_step(prefix, bits, target):
    cmd = '%s%s -j %s --mode=%s platform=x86-%s naclsdk_validate=0 %s' % (
        prefix, scons, options.jobs, options.variant, bits, target)
    bot.Print('Running "%s"' % cmd)
    subprocess.check_call(cmd, shell=True, cwd='native_client')

  build_step(bits32, '32', 'sdl=none sel_ldr')
  build_step(bits64, '64', 'sdl=none sel_ldr')
  build_step(bits32, '32', 'ncval')
  build_step(bits64, '64', 'ncval')


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
    shutil.copy(os.path.join(tool_build_path_64,
                             '%s%s' % (nacl_tool, options.exe_suffix)),
                os.path.join(options.toolchain,
                             'bin',
                             'nacl64-%s%s' % (nacl_tool, options.exe_suffix)))

#Cleans up the checkout directories if -c was provided as a command line arg.
def CleanUpCheckoutDirs(options):
  if(options.cleanup):
    if sys.platform != 'win32':
      shutil.rmtree(options.work_dir, ignore_errors=True)
    else:
      subprocess.check_call(['rmdir', '/Q', '/S', options.work_dir],
                            env=os.environ.copy(),
                            shell=True)

def BuildNaClTools(options):
  bot.BuildStep('checkout NaCl tools')
  MakeCheckoutDirs(options)
  MakeInstallDirs(options)
  Checkout(options)
  bot.BuildStep('build NaCl tools')
  Build(options)
  Install(options, ['sel_ldr', 'ncval'])
  CleanUpCheckoutDirs(options)
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
  parser.add_option(
      '-c', '--cleanup', action='store_true', dest='cleanup',
      default=False,
      help='whether to clean up the checkout files')
  parser.add_option(
      '-j', '--jobs', dest='jobs', default='1',
      help='Number of parallel jobs to use while building nacl tools')
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    bot.Print('ERROR: invalid argument')
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
