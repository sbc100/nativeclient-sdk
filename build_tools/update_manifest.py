#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Utility to update the SDK manifest file in the build_tools directory'''

import hashlib
import json
import optparse
import os
import shutil
import string
import sys
import tempfile
import urllib2

# Valid values for bundle.stability field
STABILITY_LITERALS = [
    'obsolete', 'post_stable', 'stable', 'beta', 'dev', 'canary'
]
# Valid values for the archive.host_os field
HOST_OS_LITERALS = frozenset(['mac', 'win', 'linux', 'all'])
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
# Map options keys to platform key, as stored in the bundle.
OPTION_KEY_TO_PLATFORM_MAP = {
    'mac_arch_url':    'mac',
    'win_arch_url':    'win',
    'linux_arch_url':  'linux',
    'all_arch_url':    'all',
}
# Valid keys for various sdk objects, used for validation.
VALID_ARCHIVE_KEYS = frozenset(['host_os', 'size', 'checksum', 'url'])
VALID_BUNDLE_KEYS = frozenset([
    'name', 'version', 'revision', 'description', 'desc_url', 'stability',
    'recommended', 'archives',
])
VALID_MANIFEST_KEYS = frozenset(['manifest_version', 'bundles'])


class Error(Exception):
  ''' Exceptions raised within update_manifest '''
  pass


class Archive(dict):
  ''' A placeholder for sdk archive information. We derive Archive from
      dict so that it is easily serializable. '''
  def __init__(self, host_os_name):
    ''' Create a new archive for the given host-os name. '''
    self['host_os'] = host_os_name

  def CopyFrom(self, dict):
    ''' Update the content of the archive by copying values from the given
        dictionary.

    Args:
      dict: The dictionary whose values must be copied to the archive.'''
    for key, value in dict.items():
      self[key] = value

  def Validate(self):
    ''' Validate the content of the archive object. Raise an Error if
        an invalid or missing field is found. '''
    host_os = self.get('host_os', None)
    if host_os and host_os not in HOST_OS_LITERALS:
      raise Error('Invalid host-os name in archive')
    # Ensure host_os has a valid string. We'll use it for pretty printing.
    if not host_os:
      host_os = 'all (default)'
    if not self.get('url', None):
      raise Error('Archive "%s" has no URL' % host_os)
    # Verify that all key names are valid.
    for key, val in self.iteritems():
      if key not in VALID_ARCHIVE_KEYS:
        raise Error('Archive "%s" has invalid attribute "%s"' % (host_os, key))

  def _OpenURLStream(self):
    ''' Open a file-like stream for the archives's url. Raises an Error if the
        url can't be opened.

    Return:
      A file-like object from which the archive's data can be read.'''
    try:
      url_stream = urllib2.urlopen(self['url'])
    except urllib2.URLError:
      raise Error('"%s" is not a valid URL for archive %s' %
          (self['url'], self['host_os']))

    return url_stream

  def _Download(self, from_stream, to_stream=None):
    ''' '''
    sha1_hash = hashlib.sha1()
    size = 0
    while(1):
      data = from_stream.read(32768)
      if not data : break
      sha1_hash.update(data)
      size += len(data)
      if to_stream:
        to_stream.write(data)
    return sha1_hash.hexdigest(), size

  def ComputeSha1AndSize(self):
    ''' Compute the sha1 hash and size of the archive's data. Raises
        an Error if the url can't be opened.

    Return:
      A tuple (sha1, size) with the sha1 hash and data size respectively.'''
    stream = None
    sha1 = None
    size = 0
    try:
      stream = self._OpenURLStream()
      sha1, size = self._Download(stream)
    finally:
      if stream: stream.close()
    return sha1, size

  def DownloadToFile(self, dest_path):
    ''' Download the archive's data to a file at dest_path. As a side effect,
        computes the sha1 hash and data size, both returned as a tuple. Raises
        an Error if the url can't be opened, or an IOError exception if
        dest_path can't be opened.

    Args:
      dest_path: Path for the file that will receive the data.
    Return:
      A tuple (sha1, size) with the sha1 hash and data size respectively.'''
    sha1 = None
    size = 0
    with open(dest_path, 'wb') as to_stream:
      try:
        from_stream = self._OpenURLStream()
        sha1, size = self._Download(from_stream, to_stream)
      finally:
        if from_stream: from_stream.close()
    return sha1, size

  def Update(self, url):
    ''' Update the archive with the new url. Automatically update the
        archive's size and checksum fields. Raises an Error if the url is
        is invalid. '''
    self['url'] = url
    sha1, size = self.ComputeSha1AndSize()
    self['size'] = size
    self['checksum'] = {'sha1': sha1}


