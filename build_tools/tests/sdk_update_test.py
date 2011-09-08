#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for sdk_update.py."""

import exceptions
import mox
import os
import subprocess
import sys
import tempfile
import unittest

from build_tools import sdk_update


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PARENT_DIR = os.path.dirname(SCRIPT_DIR)


class FakeOptions(object):
  ''' Just a placeholder for options '''
  pass


def CallSDKUpdate(args):
  '''Calls the sdk_update.py utility and returns stdout as a string

  Args:
    args: command-line arguments as a list (not including program name)

  Returns:
    string containing stdout

  Raises:
    subprocess.CalledProcessError: non-zero return code from sdk_update'''
  command = ['python', os.path.join(PARENT_DIR, 'sdk_update.py')] + args
  process = subprocess.Popen(stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             args=command)
  output, error_output = process.communicate()

  retcode = process.poll() # Note - calling wait() can cause a deadlock
  if retcode != 0:
    raise subprocess.CalledProcessError(retcode, command)
  return output


class TestSDKUpdate(unittest.TestCase):
  ''' Test basic functionality of the sdk_update package '''
  def setUp(self):
    _options = FakeOptions()
    _options.url = 'file://%s' % os.path.join(SCRIPT_DIR,
                                              'naclsdk_manifest_test.json')
    # TODO(mball) Use a proper temporary directory instead of the test
    _options.user_data_dir = SCRIPT_DIR

  def testBadArg(self):
    '''Test that using a bad argument results in an error'''
    self.assertRaises(subprocess.CalledProcessError, CallSDKUpdate, ['--bad'])

  def testGetHostOS(self):
    '''Test that the GetHostOS function returns a valid value'''
    self.assertTrue(sdk_update.GetHostOS() in ['linux', 'mac', 'win'])

  def testHelp(self):
    '''Test that basic help works'''
    # Running any help command should call sys.exit()
    self.assertRaises(exceptions.SystemExit, sdk_update.main, ['-h'])

  def testList(self):
    '''Test the List function'''
    command = ['--manifest-url',
               os.path.join(SCRIPT_DIR, 'naclsdk_manifest_test.json'),
               'list']
    bundle_list = CallSDKUpdate(command)
    # Just do some simple sanity checks on the resulting string
    self.assertEqual(bundle_list.count('sdk_tools'), 1)
    self.assertEqual(bundle_list.count('test_1'), 1)
    self.assertEqual(bundle_list.count('description:'), 2)

  def testUpdateHelp(self):
    '''Test the help for the update command'''
    self.assertRaises(exceptions.SystemExit,
                      sdk_update.main, ['help', 'update'])

  def testVersion(self):
    '''Test that showing the version works'''
    self.assertRaises(exceptions.SystemExit, sdk_update.main, ['--version'])

def main():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestSDKUpdate)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())

if __name__ == '__main__':
  sys.exit(main())
