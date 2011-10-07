#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Utility to update the SDK manifest file in the build_tools directory'''

import optparse
import sdk_update
import sys


def main(argv):
  '''Main entry for update_manifest.py'''

  parser = optparse.OptionParser(
      usage="Usage: %prog [options] [manifest_file]")

  # Setup options
  parser.add_option(
      '-b', '--bundle-version', dest='bundle_version',
      type='int',
      default=None,
      help='Required: Version number for the bundle.')
  parser.add_option(
      '-B', '--bundle-revision', dest='bundle_revision',
      type='int',
      default=None,
      help='Required: Revision number for the bundle.')
  parser.add_option(
      '-d', '--description', dest='desc',
      default=None,
      help='Required: Description for this bundle.')
  parser.add_option(
      '-M', '--mac-archive', dest='mac_arch_url',
      default=None,
      help='URL for the Mac archive.')
  parser.add_option(
      '-L', '--linux-archive', dest='linux_arch_url',
      default=None,
      help='URL for the Linux archive.')
  parser.add_option(
      '-n', '--bundle-name', dest='bundle_name',
      default=None,
      help='Required: Name of the bundle.')
  parser.add_option(
      '-r', '--recommended', dest='recommended',
      choices=sdk_update.YES_NO_LITERALS,
      default=None,
      help='Required: whether this bundle is recommended. one of "yes" or "no"')
  parser.add_option(
      '-s', '--stability', dest='stability',
      choices=sdk_update.STABILITY_LITERALS,
      default=None,
      help='Required: Stability for this bundle; one of. '
           '"obsolete", "post_stable", "stable", "beta", "dev", "canary".')
  parser.add_option(
      '-u', '--desc-url', dest='bundle_desc_url',
      default=None,
      help='Optional: URL to follow to read additional bundle info.')
  parser.add_option(
      '-v', '--manifest-version', dest='manifest_version',
      type='int',
      default=None,
      help='Required for new manifest files: '
           'Version number for the manifest.')
  parser.add_option(
      '-W', '--win-archive', dest='win_arch_url',
      default=None,
      help='URL for the Windows archive.')

  # Parse options and arguments and check.
  (options, args) = parser.parse_args(argv)
  if len(args) > 1:
    parser.error('Must provide at most one manifest file name')

  filename = None
  if len(args) == 1:
    filename = str(args[0])
  manifest_file = sdk_update.SDKManifestFile(filename)
  manifest_file.UpdateWithOptions(options)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
