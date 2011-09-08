#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Class that produces a table of contents form a tar archive."""

import os
import tarfile


class TarArchive(object):
  '''Container for a tar archive table of contents.

  The table of contents is an enumeration of each node in the archive, broken
  into four separate collections, each accessible as a property (see the
  "Attributes" section, below).

  The tar archive can be taken directly from a tarball (as long as the format
  is supported by the tarfile module), or from a manifest file that was
  generated using tar -tv.

  Attributes:
    files: A set of plain files, keys are platform-specific normalized path
        names.  Might be empty.
    dirs: A set of directories, keys are platform-specific normalized path
        names.  Might be empty.
    symlinks: A dictionary of symbolic links - these are not followed.  Keys
        are platform-specific normalized path names.  The value of each key is
        the source file of the link (also a platform-specific normalized path).
        Might be empty.
    links: A dictionary of hard links.  Keys are platform-specific normalized
        path names.  The value of each key is the source file of the link (also
        a platform-specific normalized path).  Might be empty.
  '''

  def __init__(self):
    self.files = set()
    self.dirs = set()
    self.symlinks = dict()
    self.links = dict()

  def InitWithTarFile(self, tar_archive_file):
    '''Initialize the object using a tar-format archive file.

    Wipes out any old table of contents data and replaces it with the new table
    of contents from |tar_archive_file|.  If the given tar archive doesn't
    exist, or if it is not in a recognizable format, this method does nothing.

    Args:
      tar_archive_file: The archive file.  This is expected to be a file in a
          tar format that the python tarfile module recognizes.
    Raises:
      OSError if |tar_archive_file| doesn't exist.
    '''
    def MakePathSet(condition):
      '''Helper function used with a lambda to generate a set of path names.'''
      return set([os.path.normpath(tarinfo.name)
                  for tarinfo in tar_archive if condition(tarinfo)])

    def MakeLinksDict(condition):
      '''Helper function used with a lambda to generate the link dicitonaries.

      Note that accessing tarinfo.linkname raises an exception if
      the TarInfo member is not a link, which is why there are two separate
      helper functions.
      '''
      return dict([(os.path.normpath(tarinfo.name),
                    os.path.normpath(tarinfo.linkname))
                   for tarinfo in tar_archive if condition(tarinfo)])

    if not os.path.exists(tar_archive_file):
      raise OSError('%s does not exist' % tar_archive_file)
    tar_archive = None
    try:
      tar_archive = tarfile.open(tar_archive_file)
      self.files = MakePathSet(lambda x: x.isfile())
      self.dirs = MakePathSet(lambda x: x.isdir())
      self.symlinks = MakeLinksDict(lambda x: x.issym())
      self.links = MakeLinksDict(lambda x: x.islnk())
    finally:
      if tar_archive:
        tar_archive.close()

  def InitWithManifest(self, tar_manifest_file):
    '''Parse a tar-style manifest file and return the table of contents.

    Wipes out any old table of contents data and replaces it with the new table
    of contents from |tar_manifest_file|.  If the given manifest file doesn't
    exist this method does nothing.

    Args:
      tar_manifest_file: The manifest file.  This is expected to be a file that
          contains the result of tar -tv on the associated tarball.
    Raises:
      OSError if |tar_archive_file| doesn't exist.
    '''
    # Index values into the manifest entry list.  Note that some of these
    # indices are negative (counting back from the last element), this is
    # because the intermediate fields (such as date) from tar -tv differ from
    # platform to platform.  The last fields in the link members are always the
    # same, however.  All symlinks end in ['link_src', '->', 'link']; all hard
    # links end in ['link_src', 'link', 'to', 'link'].
    PERM_BITS_INDEX = 0
    # The symbolic link name is the third-from-last entry.
    SYMLINK_FILENAME_INDEX = -3
    # The hard link name is the fourth-from-last entry.
    HLINK_FILENAME_INDEX = -4
    LAST_ITEM_INDEX = -1

    if not os.path.exists(tar_manifest_file):
      raise OSError('%s does not exist' % tar_manifest_file)
    with open(tar_manifest_file) as manifest:
      for manifest_item in map(lambda line: line.split(), manifest):
        # Parse a single tar -tv entry in a manifest.
        # The tar -tv entry is represented as a list of strings.  The first
        # string represents the permission bits; the first bit indicates the
        # kind of entry.  Depending on the kind of entry, other fields represent
        # the member's path name, link source, etc.  An entry list might look
        # like this:
        #   ['hrwxr-xr-x', 'Administrators/Domain', 'Users', '0', '2011-08-09',
        #    '08:11', 'toolchain/win_x86/bin/i686-nacl-g++.exe', 'link', 'to',
        #    'toolchain/win_x86/bin/i686-nacl-addr2line.exe']
        # This example is a hard link from i686-nacl-addr2line.exe to
        # i686-nacl-g++.exe.
        file_type = manifest_item[PERM_BITS_INDEX][0]
        if file_type == 'd':
          # A directory: the name is the final element of the entry.
          self.dirs.add(os.path.normpath(manifest_item[LAST_ITEM_INDEX]))
        elif file_type == 'h':
          # A hard link: the last element is the source of the hard link, and is
          # entered as the key in the |links| dictionary.
          link_name = os.path.normpath(manifest_item[HLINK_FILENAME_INDEX])
          self.links[link_name] = os.path.normpath(
              manifest_item[LAST_ITEM_INDEX])
        elif file_type == 'l':
          # A symbolic link: the last element is the source of the symbolic
          # link.
          link_name = os.path.normpath(manifest_item[SYMLINK_FILENAME_INDEX])
          self.symlinks[link_name] = os.path.normpath(
              manifest_item[LAST_ITEM_INDEX])
        else:
          # Everything else is considered a plain file.
          self.files.add(os.path.normpath(manifest_item[LAST_ITEM_INDEX]))

