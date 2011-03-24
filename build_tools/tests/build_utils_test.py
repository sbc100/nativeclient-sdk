#!/usr/bin/python2.6
#
# Copyright 2011 The Native Client SDK Authors. All Rights Reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for build_utils.py."""

__author__ = 'mball@google.com (Matt Ball)'

import os
import sys
import unittest

from build_tools import build_utils
import mox


class TestBuildUtils(unittest.TestCase):
  """Class for test cases to cover globally declared helper functions."""

  def testDummy(self):
    self.assertEqual(1, 1)


def RunTests():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestBuildUtils)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())

if __name__ == '__main__':
  sys.exit(RunTests())
