#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Utility to update the SDK manifest file in the build_tools directory'''

import json
import optparse

class SDKManifest(object):
  '''This class contains utilities for manipulation an SDK manifest string

  For ease of unit-testing, this class should not contain any file I/O.
  '''

  def __init__(self):
    '''Create a new SDKManifest object with default contents'''
    self.MANIFEST_VERSION = 1
    self._manifest = {
        "manifest_version": self.MANIFEST_VERSION,
        "bundles": []
    }


  def _ValidateManifest(self):
    '''Validate the Manifest file and raises an exception for problems'''
    if self._manifest["manifest_version"] > self.MANIFEST_VERSION:
      raise Exception("Manifest version too large: %s" %
                      self._manifest["manifest_version"])

    # TODO(mball,gwink) check other aspects of manifest file here


  def LoadManifestString(self, json_string):
    '''Load a JSON manifest string

    Args:
      json_string: a JSON-formatted string containing the previous manifest'''
    new_manifest = json.loads(json_string)
    for key, value in new_manifest.items():
      self._manifest[key] = value
    self._ValidateManifest()


  def GetManifestString(self):
    '''Returns the current JSON manifest object, pretty-printed'''
    pretty_string = json.dumps(self._manifest, sort_keys=False, indent=2)
    # json.dumps sometimes returns trailing whitespace and does not put
    # a newline at the end.  This code fixes these problems.
    pretty_lines = pretty_string.split('\n')
    return '\n'.join([line.rstrip() for line in pretty_lines]) + '\n'


def main():
  '''Main entry for update_manifest.py'''

  parser = optparse.OptionParser(
      usage="Usage: %prog [options] [manifest_file]")

  # TODO(mball,gwink) Add command-line options to allow specifying the
  # URLs of the SDK bundles.

  # TODO(mball,gwink) Add logic to load and save manifest file.
  return 0

if __name__ == '__main__':
  sys.exit(main())
