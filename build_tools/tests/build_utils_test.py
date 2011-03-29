#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for build_utils.py."""

__author__ = 'mball@google.com (Matt Ball)'

import sys
import unittest

from build_tools import build_utils
import mox


class TestBuildUtils(unittest.TestCase):
  """This class tests basic functionality of the build_utils package"""
  def setUp(self):
    self.mock_factory = mox.Mox()

  def testBotAnnotator(self):
    """Unittest for the BotAnnotator class"""
    stdout_mock = self.mock_factory.CreateMock(sys.stdout)
    stdout_mock.write("My Bot Message\n")
    stdout_mock.flush()
    stdout_mock.write("@@@BUILD_STEP MyBuildStep@@@\n")
    stdout_mock.flush()
    self.mock_factory.ReplayAll()
    bot = build_utils.BotAnnotator(stdout_mock)
    bot.Print("My Bot Message")
    bot.BuildStep("MyBuildStep")
    self.mock_factory.VerifyAll()


def RunTests():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestBuildUtils)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())

if __name__ == '__main__':
  sys.exit(RunTests())
