#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Utility to update the SDK manifest file in the build_tools directory'''

import optparse
import os
import sdk_update
import string
import sys


# Map option keys to manifest attribute key. Option keys are used to retrieve
# option values from cmd-line options. Manifest attribute keys label the
# corresponding value in the manifest object.
OPTION_KEY_MAP = {
  #  option key         manifest attribute key
    'bundle_desc_url': 'desc_url',
    'bundle_revision': sdk_update.REVISION_KEY,
    'bundle_version':  sdk_update.VERSION_KEY,
    'desc':            'description',
    'recommended':     'recommended',
    'stability':       'stability',
    }
# Map options keys to platform key, as stored in the bundle.
OPTION_KEY_TO_PLATFORM_MAP = {
    'mac_arch_url':    'mac',
    'win_arch_url':    'win',
    'linux_arch_url':  'linux',
    'all_arch_url':    'all',
    }


class Error(Exception):
  '''Generic error/exception for update_manifest module'''
  pass


def UpdateBundle(bundle, options):
  ''' Update the bundle per content of the options.

  Args:
    options: options data. Attributes that are used are also deleted from
             options.'''
  # Check, set and consume individual bundle options.
  for option_key, attribute_key in OPTION_KEY_MAP.iteritems():
    option_val = getattr(options, option_key, None)
    if option_val is not None:
      bundle[attribute_key] = option_val
      delattr(options, option_key);
  # Validate what we have so far; we may just avoid going through a lengthy
  # download, just to realize that some other trivial stuff is missing.
  bundle.Validate()
  # Check and consume archive-url options.
  for option_key, host_os in OPTION_KEY_TO_PLATFORM_MAP.iteritems():
    platform_url = getattr(options, option_key, None)
    if platform_url is not None:
      bundle.UpdateArchive(host_os, platform_url)
      delattr(options, option_key);


class UpdateSDKManifest(sdk_update.SDKManifest):
  '''Adds functions to SDKManifest that are only used in update_manifest'''

  def _ValidateBundleName(self, name):
    ''' Verify that name is a valid bundle.

    Args:
      name: the proposed name for the bundle.

    Return:
      True if the name is valid for a bundle, False otherwise.'''
    valid_char_set = '()-_.%s%s' % (string.ascii_letters, string.digits)
    name_len = len(name)
    return (name_len > 0 and all(c in valid_char_set for c in name))

  def _UpdateManifestVersion(self, options):
    ''' Update the manifest version number from the options

    Args:
      options: options data containing an attribute self.manifest_version '''
    version_num = int(options.manifest_version)
    self._manifest_data['manifest_version'] = version_num
    del options.manifest_version

  def _UpdateBundle(self, options):
    ''' Update or setup a bundle from the options.

    Args:
      options: options data containing at least a valid bundle_name
               attribute. Other relevant bundle attributes will also be
               used (and consumed) by this function. '''
    # Get and validate the bundle name
    if not self._ValidateBundleName(options.bundle_name):
      raise Error('Invalid bundle name: "%s"' % options.bundle_name)
    bundle_name = options.bundle_name
    del options.bundle_name
    # Get the corresponding bundle, or create it.
    bundle = self.GetBundle(bundle_name)
    if not bundle:
      bundle = sdk_update.Bundle(bundle_name)
      self.SetBundle(bundle)
    UpdateBundle(bundle, options)

  def _VerifyAllOptionsConsumed(self, options, bundle_name):
    ''' Verify that all the options have been used. Raise an exception if
        any valid option has not been used. Returns True if all options have
        been consumed.

    Args:
      options: the object containing the remaining unused options attributes.
      bundl_name: The name of the bundle, or None if it's missing.'''
    # Any option left in the list should have value = None
    for key, val in options.__dict__.items():
      if val != None:
        if bundle_name:
          raise Error('Unused option "%s" for bundle "%s"' % (key, bundle_name))
        else:
          raise Error('No bundle name specified')
    return True;

  def UpdateManifest(self, options):
    ''' Update the manifest object with values from the command-line options

    Args:
      options: options object containing attribute for the command-line options.
               Note that all the non-trivial options are consumed.
    '''
    # Go over all the options and update the manifest data accordingly.
    # Valid options are consumed as they are used. This gives us a way to
    # verify that all the options are used.
    if options.manifest_version is not None:
      self._UpdateManifestVersion(options)
    # Keep a copy of bundle_name, which will be consumed by UpdateBundle, for
    # use in _VerifyAllOptionsConsumed below.
    bundle_name = options.bundle_name
    if bundle_name is not None:
      self._UpdateBundle(options)
    self._VerifyAllOptionsConsumed(options, bundle_name)
    self._ValidateManifest()


class UpdateSDKManifestFile(sdk_update.SDKManifestFile):
  '''Adds functions to SDKManifestFile that are only used in update_manifest'''

  def __init__(self, json_filepath):
    '''Create a new SDKManifest object with default contents.

    If |json_filepath| is specified, and it exists, its contents are loaded and
    used to initialize the internal manifest.

    Args:
      json_filepath: path to jason file to read/write, or None to write a new
          manifest file to stdout.
    '''
    self._json_filepath = json_filepath
    self._manifest = UpdateSDKManifest()
    if self._json_filepath:
      self._LoadFile()

  def UpdateWithOptions(self, options):
    ''' Update the manifest file with the given options. Create the manifest
        if it doesn't already exists. Raises an Error if the manifest doesn't
        validate after updating.

    Args:
      options: option data'''
    self._manifest.UpdateManifest(options)
    self.WriteFile()


def main(argv):
  '''Main entry for update_manifest.py'''

  buildtools_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

  parser = optparse.OptionParser(
      usage="Usage: %prog [options]")

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
  parser.add_option(
      '-f', '--manifest-file', dest='manifest_file',
      default=os.path.join(buildtools_dir, 'json',
                           sdk_update.MANIFEST_FILENAME),
      help='location of manifest file to read and update')

  # Parse options and arguments and check.
  (options, args) = parser.parse_args(argv)
  if len(args) > 0:
    parser.error('These arguments were not understood: %s' % args)

  manifest_file = UpdateSDKManifestFile(options.manifest_file)
  del options.manifest_file
  manifest_file.UpdateWithOptions(options)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
