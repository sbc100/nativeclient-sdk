#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for install_nsis.py."""

import os
import shutil
import subprocess
import sys
import unittest

from build_tools import install_nsis


class TestInstallNsis(unittest.TestCase):
  """This class tests basic functionality of the install_nsis package"""
  def setUp(self):
    self.nsis_installer_ = os.path.join(os.path.abspath('build_tools'),
                                        install_nsis.NSIS_INSTALLER)

  def testNsisInstallerExists(self):
    """Ensure that the correct version of NSIS is present."""
    self.assertTrue(os.path.exists(self.nsis_installer_))

  def testBogusNsisInstaller(self):
    """Make sure the installer handles invalid directory names."""
    self.assertRaises(IOError, install_nsis.InstallNsis, 'bogus', 'not_a_dir')

  def testNsisInstaller(self):
    """Make sure the installer produces an NSIS directory."""
    target_dir = os.path.join(os.path.dirname(self.nsis_installer_),
                              'nsis_test',
                              'NSIS')
    install_nsis.InstallNsis(self.nsis_installer_, target_dir)
    self.assertTrue(os.path.exists(os.path.join(target_dir, 'makensis.exe')))
    shutil.rmtree(os.path.dirname(target_dir))


def RunTests():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestInstallNsis)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())


if __name__ == '__main__':
  sys.exit(RunTests())
