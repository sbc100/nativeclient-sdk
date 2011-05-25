#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" Tests for the SDK Installer

The general structure is as follows:

 1. Extract the installer into a given temporary directory
 2. Run tests -- See TestSDK class (e.g., testExamples)
 3. Remove the installer directory
"""

from __future__ import with_statement

import datetime
import optparse
import os
import platform
import shutil
import string
import subprocess
import sys
import unittest

from build_tools import build_utils

annotator = build_utils.BotAnnotator()


def TestingClosure(_outdir, _jobs):
  '''This closure provides the variables needed by the various tests

  Args:
    _outdir: The output directory that holds the extracted installer
    _jobs: Number of parallel jobs for Make or Scons

  Returns:
    A TestCase class that can be used by the unittest loader
  '''
  toolchain_base = build_utils.NormalizeToolchain(base_dir=_outdir)
  toolchain_bin = os.path.join(toolchain_base, 'bin')
  gcc64 = os.path.join(toolchain_bin, 'nacl64-gcc')
  sel_ldr64 = os.path.join(toolchain_bin, 'nacl64-sel_ldr')

  class TestSDK(unittest.TestCase):
    '''Contains tests that run within an extracted SDK installer'''

    def testBuildExamples(self):
      '''Verify that we can build the SDK examples.'''

      scons = 'scons.bat' if sys.platform == 'win32' else 'scons'
      path = os.path.join(_outdir, 'examples')
      command = [os.path.join(path, scons), '-j', _jobs]

      annotator.Run(' '.join(command), cwd=path, shell=True)
      return True

    def testReadMe(self):
      '''Check that the current build version and date are in the README file'''

      filename = 'README.txt' if sys.platform == 'win32' else 'README'
      with open(os.path.join(_outdir, filename), 'r') as file:
        contents = file.read()
      version = build_utils.RawVersion()
      annotator.Print('Checking that SDK version = %s' % version)
      self.assertTrue(contents.count(version) == 1,
                      'Version mismatch in %s' % filename)

      # Check that the README contains either the current date or yesterday's
      # date (which happens when building over midnight)
      self.assertEqual(
          1,
          contents.count(str(datetime.date.today())) +
          contents.count(str(datetime.date.today() -
                             datetime.timedelta(days=1))),
          "Cannot find today's or yesterday's date in README")
      return True

    def testValgrind(self):
      '''Verify that Valgrind works properly (Linux 64-bit only)'''

      if (sys.platform not in ['linux', 'linux2'] or
          platform.machine() != 'x86_64'):
        annotator.Print('Not running on 64-bit Linux -- skip')
        return True
      true_basename = os.path.join(_outdir, 'true')
      true_c_filename = '%s.c' % true_basename
      true_nexe_filename = '%s.nexe' % true_basename
      with open(true_c_filename, 'w') as true_file:
        true_file.write('int main(void) { return 0; }\n')
      annotator.Run([gcc64, '-o', true_nexe_filename, '-m64', '-O0',
                     '-Wl,-u,have_nacl_valgrind_interceptors', '-g',
                     true_c_filename, '-lvalgrind'])
      memcheck = os.path.join(_outdir, 'third_party', 'valgrind', 'memcheck.sh')
      annotator.Run([memcheck, sel_ldr64, '-Q', true_nexe_filename])

  return TestSDK


def ExtractInstaller(installer, outdir):
  '''Extract the SDK installer into a given directory

  If the outdir already exists, then this function deletes it

  Args:
    installer: full path of the SDK installer
    outdir: output directory where to extract the installer

  Raises:
    OSError - if the outdir already exists
    CalledProcessError - if the extract operation fails
  '''

  annotator.Print('Extracting installer %s into %s' % (installer, outdir))

  if os.path.exists(outdir):
    RemoveDir(outdir)

  if sys.platform == 'win32':
    # Run the self-extracting installer in silent mode with a specified
    # output directory
    command = [installer, '/S', '/D=%s' % outdir]
  else:
    os.mkdir(outdir)
    command = ['tar', '-C', outdir, '--strip-components=1',
               '-xvzf', installer]

  annotator.Print('Running command: %s' % command)
  subprocess.check_call(command)


def RemoveDir(outdir):
  '''Removes the given directory

  On Unix systems, this just runs shutil.rmtree, but on Windows, this doesn't
  work when the directory contains junctions (as does our SDK installer).
  Therefore, on Windows, it runs rmdir /S /Q as a shell command.  This always
  does the right thing on Windows.

  Args:
    outdir: The directory to delete

  Raises:
    CalledProcessError - if the delete operation fails on Windows
    OSError - if the delete operation fails on Linux
  '''

  annotator.Print('Removing %s' % outdir)
  if sys.platform == 'win32':
    subprocess.check_call(['rmdir /S /Q', outdir], shell=True)
  else:
    shutil.rmtree(outdir)


def main():
  '''Main entry for installer tests

  Returns:
    0: Success
    1: Failure

  Also, raises various exceptions for error conditions.
  '''

  parser = optparse.OptionParser(
      usage='Usage: %prog [options] sdk_installer')
  parser.add_option(
      '-o', '--outdir', dest='outdir', default='sdk_temp_dir',
      help='temporary output directory for holding the installer')
  parser.add_option(
      '-j', '--jobs', dest='jobs', default=1,
      help='number of parallel jobs to run')

  options, args = parser.parse_args()

  if len(args) == 0:
    parser.error('Must provide an sdk_installer')

  if len(args) > 1:
    parser.error('Must provide only one sdk_installer')

  installer = args[0]
  outdir = os.path.abspath(options.outdir)

  annotator.Print("Running with installer = %s, outdir = %s, jobs = %s" % (
                  installer, outdir, options.jobs))
  ExtractInstaller(installer, outdir)

  suite = unittest.TestLoader().loadTestsFromTestCase(
      TestingClosure(outdir, options.jobs))
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  RemoveDir(outdir)

  return int(not result.wasSuccessful())


if __name__ == '__main__':
  sys.exit(main())
