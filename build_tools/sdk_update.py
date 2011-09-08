#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''A simple tool to update the Native Client SDK to the latest version'''

import exceptions
import optparse
import os
import sys
import update_manifest

#------------------------------------------------------------------------------
# Constants

MAJOR_REV = 1
MINOR_REV = 0

GLOBAL_HELP = '''Usage: %prog [options] command [command_options]

sdk_update is a simple utility that updates the Native Client (NaCl)
Software Developer's Kit (SDK).

Commands:
  help [command] - Get either general or command-specific help
  delete - Deletes a given bundle (not implemented yet)
  list - Lists the available bundles
  update - Updates the SDK to the latest recommended toolchains'''

MANIFEST_FILENAME='naclsdk_manifest.json'


#------------------------------------------------------------------------------
# General Utilities

_debug_print = False


def DebugPrint(msg):
  if _debug_print:
    sys.stderr.write("%s\n" % msg)
    sys.stderr.flush()


class Error(Exception):
  '''Generic error/exception for sdk_update module'''
  pass


def GetHostOS():
  '''Returns the host_os value that corresponds to the current host OS'''
  return {
      'linux2': 'linux',
      'darwin': 'mac',
      'cygwin': 'win',
      'win32':  'win'
  }[sys.platform]


class ManifestTools(object):
  '''Wrapper class for supporting the SDK manifest file'''
  def __init__(self, options):
    self._options = options
    self._manifest = None

  def LoadManifest(self):
    DebugPrint("Running LoadManifest")
    # TODO(mball): Download manifest file
    current_manifest_filename = self._options.manifest_url
    with open(current_manifest_filename, 'r') as file:
      current_manifest_string = file.read()
    self._manifest = update_manifest.SDKManifest()
    self._manifest.LoadManifestString(current_manifest_string)

  def GetBundles(self):
    return self._manifest._manifest_data['bundles']


#------------------------------------------------------------------------------
# Commands


def List(options, argv):
  '''Usage: %prog [options] list

  Lists the available SDK bundles that are available for download.'''
  DebugPrint("Running List command with: %s, %s" %(options, argv))

  parser = optparse.OptionParser(usage=List.__doc__)
  (list_options, args) = parser.parse_args(argv)
  tools = ManifestTools(options)
  tools.LoadManifest()
  bundles = tools.GetBundles()
  print 'Available bundles:\n'
  for bundle in bundles:
    print bundle['name']
    for key, value in bundle.iteritems():
      if key not in ['archives', 'name']:
        print "  %s: %s" % (key, value)


def Update(options, argv):
  '''Usage: %prog [options] update [target]

  Updates the Native Client SDK to a specified version.  By default, this
  command updates all the recommended components

  Targets:
    recommended: (default) Install/Update all recommended components
    all:         Install/Update all available components'''
  DebugPrint("Running Update command with: %s, %s" % (options, argv))

  parser = optparse.OptionParser(usage=Update.__doc__)
  # TODO(mball): Add update options here
  (update_options, args) = parser.parse_args(argv)
  tools = ManifestTools(options)
  tools.LoadManifest()


#------------------------------------------------------------------------------
# Command-line interface


def main(argv):
  '''Main entry for the sdk_update utility'''
  parser = optparse.OptionParser(usage=GLOBAL_HELP)

  parser.add_option(
      '-U', '--manifest-url', dest='manifest_url',
      default='http://commondatastorage.googleapis.com/nativeclient-mirror/'
              'nacl/nacl_sdk/%s' % MANIFEST_FILENAME,
      help='override the default URL for the NaCl manifest file')
  parser.add_option(
      '-d', '--debug', dest='debug',
      default=False, action='store_true',
      help='enable displaying debug information to stderr')
  parser.add_option(
      '-u', '--user-data-dir', dest='user_data_dir',
      # TODO(mball): the default should probably be in something like
      # ~/.naclsdk (linux), or ~/Library/Application Support/NaClSDK (mac),
      # or %HOMEPATH%\Application Data\NaClSDK (i.e., %APPDATA% on windows)
      default=os.path.dirname(os.path.abspath(__file__)),
      help="specify location of NaCl SDK's data directory")
  parser.add_option(
      '-v', '--version', dest='show_version',
      action='store_true',
      help='show version information and exit')

  (options, args) = parser.parse_args(argv)

  global _debug_print
  _debug_print = options.debug

  def PrintHelpAndExit(unused_options=None, unused_args=None):
    parser.print_help()
    exit(1)

  if options.show_version:
    print "Native Client SDK Updater, version %s.%s" % (MAJOR_REV, MINOR_REV)
    exit(0)

  if not args:
    print "Need to supply a command"
    PrintHelpAndExit()

  COMMANDS = {
      'list': List,
      'update': Update,
  }

  def DefaultHandler(unused_options=None, unused_args=None):
    print "Unknown Command: %s" % args[0]
    PrintHelpAndExit()

  def InvokeCommand(args):
    command = COMMANDS.get(args[0], DefaultHandler)
    command(options, args[1:])

  if args[0] == 'help':
    if len(args) == 1:
      PrintHelpAndExit()
    else:
      InvokeCommand([args[1], '-h'])
  else:
    InvokeCommand(args)

  return 0

if __name__ == '__main__':
  return_value = 1
  try:
    return_value = main(sys.argv[1:])
  except exceptions.SystemExit:
    raise
  except:
    if not _debug_print:
      print "Abnormal program termination: %s" % sys.exc_info()[1]
      print "Run again in debug mode (-d option) for stack trace."
    else:
      raise

  sys.exit(return_value)
