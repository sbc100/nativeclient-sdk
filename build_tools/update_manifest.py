#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Utility to update the SDK manifest file in the build_tools directory'''

import json
import optparse
import os
import shutil
import string
import sys
import tempfile

# Valid values for bundle.stability field
STABILITY_LITERALS = [
    'obsolete', 'post_stable', 'stable', 'beta', 'dev', 'canary'
]
# Valid values for bundle-recommended field.
YES_NO_LITERALS = ['yes', 'no']
# Map option keys to manifest attribute key. Option keys are used to retrieve
# option values fromcmd-line options. Manifest attribute keys label the
# corresponding value in the manifest object.
OPTION_KEY_MAP = {
  #  option key         manifest attribute key
    'bundle_desc_url': 'desc_url',
    'bundle_revision': 'revision',
    'bundle_version':  'version',
    'desc':            'description',
    'recommended':     'recommended',
    'stability':       'stability',
}



class ManifestException(Exception):
  ''' Exceptions raised within update_manifest '''
  pass


class Bundle(dict):
  ''' A placeholder for sdk bundle information. We derive Bundle from
      dict so that it is easily serializable. '''
  def __init__(self, name):
    ''' Create a new bundle with the given bundle name. '''
    self['name'] = name

  def Validate(self):
    ''' Validate the content of the bundle. Raise ManifestException if
        an invalid or missing field is found. '''
    # Check required fields.
    if not self.get('name', None):
      raise ManifestException('Bundle has no name')
    if not self.get('revision', None):
      raise ManifestException('Bundle "%s" is missing a revision number' %
                              self['name'])
    if not self.get('description', None):
      raise ManifestException('Bundle "%s" is missing a description' %
                              self['name'])
    if not self.get('stability', None):
      raise ManifestException('Bundle "%s" is missing stability info' %
                              self['name'])
    if self.get('recommended', None) == None:
      raise ManifestException('Bundle "%s" is missing the recommended field' %
                              self['name'])
    # Check specific values
    if self['stability'] not in STABILITY_LITERALS:
      raise ManifestException('Bundle "%s" has invalid stability field: "%s"' %
                              (self['name'], self['stability']))
    if self['recommended'] not in YES_NO_LITERALS:
      raise ManifestException(
          'Bundle "%s" has invalid recommended field: "%s"' %
          (self['name'], self['recommended']))

  def Update(self, options):
    ''' Update the bundle per content of the options.

    Args:
      options: options data. Attributes that are used are also deleted from
               options.'''
    # Check, set and consume individual options
    for option_key, attribute_key in OPTION_KEY_MAP.iteritems():
      option_val = getattr(options, option_key, None)
      if option_val:
        self[attribute_key] = option_val
        delattr(options, option_key);


class SDKManifest(object):
  '''This class contains utilities for manipulation an SDK manifest string

  For ease of unit-testing, this class should not contain any file I/O.
  '''

  def __init__(self):
    '''Create a new SDKManifest object with default contents'''
    self.MANIFEST_VERSION = 1
    self._manifest_data = {
        "manifest_version": self.MANIFEST_VERSION,
        "bundles": []
    }


  def _ValidateManifest(self):
    '''Validate the Manifest file and raises an exception for problems'''
    # Validate the manifest top level
    if self._manifest_data["manifest_version"] > self.MANIFEST_VERSION:
      raise ManifestException("Manifest version too high: %s" %
                              self._manifest_data["manifest_version"])
    # Validate each bundle
    for bundle in self._manifest_data['bundles']:
      bundle.Validate()

  def _ValidateBundleName(self, name):
    ''' Verify that name is a valid bundle.

    Args:
      name: the proposed name for the bundle.

    Return:
      True if the name is valid for a bundle, False otherwise.'''
    valid_char_set = '()-_.%s%s' % (string.ascii_letters, string.digits)
    name_len = len(name)
    return (name_len > 0 and all(c in valid_char_set for c in name))

  def _GetBundle(self, name):
    ''' Get a bundle from the array of bundles.

    Args:
      name: the name of the bundle to return.
    Return:
      The bundle with the given name, or None if it is not found.'''
    if not 'bundles' in self._manifest_data:
      return None
    bundles = self._manifest_data['bundles']
    for bundle in bundles:
      if 'name' in bundle and bundle['name'] == name:
        return bundle
    return None

  def _AddBundle(self, bundle):
    ''' Add a bundle to the manifest

    Args:
      bundle: to the bundle to add to the manifest.'''
    if not 'bundle' in self._manifest_data:
      self._manifest_data['bundles'] = []
    self._manifest_data['bundles'].append(bundle)

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
      raise ManifestException('Invalid bundle name: "%s"' %
                              options.bundle_name)
    bundle_name = options.bundle_name
    del options.bundle_name
    # Get the corresponding bundle, or create it.
    bundle = self._GetBundle(bundle_name)
    if not bundle:
      bundle = Bundle(bundle_name)
      self._AddBundle(bundle)
    bundle.Update(options)

  def _VerifyAllOptionsConsumed(self, options):
    ''' Verify that all the options have been used. Raise an exception if
        any valid option has not been used. Returns True if all options have
        been consumed.

    Args:
      options: the object containg the remaining unused options attributes.'''
    # Any option left in the list should have value = None
    for key, val in options.__dict__.items():
      if val != None:
        raise ManifestException("Unused option: %s" % key)
    return True;


  def LoadManifestString(self, json_string):
    ''' Load a JSON manifest string. Raises an exception if json_string
        is not well-formed JSON.

    Args:
      json_string: a JSON-formatted string containing the previous manifest'''
    new_manifest = json.loads(json_string)
    for key, value in new_manifest.items():
      self._manifest_data[key] = value
    self._ValidateManifest()


  def GetManifestString(self):
    '''Returns the current JSON manifest object, pretty-printed'''
    pretty_string = json.dumps(self._manifest_data, sort_keys=False, indent=2)
    # json.dumps sometimes returns trailing whitespace and does not put
    # a newline at the end.  This code fixes these problems.
    pretty_lines = pretty_string.split('\n')
    return '\n'.join([line.rstrip() for line in pretty_lines]) + '\n'

  def UpdateManifest(self, options):
    ''' Update the manifest object with value from the command-line options

    Args:
      options: options object containing attribute for the command-line options.
               Note that all the non-trivial options are consumed.
    '''
    # Go over all the options and update the manifest data accordingly.
    # Valid options are consumed as they are used. This gives us a way to
    # verify that all the options are used.
    if options.manifest_version is not None:
      self._UpdateManifestVersion(options)
    if options.bundle_name is not None:
      self._UpdateBundle(options)
    self._VerifyAllOptionsConsumed(options)
    self._ValidateManifest()