class Bundle(dict):
  ''' A placeholder for sdk bundle information. We derive Bundle from
      dict so that it is easily serializable.'''
  def __init__(self, name):
    ''' Create a new bundle with the given bundle name.

    Args:
      name: A name to give to the new bundle.'''
    self['archives'] = []
    self['name'] = name

  def CopyFrom(self, dict):
    ''' Update the content of the bundle by copying values from the given
        dictionary.

    Args:
      dict: The dictionary whose values must be copied to the bundle.'''
    for key, value in dict.items():
      if key == 'archives':
        archives = []
        for a in value:
          new_archive = Archive(a['host_os'])
          new_archive.CopyFrom(a)
          archives.append(new_archive)
        self['archives'] = archives
      else:
        self[key] = value

  def Validate(self):
    ''' Validate the content of the bundle. Raise an Error if an invalid or
        missing field is found. '''
    # Check required fields.
    if not self.get('name', None):
      raise Error('Bundle has no name')
    if not self.get('revision', None):
      raise Error('Bundle "%s" is missing a revision number' %
                              self['name'])
    if not self.get('description', None):
      raise Error('Bundle "%s" is missing a description' %
                              self['name'])
    if not self.get('stability', None):
      raise Error('Bundle "%s" is missing stability info' %
                              self['name'])
    if self.get('recommended', None) == None:
      raise Error('Bundle "%s" is missing the recommended field' %
                              self['name'])
    # Check specific values
    if self['stability'] not in STABILITY_LITERALS:
      raise Error('Bundle "%s" has invalid stability field: "%s"' %
                              (self['name'], self['stability']))
    if self['recommended'] not in YES_NO_LITERALS:
      raise Error(
          'Bundle "%s" has invalid recommended field: "%s"' %
          (self['name'], self['recommended']))
    # Verify that all key names are valid.
    for key, val in self.iteritems():
      if key not in VALID_BUNDLE_KEYS:
        raise Error('Bundle "%s" has invalid attribute "%s"' %
                    (self['name'], key))
    # Validate the archives
    for archive in self['archives']:
      archive.Validate()

  def GetArchive(self, host_os_name):
    ''' Retrieve the archive for the given host os.

    Args:
      host_os_name: name of host os whose archive must be retrieved.
    Return:
      An Archive instance or None if it doesn't exist.'''
    for archive in self['archives']:
      if archive['host_os'] == host_os_name:
        return archive
    return None

  def UpdateArchive(self, host_os, url):
    ''' Update or create  the archive for host_os with the new url.
        Automatically updates the archive size and checksum info by downloading
        the data from the given archive. Raises an Error if the url is invalid.

    Args:
      host_os: name of host os whose archive must be updated or created.
      url: the new url for the archive.'''
    archive = self.GetArchive(host_os)
    if not archive:
      archive = Archive(host_os_name=host_os)
      self['archives'].append(archive)
    archive.Update(url)

  def Update(self, options):
    ''' Update the bundle per content of the options.

    Args:
      options: options data. Attributes that are used are also deleted from
               options.'''
    # Check, set and consume individual bundle options.
    for option_key, attribute_key in OPTION_KEY_MAP.iteritems():
      option_val = getattr(options, option_key, None)
      if option_val:
        self[attribute_key] = option_val
        delattr(options, option_key);
    # Check and consume archive-url options.
    for option_key, host_os in OPTION_KEY_TO_PLATFORM_MAP.iteritems():
      platform_url = getattr(options, option_key, None)
      if platform_url:
        self.UpdateArchive(host_os, platform_url)
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
      raise Error("Manifest version too high: %s" %
                              self._manifest_data["manifest_version"])
    # Verify that all key names are valid.
    for key, val in self._manifest_data.iteritems():
      if key not in VALID_MANIFEST_KEYS:
        raise Error('Manifest has invalid attribute "%s"' % key)
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
      raise Error('Invalid bundle name: "%s"' %
                              options.bundle_name)
    bundle_name = options.bundle_name
    del options.bundle_name
    # Get the corresponding bundle, or create it.
    bundle = self._GetBundle(bundle_name)
    if not bundle:
      bundle = Bundle(name=bundle_name)
      self._AddBundle(bundle)
    bundle.Update(options)

  def _VerifyAllOptionsConsumed(self, options, bundle_name):
    ''' Verify that all the options have been used. Raise an exception if
        any valid option has not been used. Returns True if all options have
        been consumed.

    Args:
      options: the object containg the remaining unused options attributes.
      bundl_name: The name of the bundle, or None if it's missing.'''
    # Any option left in the list should have value = None
    for key, val in options.__dict__.items():
      if val != None:
        if bundle_name:
          raise Error('Unused option "%s" for bundle "%s"' % (key, bundle_name))
        else:
          raise Error('No bundle name specified')
    return True;


  def LoadManifestString(self, json_string):
    ''' Load a JSON manifest string. Raises an exception if json_string
        is not well-formed JSON.

    Args:
      json_string: a JSON-formatted string containing the previous manifest'''
    new_manifest = json.loads(json_string)
    for key, value in new_manifest.items():
      if key == 'bundles':
        # Remap each bundle in |value| to a Bundle instance
        bundles = []
        for b in value:
          new_bundle = Bundle(b['name'])
          new_bundle.CopyFrom(b)
          bundles.append(new_bundle)
        self._manifest_data[key] = bundles
      else:
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
    # Keep a copy of bundle_name, which will be consumed by UpdateBundle, for
    # use in _VerifyAllOptionsConsumed below.
    bundle_name = options.bundle_name
    if bundle_name is not None:
      self._UpdateBundle(options)
    self._VerifyAllOptionsConsumed(options, bundle_name)
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
        if it doesn't already exists. Raises an Error if the manifest doesn't
        validate after updating.

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
      '-B', '--bundle-revision', dest='bundle_revision',
      type='int',
      default=None,
      help='Required: Revision number for the bundle.')
  parser.add_option(
      '-b', '--bundle-version', dest='bundle_version',
      type='int',
      default=None,
      help='Optional: Version number for the bundle.')
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
  manifest_file = SDKManifestFile(filename)
  manifest_file.UpdateWithOptions(options)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
