#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for build_utils.py."""

__author__ = 'mball@google.com (Matt Ball)'

import platform
import os
import subprocess
import sys
import tarfile
import unittest

from build_tools import build_utils
import mox


class TestBuildUtils(unittest.TestCase):
  """This class tests basic functionality of the build_utils package"""
  def setUp(self):
    self.mock_factory = mox.Mox()

  def testArchitecture(self):
    """Testing the Architecture function"""
    bit_widths = build_utils.SupportedNexeBitWidths()
    # Make sure word-width of either 32 or 64.
    self.assertTrue(32 in bit_widths or 64 in bit_widths)
    if sys.platform in ['linux', 'linux2']:
      self.assertTrue(32 in bit_widths)
      if '64' in platform.machine():
        self.assertTrue(64 in bit_widths)
    elif sys.platform == 'darwin':
      # Mac should have both 32- and 64-bit support.
      self.assertTrue(32 in bit_widths)
      self.assertTrue(64 in bit_widths)
    else:
      # Windows supports either 32- or 64-bit, but not both.
      self.assertEqual(1, len(bit_widths))

  def testGetArchiveTableOfContents(self):
    """Testing the GetArchiveTableOfContents function"""
    # Use a known test archive to validate the TOC entries.
    # The test tar file has this content (from tar tv):
    #   drwxr-xr-x test_links/
    #   drwxr-xr-x test_links/test_dir/
    #   lrwxr-xr-x test_links/test_dir_slnk -> test_dir
    #   -rw-r--r-- test_links/test_hlnk_file_dst1.txt
    #   hrw-r--r-- test_links/test_hlnk_file_dst2.txt link to \
    #              test_links/test_hlnk_file_dst1.txt
    #   hrw-r--r-- test_links/test_hlnk_file_src.txt link to \
    #              test_links/test_hlnk_file_dst1.txt
    #   lrwxr-xr-x test_links/test_slnk_file_dst.txt -> test_slnk_file_src.txt
    #   -rw-r--r-- test_links/test_slnk_file_src.txt
    #   -rw-r--r-- test_links/test_dir/test_file.txt
    #   hrw-r--r-- test_links/test_dir/test_hlnk_file_dst3.txt \
    #              test_links/test_hlnk_file_dst1.txt
    #   lrwxr-xr-x test_links/test_dir/test_slnk_file_dst2.txt -> \
    #              ../test_slnk_file_src.txt

    test_dir = os.path.join('build_tools', 'tests')
    files, dirs, symlinks, links = build_utils.GetArchiveTableOfContents(
        os.path.join(test_dir, 'test_links.tgz'))
    self.assertEqual(3, len(files))
    self.assertTrue(os.path.join('test_links', 'test_slnk_file_src.txt') in
                    files)
    self.assertTrue(os.path.join('test_links', 'test_dir', 'test_file.txt') in
                    files)
    for file in files:
      self.assertFalse('_dir' in os.path.basename(file))
    self.assertTrue(os.path.join('test_links', 'test_dir', 'test_file.txt') in
                    files)

    self.assertEqual(2, len(dirs))
    self.assertTrue(os.path.join('test_links', 'test_dir') in dirs)
    for dir in dirs:
      self.assertFalse('slnk' in dir)
      self.assertFalse('.txt' in dir)
      self.assertFalse(dir in files)
      self.assertFalse(dir in symlinks.keys())

    self.assertEqual(3, len(symlinks))
    self.assertTrue(os.path.join('test_links', 'test_dir_slnk') in symlinks)
    self.assertTrue(os.path.join('test_links', 'test_slnk_file_dst.txt') in
                    symlinks)
    for path, target in symlinks.items():
      self.assertFalse(path in files)
      self.assertFalse(path in dirs)
      # Make sure the target exists in either |files| or |dirs|. The target
      # path in the archive is relative to the source file's path.
      target_path = os.path.normpath(os.path.join(
          os.path.dirname(path), target))
      self.assertTrue((target_path in files) or (target_path in dirs))

    self.assertEqual(3, len(links))
    # There is no "source" file for hard links like there is for a symbolic
    # link, so there it's possible that the hlnk_src file is in the links
    # list, which is OK as long as one of the hlnk files is in the |files|
    # list.  Make sure that only hlnk files are in the |links| list.
    for path, target in links.items():
      self.assertTrue('test_hlnk_file' in path)
      self.assertFalse(path in files)
      self.assertFalse(path in dirs)
      self.assertTrue(target in files)
      self.assertFalse(target in dirs)

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

  def testJoinPathToNaClRepo(self):
    """Testing the 'JoinPathToNaClRepo' utility function."""
    # Test an empty arg list.
    test_dir = os.path.join('third_party', 'native_client')
    self.assertEqual(test_dir, build_utils.JoinPathToNaClRepo())
    # Test an empty arg list with just the root_dir key set.
    test_dir = os.path.join('test_root', test_dir)
    self.assertEqual(test_dir,
                     build_utils.JoinPathToNaClRepo(root_dir='test_root'))
    # Test non-empty arg lists and with and without root_dir.
    test_dir = os.path.join('third_party', 'native_client', 'testing', 'file')
    self.assertEqual(test_dir,
                     build_utils.JoinPathToNaClRepo('testing', 'file'))
    test_dir = os.path.join('test_root', test_dir)
    self.assertEqual(test_dir,
        build_utils.JoinPathToNaClRepo('testing', 'file', root_dir='test_root'))


def RunTests():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestBuildUtils)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())

if __name__ == '__main__':
  sys.exit(RunTests())