class SDKManifestFile(object):
  ''' This class provides basic file I/O support for manifest objects.'''

  def __init__(self, json_filepath):
    '''Create a new SDKManifest object with default contents.

    Args:
      json_filepath: path to jason file to read/write, or None to write a new
      manifest file to stdout.'''
    self._json_filepath = json_filepath
    self._manifest = SDKManifest()

  def _LoadFile(self):
    '''Load the manifest from the JSON file. This function returns quietly
       if the file doesn't exit.'''
    if not os.path.exists(self._json_filepath):
      return

    with open(self._json_filepath, 'r') as f:
      json_string = f.read()
    if json_string:
      self._manifest.LoadManifestString(json_string)


  def _WriteFile(self):
    '''Write the json data to the file. If not file name was specified, the
       data is written to stdout.'''
    json_string = self._manifest.GetManifestString()
    if not self._json_filepath:
      # No file is specified; print the json data to stdout
      sys.stdout.write(json_string)
    else:
      # Write the JSON data to a temp file.
      temp_file_name = None
      with tempfile.NamedTemporaryFile(mode='w', delete=False) as f:
        f.write(json_string)
        temp_file_name = f.name
      # Move the temp file to the actual file.
      if os.path.exists(self._json_filepath):
        os.remove(self._json_filepath)
      shutil.move(temp_file_name, self._json_filepath)

  def UpdateWithOptions(self, options):
    ''' Update the manifest file with the given options. Create the manifest
        if it doesn't already exists. Raises a ManifestException if the
        manifest doesn't validate after updating.

    Args:
      options: option data'''
    if self._json_filepath:
      self._LoadFile()
    self._manifest.UpdateManifest(options)
    self._WriteFile()


def main(argv):
  '''Main entry for update_manifest.py'''

  parser = optparse.OptionParser(
      usage="Usage: %prog [options] [manifest_file]")

  # Setup options
  parser.add_option(
      '-A', '--all_tgz', dest='all_tgz_url',
      default=None,
      help='URL for an all-platform tgz archive.')
  parser.add_option(
      '-B', '--bundle_revision', dest='bundle_revision',
      type='int',
      default=None,
      help='Required: Revision number for the bundle.')
  parser.add_option(
      '-b', '--bundle_version', dest='bundle_version',
      type='int',
      default=None,
      help='Optional: Version number for the bundle.')
  parser.add_option(
      '-d', '--description', dest='desc',
      default=None,
      help='Required: Description for this bundle.')
  parser.add_option(
      '-M', '--mac_tgz', dest='mac_tgz_url',
      default=None,
      help='URL for the Mac tgz archive.')
  parser.add_option(
      '-L', '--linux_tgz', dest='linux_tgz_url',
      default=None,
      help='URL for the Linux tgz archive.')
  parser.add_option(
      '-n', '--bundle_name', dest='bundle_name',
      default=None,
      help='Required: Name of the bundle.')
  parser.add_option(
      '-r', '--recommended', dest='recommended',
      choices=YES_NO_LITERALS,
      default=None,
      help='Required: True or False, whether this bundle is recommended.')
  parser.add_option(
      '-s', '--stability', dest='stability',
      choices=STABILITY_LITERALS,
      default=None,
      help='Required: Stability for this bundle; one of. '
           '"obsolete", "post_stable", "stable", "beta", "dev", "canary".')
  parser.add_option(
      '-u', '--desc_url', dest='bundle_desc_url',
      default=None,
      help='Optional: URL to follow to read additional bundle info.')
  parser.add_option(
      '-v', '--manifest_version', dest='manifest_version',
      type='int',
      default=None,
      help='Required for new manifest files: '
           'Version number for the manifest.')
  parser.add_option(
      '-W', '--win_tgz', dest='win_tgz_url',
      default=None,
      help='URL for the Windows tgz archive.')

  # Parse options and arguments and check.
  (options, args) = parser.parse_args(argv)
  if len(args) > 1:
    parser.error('Must provide at most one manifest file name')

  filename = None
  if len(args) == 1:
    filename = str(args[0])
  manifest_file = SDKManifestFile(filename)
  manifest_file.UpdateWithOptions(options)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
