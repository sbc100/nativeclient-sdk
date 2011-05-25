#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for build_utils.py."""

__author__ = 'mball@google.com (Matt Ball)'

import subprocess
import sys
import unittest

from build_tools import build_utils
import mox


class TestBuildUtils(unittest.TestCase):
  """This class tests basic functionality of the build_utils package"""
  def setUp(self):
    self.mock_factory = mox.Mox()

  def testBotAnnotatorPrint(self):
    """Testing the Print function of the BotAnnotator class"""
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

  def testBotAnnotatorRun(self):
    """Testing the 'Run' command of the BotAnnotator class"""
    out_string = 'hello'
    print_command = ['python', '-c',
                     "import sys; sys.stdout.write('%s')" % out_string]
    error_command = ['python', '-c', "import sys; sys.exit(1)"]
    stdout_mock = self.mock_factory.CreateMock(sys.stdout)
    stdout_mock.write('Running %s\n' % print_command)
    stdout_mock.flush()
    stdout_mock.write('%s\n' % out_string)
    stdout_mock.flush()
    stdout_mock.write('Running %s\n' % error_command)
    stdout_mock.flush()
    stdout_mock.write('\n')
    stdout_mock.flush()
    self.mock_factory.ReplayAll()
    bot = build_utils.BotAnnotator(stdout_mock)
    run_output = bot.Run(print_command)
    self.assertEqual(run_output, "%s" % out_string)
    self.assertRaises(subprocess.CalledProcessError, bot.Run, error_command)
    self.mock_factory.VerifyAll()

def RunTests():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestBuildUtils)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())

if __name__ == '__main__':
  sys.exit(RunTests())
