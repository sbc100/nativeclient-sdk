#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for tar_archive.py."""

import os
import sys
import tarfile
import unittest

from build_tools import tar_archive


class TestTarArchive(unittest.TestCase):
  """This class tests basic functionality of the tar_archive package"""

  def setUp(self):
    self.archive = tar_archive.TarArchive()

  def ValidateTableOfContents(self, archive):
    '''Helper method to validate an archive table of contents.

    The test table of contents file has these entries (from tar tv):
      drwxr-xr-x test_links/
      drwxr-xr-x test_links/test_dir/
      lrwxr-xr-x test_links/test_dir_slnk -> test_dir
      -rw-r--r-- test_links/test_hlnk_file_dst1.txt
      hrw-r--r-- test_links/test_hlnk_file_dst2.txt link to \
                 test_links/test_hlnk_file_dst1.txt
      hrw-r--r-- test_links/test_hlnk_file_src.txt link to \
                 test_links/test_hlnk_file_dst1.txt
      lrwxr-xr-x test_links/test_slnk_file_dst.txt -> test_slnk_file_src.txt
      -rw-r--r-- test_links/test_slnk_file_src.txt
      -rw-r--r-- test_links/test_dir/test_file.txt
      hrw-r--r-- test_links/test_dir/test_hlnk_file_dst3.txt \
                 test_links/test_hlnk_file_dst1.txt
      lrwxr-xr-x test_links/test_dir/test_slnk_file_dst2.txt -> \
                 ../test_slnk_file_src.txt

    Args:
      archive: The TarArchive object under test.
    '''
    self.assertEqual(3, len(archive.files))
    self.assertTrue(os.path.join('test_links', 'test_slnk_file_src.txt') in
                    archive.files)
    self.assertTrue(os.path.join('test_links', 'test_dir', 'test_file.txt') in
                    archive.files)
    for file in archive.files:
      self.assertFalse('_dir' in os.path.basename(file))
    self.assertTrue(os.path.join('test_links', 'test_dir', 'test_file.txt') in
                    archive.files)

    self.assertEqual(2, len(archive.dirs))
    self.assertTrue(os.path.join('test_links', 'test_dir') in archive.dirs)
    for dir in archive.dirs:
      self.assertFalse('slnk' in dir)
      self.assertFalse('.txt' in dir)
      self.assertFalse(dir in archive.files)
      self.assertFalse(dir in archive.symlinks.keys())

    self.assertEqual(3, len(archive.symlinks))
    self.assertTrue(os.path.join('test_links', 'test_dir_slnk') in
                    archive.symlinks)
    self.assertTrue(os.path.join('test_links', 'test_slnk_file_dst.txt') in
                    archive.symlinks)
    for path, target in archive.symlinks.items():
      self.assertFalse(path in archive.files)
      self.assertFalse(path in archive.dirs)
      # Make sure the target exists in either |archive.files| or
      # |archive.dirs|. The target path in the archive is relative to the
      #source file's path.
      target_path = os.path.normpath(os.path.join(
          os.path.dirname(path), target))
      self.assertTrue((target_path in archive.files) or
                      (target_path in archive.dirs))

    self.assertEqual(3, len(archive.links))
    # There is no "source" file for hard links like there is for a symbolic
    # link, so there it's possible that the hlnk_src file is in the
    # |archive.links| set, which is OK as long as one of the hlnk files is
    # in the |archive.files| set.  Make sure that only hlnk files are in the
    # |archive.links| list.
    for path, target in archive.links.items():
      self.assertTrue('test_hlnk_file' in path)
      self.assertFalse(path in archive.files)
      self.assertFalse(path in archive.dirs)
      self.assertTrue(target in archive.files)
      self.assertFalse(target in archive.dirs)

  def testConstructor(self):
    """Test default constructor."""
    self.assertEqual(0, len(self.archive.files))
    self.assertEqual(0, len(self.archive.dirs))
    self.assertEqual(0, len(self.archive.symlinks))
    self.assertEqual(0, len(self.archive.links))

  def testFromTarball(self):
    """Testing the TarArchive when using a tarball"""
    # Use a known test archive to validate the TOC entries.
    self.archive.InitWithTarFile(os.path.join('build_tools',
                                              'tests',
                                              'test_links.tgz'))
    self.ValidateTableOfContents(self.archive)

  def testFromTarballBadFile(self):
    """Testing the TarArchive when using a bad tarball"""
    self.assertRaises(OSError,
                      self.archive.InitWithTarFile,
                      'nosuchfile')
    self.assertRaises(tarfile.ReadError,
                      self.archive.InitWithTarFile,
                      os.path.join('build_tools',
                                   'tests',
                                   'test_links.tgz.manifest'))

  def testFromManifest(self):
    """Testing the TarArchive when using a manifest file"""
    # Use a known test manifest to validate the TOC entries.
    # The test manifest file is the output of tar -tv on the tarball used in
    # testGetArchiveTableOfContents().
    self.archive.InitWithManifest(os.path.join('build_tools',
                                               'tests',
                                               'test_links.tgz.manifest'))
    self.ValidateTableOfContents(self.archive)

  def testFromManifestBadFile(self):
    """Testing the TarArchive when using a bad manifest file"""
    self.assertRaises(OSError, self.archive.InitWithManifest, 'nosuchfile')


def RunTests():
  suite = unittest.TestLoader().loadTestsFromTestCase(TestTarArchive)
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  return int(not result.wasSuccessful())


if __name__ == '__main__':
  sys.exit(RunTests())
