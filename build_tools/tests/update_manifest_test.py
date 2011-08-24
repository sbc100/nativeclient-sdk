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
import unittest

from build_tools import update_manifest


class TestUpdateManifest(unittest.TestCase):
  """This class tests basic functionality of the update_manifest package"""
  def setUp(self):
    pass

  def testJSONBoilerplate(self):
    json_boilerplate=(
        '{\n'
        '  "bundles": [],\n'
        '  "manifest_version": 1\n'
        '}\n')
    manifest = update_manifest.SDKManifest()
    self.assertEqual(manifest.GetManifestString(), json_boilerplate)
    # Test using a manifest file with a version that is too high
    self.assertRaises(Exception,
                      manifest.LoadManifestString,
                      '{"manifest_version": 2}')

def main():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestUpdateManifest)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())

if __name__ == '__main__':
  sys.exit(main())
