#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''A simple tool to update the Native Client SDK to the latest version'''

import cStringIO
import exceptions
import optparse
import os
import shutil
import subprocess
import sys
import update_manifest
import urllib2
import urlparse

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

# The following SSL certificates are used to validate the SSL connection
# to https://commondatastorage.googleapis.com
# TODO(mball): Validate at least one of these certificates.
# http://stackoverflow.com/questions/1087227/validate-ssl-certificates-with-python

EQUIFAX_SECURE_CA_CERTIFICATE='''-----BEGIN CERTIFICATE-----
MIIDIDCCAomgAwIBAgIENd70zzANBgkqhkiG9w0BAQUFADBOMQswCQYDVQQGEwJV
UzEQMA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2Vy
dGlmaWNhdGUgQXV0aG9yaXR5MB4XDTk4MDgyMjE2NDE1MVoXDTE4MDgyMjE2NDE1
MVowTjELMAkGA1UEBhMCVVMxEDAOBgNVBAoTB0VxdWlmYXgxLTArBgNVBAsTJEVx
dWlmYXggU2VjdXJlIENlcnRpZmljYXRlIEF1dGhvcml0eTCBnzANBgkqhkiG9w0B
AQEFAAOBjQAwgYkCgYEAwV2xWGcIYu6gmi0fCG2RFGiYCh7+2gRvE4RiIcPRfM6f
BeC4AfBONOziipUEZKzxa1NfBbPLZ4C/QgKO/t0BCezhABRP/PvwDN1Dulsr4R+A
cJkVV5MW8Q+XarfCaCMczE1ZMKxRHjuvK9buY0V7xdlfUNLjUA86iOe/FP3gx7kC
AwEAAaOCAQkwggEFMHAGA1UdHwRpMGcwZaBjoGGkXzBdMQswCQYDVQQGEwJVUzEQ
MA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2VydGlm
aWNhdGUgQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMBoGA1UdEAQTMBGBDzIwMTgw
ODIyMTY0MTUxWjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAUSOZo+SvSspXXR9gj
IBBPM5iQn9QwHQYDVR0OBBYEFEjmaPkr0rKV10fYIyAQTzOYkJ/UMAwGA1UdEwQF
MAMBAf8wGgYJKoZIhvZ9B0EABA0wCxsFVjMuMGMDAgbAMA0GCSqGSIb3DQEBBQUA
A4GBAFjOKer89961zgK5F7WF0bnj4JXMJTENAKaSbn+2kmOeUJXRmm/kEd5jhW6Y
7qj/WsjTVbJmcVfewCHrPSqnI0kBBIZCe/zuf6IWUrVnZ9NA2zsmWLIodz2uFHdh
1voqZiegDfqnc1zqcPGUIWVEX/r87yloqaKHee9570+sB3c4
-----END CERTIFICATE-----'''

GOOGLE_INTERNET_AUTHORITY_CERTIFICATE='''-----BEGIN CERTIFICATE-----
MIICsDCCAhmgAwIBAgIDC2dxMA0GCSqGSIb3DQEBBQUAME4xCzAJBgNVBAYTAlVT
MRAwDgYDVQQKEwdFcXVpZmF4MS0wKwYDVQQLEyRFcXVpZmF4IFNlY3VyZSBDZXJ0
aWZpY2F0ZSBBdXRob3JpdHkwHhcNMDkwNjA4MjA0MzI3WhcNMTMwNjA3MTk0MzI3
WjBGMQswCQYDVQQGEwJVUzETMBEGA1UEChMKR29vZ2xlIEluYzEiMCAGA1UEAxMZ
R29vZ2xlIEludGVybmV0IEF1dGhvcml0eTCBnzANBgkqhkiG9w0BAQEFAAOBjQAw
gYkCgYEAye23pIucV+eEPkB9hPSP0XFjU5nneXQUr0SZMyCSjXvlKAy6rWxJfoNf
NFlOCnowzdDXxFdF7dWq1nMmzq0yE7jXDx07393cCDaob1FEm8rWIFJztyaHNWrb
qeXUWaUr/GcZOfqTGBhs3t0lig4zFEfC7wFQeeT9adGnwKziV28CAwEAAaOBozCB
oDAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFL/AMOv1QxE+Z7qekfv8atrjaxIk
MB8GA1UdIwQYMBaAFEjmaPkr0rKV10fYIyAQTzOYkJ/UMBIGA1UdEwEB/wQIMAYB
Af8CAQAwOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybC5nZW90cnVzdC5jb20v
Y3Jscy9zZWN1cmVjYS5jcmwwDQYJKoZIhvcNAQEFBQADgYEAuIojxkiWsRF8YHde
BZqrocb6ghwYB8TrgbCoZutJqOkM0ymt9e8kTP3kS8p/XmOrmSfLnzYhLLkQYGfN
0rTw8Ktx5YtaiScRhKqOv5nwnQkhClIZmloJ0pC3+gz4fniisIWvXEyZ2VxVKfml
UUIuOss4jHg7y/j7lYe8vJD5UDI=
-----END CERTIFICATE-----'''

