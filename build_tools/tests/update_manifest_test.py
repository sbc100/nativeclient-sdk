#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for update_manifest.py."""

__author__ = 'mball@google.com (Matt Ball)'

import mox
import os
import subprocess
import sys
import tempfile
import unittest

from build_tools import update_manifest


class FakeOptions(object):
  ''' Just a placeholder for options '''
  pass


class TestUpdateManifest(unittest.TestCase):
  ''' Test basic functionality of the update_manifest package '''
  def setUp(self):
    self._json_boilerplate=(
        '{\n'
        '  "bundles": [],\n'
        '  "manifest_version": 1\n'
        '}\n')
    self._temp_dir = tempfile.gettempdir()
    # os.path.join('build_tools', 'tests', 'test_archive')
    self._manifest = update_manifest.SDKManifest()

  def testJSONBoilerplate(self):
    ''' Test creating a manifest object'''
    self.assertEqual(self._manifest.GetManifestString(),
                     self._json_boilerplate)
    # Test using a manifest file with a version that is too high
    self.assertRaises(Exception,
                      self._manifest.LoadManifestString,
                      '{"manifest_version": 2}')

  def testWriteLoadManifestFile(self):
    ''' Test writing to and loading from a manifest file'''
    # Remove old test file
    file_path = os.path.join(self._temp_dir, 'temp_manifest.json')
    if os.path.exists(file_path):
      os.remove(file_path)
    # Create a basic manifest file
    manifest_file = update_manifest.SDKManifestFile(file_path)
    manifest_file._WriteFile();
    self.assertTrue(os.path.exists(file_path))
    # Test re-loading the file
    manifest_file._manifest._manifest_data['manifest_version'] = 0
    manifest_file._LoadFile()
    self.assertEqual(manifest_file._manifest.GetManifestString(),
                     self._json_boilerplate)

  def testValidateBundleName(self):
    ''' Test validating good and bad bundle names '''
    self.assertTrue(
        self._manifest._ValidateBundleName('A_Valid.Bundle-Name(1)'))
    self.assertFalse(self._manifest._ValidateBundleName('A bad name'))
    self.assertFalse(self._manifest._ValidateBundleName('A bad/name'))
    self.assertFalse(self._manifest._ValidateBundleName('A bad;name'))
    self.assertFalse(self._manifest._ValidateBundleName('A bad,name'))

  def testUpdateManifestVersion(self):
    ''' Test updating the manifest version number '''
    options = FakeOptions()
    options.manifest_version = 99
    self.assertEqual(self._manifest._manifest_data['manifest_version'], 1)
    self._manifest._UpdateManifestVersion(options)
    self.assertEqual(self._manifest._manifest_data['manifest_version'], 99)

  def testVerifyAllOptionsConsumed(self):
    ''' Test function _VerifyAllOptionsConsumed '''
    options = FakeOptions()
    options.opt1 = None
    self.assertTrue(self._manifest._VerifyAllOptionsConsumed(options))
    options.opt2 = 'blah'
    self.assertRaises(Exception,
                      self._manifest._VerifyAllOptionsConsumed,
                      options)

def main():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestUpdateManifest)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())

if __name__ == '__main__':
  sys.exit(main())
