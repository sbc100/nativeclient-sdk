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

import optparse
import os
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

  class TestSDK(unittest.TestCase):
    '''Contains tests that run within an extracted SDK installer'''

    def testExamples(self):
      '''Performs simple testing on the installed SDK'''

      scons = 'scons.bat' if sys.platform == 'win32' else 'scons'
      path = os.path.join(_outdir, 'examples')
      command = [os.path.join(path, scons), '-j', _jobs]

      annotator.Print('Running %s in %s' % (command, path))
      subprocess.check_call(command, cwd=path, shell=True)
      return True

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