GOOGLE_USER_CONTENT_CERTIFICATE='''-----BEGIN CERTIFICATE-----
MIIEPDCCA6WgAwIBAgIKUaoA4wADAAAueTANBgkqhkiG9w0BAQUFADBGMQswCQYD
VQQGEwJVUzETMBEGA1UEChMKR29vZ2xlIEluYzEiMCAGA1UEAxMZR29vZ2xlIElu
dGVybmV0IEF1dGhvcml0eTAeFw0xMTA4MTIwMzQ5MjlaFw0xMjA4MTIwMzU5Mjla
MHExCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQHEw1N
b3VudGFpbiBWaWV3MRMwEQYDVQQKEwpHb29nbGUgSW5jMSAwHgYDVQQDFBcqLmdv
b2dsZXVzZXJjb250ZW50LmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA
uDmDvqlKBj6DppbENEuUmwVsHe5hpixV0bn6D+Ujy3mWUP9HtkO35/RmeFf4/y9i
nGy78uWO6tk9QY1PsPSiyZN6LgplalBdkTeODCGAieVOVJFhHQ0KM330qDy9sKNM
rwdMOfLPzkBMYPyr1C7CCm24j//aFiMCxD40bDQXRJkCAwEAAaOCAgQwggIAMB0G
A1UdDgQWBBRyHxPfv+Lnm2Kgid72ja3pOszMsjAfBgNVHSMEGDAWgBS/wDDr9UMR
Pme6npH7/Gra42sSJDBbBgNVHR8EVDBSMFCgTqBMhkpodHRwOi8vd3d3LmdzdGF0
aWMuY29tL0dvb2dsZUludGVybmV0QXV0aG9yaXR5L0dvb2dsZUludGVybmV0QXV0
aG9yaXR5LmNybDBmBggrBgEFBQcBAQRaMFgwVgYIKwYBBQUHMAKGSmh0dHA6Ly93
d3cuZ3N0YXRpYy5jb20vR29vZ2xlSW50ZXJuZXRBdXRob3JpdHkvR29vZ2xlSW50
ZXJuZXRBdXRob3JpdHkuY3J0MCEGCSsGAQQBgjcUAgQUHhIAVwBlAGIAUwBlAHIA
dgBlAHIwgdUGA1UdEQSBzTCByoIXKi5nb29nbGV1c2VyY29udGVudC5jb22CFWdv
b2dsZXVzZXJjb250ZW50LmNvbYIiKi5jb21tb25kYXRhc3RvcmFnZS5nb29nbGVh
cGlzLmNvbYIgY29tbW9uZGF0YXN0b3JhZ2UuZ29vZ2xlYXBpcy5jb22CEGF0Z2ds
c3RvcmFnZS5jb22CEiouYXRnZ2xzdG9yYWdlLmNvbYIUKi5zLmF0Z2dsc3RvcmFn
ZS5jb22CCyouZ2dwaHQuY29tgglnZ3BodC5jb20wDQYJKoZIhvcNAQEFBQADgYEA
XDvIl0/id823eokdFpLA8bL3pb7wQGaH0i3b29572aM7cDKqyxmTBbwi9mMMgbxy
E/St8DoSEQg3cJ/t2UaTXtw8wCrA6M1dS/RFpNLfV84QNcVdNhLmKEuZjpa+miUK
8OtYzFSMdfwXrbqKgkAIaqUs6m+LWKG/AQShp6DvTPo=
-----END CERTIFICATE-----'''

