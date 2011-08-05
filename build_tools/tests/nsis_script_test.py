#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for nsis_script.py."""

import filecmp
import os
import sys
import tarfile
import tempfile
import unittest

from build_tools import nsis_script


class TestNsisScript(unittest.TestCase):
  """This class tests basic functionality of the nsis_script package"""

  def FilterSvn(self, list):
    '''A filter used to remove .svn dirs from installer lists.'''
    return [elt for elt in list if elt.count('.svn') == 0]

  def testConstructor(self):
    """Test default constructor."""
    script = nsis_script.NsisScript('test_script.nsi')
    self.assertEqual('test_script.nsi', script.GetScriptFile())
    self.assertEqual(0, len(script.GetFileList()))
    self.assertEqual(0, len(script.GetDirectoryList()))
    self.assertEqual(0, len(script.GetSymlinkList()))

  def testInstallDir(self):
    """Test install directory accessor/mutator."""
    script = nsis_script.NsisScript('test_script.nsi')
    script.SetInstallDirectory('C:\\test_install_dir')
    self.assertEqual('C:\\test_install_dir', script.GetInstallDirectory())

  def testCreateFromDirectory(self):
    """Test creation of artifact lists from an archive directory."""
    script = nsis_script.NsisScript('test_script.nsi')
    archive_dir = os.path.join('build_tools', 'tests', 'test_archive')
    script.CreateFromDirectory(archive_dir,
                               dir_filter=self.FilterSvn,
                               file_filter=self.FilterSvn)
    file_list = script.GetFileList()
    self.assertEqual(3, len(file_list))
    self.assertTrue(os.path.join(archive_dir, 'test_file.txt') in file_list)
    self.assertTrue(os.path.join(archive_dir, 'test_dir', 'test_dir_file1.txt')
        in file_list)
    self.assertTrue(os.path.join(archive_dir, 'test_dir', 'test_dir_file2.txt')
        in file_list)
    dir_list = script.GetDirectoryList()
    self.assertEqual(1, len(dir_list))
    self.assertTrue(os.path.join(archive_dir, 'test_dir') in dir_list)

  def testCreateInstallNameScript(self):
    """Test the install name include script."""
    test_dir = os.path.join('build_tools', 'tests')
    script = nsis_script.NsisScript(os.path.join(test_dir, 'test_script.nsi'))
    script.CreateInstallNameScript(cwd=test_dir)
    install_name_script = open(os.path.join(test_dir, 'sdk_install_name.nsh'),
                               'r')
    self.assertTrue(script.GetInstallDirectory() in install_name_script.read())
    install_name_script.close()
    os.remove(os.path.join(test_dir, 'sdk_install_name.nsh'))

  def testNormalizeInstallPath(self):
    """Test NormalizeInstallPath."""
    # If CreateFromDirectory() is not called, then install paths are unchanged.
    test_dir = os.path.join('build_tools', 'tests')
    script = nsis_script.NsisScript(os.path.join(test_dir, 'test_script.nsi'))
    test_path = os.path.join('C:', 'test', 'path')
    path = script.NormalizeInstallPath(test_path)
    self.assertEqual(test_path, path)
    # Set a relative install path.
    archive_dir = os.path.join(test_dir, 'test_archive')
    script.CreateFromDirectory(archive_dir,
                               dir_filter=self.FilterSvn,
                               file_filter=self.FilterSvn)
    test_path = os.path.join('test', 'relative', 'path')
    path = script.NormalizeInstallPath(os.path.join(archive_dir, test_path))
    self.assertEqual(test_path, path)

  def testCreateSectionNameScript(self):
    """Test the section name script."""
    test_dir = os.path.join('build_tools', 'tests')
    script = nsis_script.NsisScript(os.path.join(test_dir, 'test_script.nsi'))
    archive_dir = os.path.join(test_dir, 'test_archive')
    script.CreateFromDirectory(archive_dir,
                               dir_filter=self.FilterSvn,
                               file_filter=self.FilterSvn)
    script.CreateSectionNameScript(cwd=test_dir)
    # TODO(dspringer): re-enable this test when the bots work with it.
    with open(os.path.join(test_dir, 'sdk_section.nsh')) as nsh:
      print 'actual output:\n%s' % str(nsh.readlines())
    # self.assertEqual(1,
    #     filecmp.cmp(os.path.join(test_dir, 'sdk_section.nsh'),
    #                 os.path.join(test_dir, 'test_sdk_section.nsh')))
    os.remove(os.path.join(test_dir, 'sdk_section.nsh'))


def RunTests():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestNsisScript)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())


if __name__ == '__main__':
  sys.exit(RunTests())