#------------------------------------------------------------------------------
# General Utilities

_debug_mode = False
_quiet_mode = False


def DebugPrint(msg):
  '''Display a message to stderr if debug printing is enabled

  Note: This function appends a newline to the end of the string

  Args:
    msg: A string to send to stderr in debug mode'''
  if _debug_mode:
    sys.stderr.write("%s\n" % msg)
    sys.stderr.flush()


def InfoPrint(msg):
  '''Display an informational message to stdout if not in quiet mode

  Note: This function appends a newline to the end of the string

  Args:
    mgs: A string to send to stdio when not in quiet mode'''
  if not _quiet_mode:
    sys.stdout.write("%s\n" % msg)
    sys.stdout.flush()


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


def ExtractInstaller(installer, outdir):
  '''Extract the SDK installer into a given directory

  If the outdir already exists, then this function deletes it

  Args:
    installer: full path of the SDK installer
    outdir: output directory where to extract the installer

  Raises:
    CalledProcessError - if the extract operation fails'''
  if os.path.exists(outdir):
    RemoveDir(outdir)

  if GetHostOS() == 'win':
    # Run the self-extracting installer in silent mode with a specified
    # output directory
    command = [installer, '/S', '/D=%s' % outdir]
  else:
    os.mkdir(outdir)
    command = ['tar', '-C', outdir, '--strip-components=1',
               '-xvzf', installer]

  subprocess.check_call(command)


def RemoveDir(outdir):
  '''Removes the given directory

  On Unix systems, this just runs shutil.rmtree, but on Windows, this doesn't
  work when the directory contains junctions (as does our SDK installer).
  Therefore, on Windows, it runs rmdir /S /Q as a shell command.  This always
  does the right thing on Windows.

  Args:
    outdir: The directory to delete

  Raises:
    CalledProcessError - if the delete operation fails on Windows
    OSError - if the delete operation fails on Linux
  '''

  InfoPrint('Removing %s' % outdir)
  if sys.platform == 'win32':
    subprocess.check_call(['rmdir /S /Q', outdir], shell=True)
  else:
    shutil.rmtree(outdir)


class ManifestTools(object):
  '''Wrapper class for supporting the SDK manifest file'''

  def __init__(self, options):
    self._options = options
    self._manifest = update_manifest.SDKManifest()

  def LoadManifest(self):
    DebugPrint("Running LoadManifest")
    try:
      # TODO(mball): Add certificate validation on the server
      url_stream = urllib2.urlopen(self._options.manifest_url)
    except urllib2.URLError:
      raise Error('Unable to open %s' % self._options.manifest_url)
    manifest_stream = cStringIO.StringIO()
    sha1, size = update_manifest.DownloadAndComputeHash(
        url_stream, manifest_stream)
    self._manifest.LoadManifestString(manifest_stream.getvalue())

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
  InfoPrint('Available bundles:\n')
  for bundle in bundles:
    InfoPrint(bundle['name'])
    for key, value in bundle.iteritems():
      if key not in ['archives', 'name']:
        InfoPrint("  %s: %s" % (key, value))


def Update(options, argv):
  '''Usage: %prog [options] update [target]

  Updates the Native Client SDK to a specified version.  By default, this
  command updates all the recommended components

  Targets:
    recommended: (default) Install/Update all recommended components
    all:         Install/Update all available components'''
  DebugPrint("Running Update command with: %s, %s" % (options, argv))

  parser = optparse.OptionParser(usage=Update.__doc__)
  parser.add_option(
      '-F', '--force', dest='force',
      default=False, action='store_true',
      help='Force updating existing components that already exist')
  (update_options, args) = parser.parse_args(argv)
  tools = ManifestTools(options)
  tools.LoadManifest()
  bundles = tools.GetBundles()
  for bundle in bundles:
    bundle_name = bundle['name']
    bundle_path = os.path.join(options.sdk_root_dir, bundle_name)
    if not update_options.force and os.path.exists(bundle_path):
      InfoPrint('Skipping bundle %s because directory already exists.'
                % bundle_name)
      InfoPrint('Use --force option to force overwriting existing directory')
      continue
    archive = bundle.GetArchive(GetHostOS())
    (scheme, host, path, _, _, _) = urlparse.urlparse(archive['url'])
    dest_filename = os.path.join(options.user_data_dir, path.split('/')[-1])
    InfoPrint('Downloading %s archive to %s' % (bundle_name, dest_filename))
    sha1, size = archive.DownloadToFile(os.path.join(options.user_data_dir,
                                                     dest_filename))
    if sha1 != archive['checksum']['sha1']:
      raise Error("SHA1 checksum mismatch.  Expected %s but got %s" %
                  (archive['checksum']['sha1'], sha1))
    if size != archive['size']:
      raise Error("Size mismatch on Archive.  Expected %s but got %s bytes" %
                  (archive['size'], size))
    InfoPrint('Extracting installer %s into %s' %
              (dest_filename, bundle_name))
    ExtractInstaller(dest_filename, bundle_path)
    os.remove(dest_filename)


#------------------------------------------------------------------------------
# Command-line interface


def main(argv):
  '''Main entry for the sdk_update utility'''
  parser = optparse.OptionParser(usage=GLOBAL_HELP)

  parser.add_option(
      '-U', '--manifest-url', dest='manifest_url',
      default='https://commondatastorage.googleapis.com/nativeclient-mirror/'
              'nacl/nacl_sdk/%s' % MANIFEST_FILENAME,
      help='override the default URL for the NaCl manifest file')
  parser.add_option(
      '-d', '--debug', dest='debug',
      default=False, action='store_true',
      help='enable displaying debug information to stderr')
  parser.add_option(
      '-q', '--quiet', dest='quiet',
      default=False, action='store_true',
      help='suppress displaying informational prints to stdout')
  parser.add_option(
      '-u', '--user-data-dir', dest='user_data_dir',
      # TODO(mball): the default should probably be in something like
      # ~/.naclsdk (linux), or ~/Library/Application Support/NaClSDK (mac),
      # or %HOMEPATH%\Application Data\NaClSDK (i.e., %APPDATA% on windows)
      default=os.path.dirname(os.path.abspath(__file__)),
      help="specify location of NaCl SDK's data directory")
  parser.add_option(
      '-s', '--sdk-root-dir', dest='sdk_root_dir',
      default=os.path.dirname(os.path.abspath(__file__)),
      help="location where the SDK bundles are installed")
  parser.add_option(
      '-v', '--version', dest='show_version',
      action='store_true',
      help='show version information and exit')

  COMMANDS = {
      'list': List,
      'update': Update,
  }

  # Separate global options from command-specific options
  global_argv = argv
  command_argv = []
  for index, arg in enumerate(argv):
    if arg in COMMANDS:
      global_argv = argv[:index]
      command_argv = argv[index:]
      break

  (options, args) = parser.parse_args(global_argv)
  args += command_argv

  global _debug_mode, _quiet_mode
  _debug_mode = options.debug
  _quiet_mode = options.quiet

  def PrintHelpAndExit(unused_options=None, unused_args=None):
    parser.print_help()
    exit(1)

  if options.show_version:
    print "Native Client SDK Updater, version %s.%s" % (MAJOR_REV, MINOR_REV)
    exit(0)

  if not args:
    print "Need to supply a command"
    PrintHelpAndExit()

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

  return 0  # Success


if __name__ == '__main__':
  return_value = 1
  try:
    return_value = main(sys.argv[1:])
  except exceptions.SystemExit:
    raise
  except Error as error:
    print "Error: %s" % error
  except:
    if not _debug_mode:
      print "Abnormal program termination: %s" % sys.exc_info()[1]
      print "Run again in debug mode (-d option) for stack trace."
    else:
      raise

  sys.exit(return_value)
