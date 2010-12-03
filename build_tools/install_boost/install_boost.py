#!/usr/bin/python
# Copyright (c) 2010 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.

"""Install boost into the toolchain, in such a way that it gets pulled into the
SDK installer.  By default this script builds both the 32- and 64-bit versions
of boost.  Note that this script only installs boost headers, and does not build
any of the boost libraries (such as boost_datetime).
"""

import build_utils
import os
import shutil
import subprocess
import sys
import tempfile
import urllib

from optparse import OptionParser

# Default value for the --bit-spec command line argument.
BIT_SPEC_DEFAULT = "32,64"

# The original boost distro can be found here:
# http://sourceforge.net/projects/boost/files/boost/1.43.0/\
#     boost_1_43_0.tar.gz/download
BOOST_URL = ("http://commondatastorageapis.appspot.com/nativeclient-mirror/"
             "/nacl/boost_1_43_0.tar.gz")
BOOST_PATH = "boost_1_43_0"
BOOST_PATCH_FILE = "nacl-boost_1_43_0.patch"


def DownloadAndExtract(options, url, path):
  os.chdir(options.working_dir)
  print "Download: %s" % url
  try:
    (zip_file, headers) = urllib.urlretrieve(url, '%s.tgz' % path)
    p = subprocess.Popen('tar xzf %s' % (zip_file),
                         env=options.shell_env,
                         shell=True)
    assert p.wait() == 0
  except (URLError, ContentTooShortError):
    os.chdir(options.cwd)
    print "Error retrieving %s" % url
    raise

  os.chdir(options.cwd)


# Apply the patch file to the extracted boost pacakge.
def Patch(options, patch_file):
  print "Patching %s with: %s" % (options.working_dir, patch_file)
  p = subprocess.Popen('patch -p0 < %s' % (patch_file),
                       cwd=options.working_dir,
                       env=options.shell_env,
                       shell=True)
  assert p.wait() == 0


# Install the boost headers into the toolchain.
def BuildAndInstall(options):
  # Build and install for each bit-spec.
  for bit_spec in options.bit_spec.split(','):
    # 32-bits is treated specially, due to being the empty string in legacy
    # code.
    nacl_spec = bit_spec == '32' and 'nacl' or ('nacl%s' % bit_spec)
    print 'Installing boost for NaCl spec: %s.' % nacl_spec
    # Make sure the target directories exist.
    nacl_usr_include = os.path.join(options.toolchain,
                                    nacl_spec,
                                    'usr',
                                    'include')
    build_utils.ForceMakeDirs(nacl_usr_include)
    boost_path = os.path.join(options.working_dir, BOOST_PATH)
    boost_tar_excludes = "--exclude='asio.hpp' \
                          --exclude='asio' \
                          --exclude='mpi.hpp' \
                          --exclude='mpi'"
    tar_cf = subprocess.Popen("tar cf - %s boost" % boost_tar_excludes,
                              cwd=boost_path,
                              env=options.shell_env,
                              shell=True,
                              stdout=subprocess.PIPE)
    tar_xf = subprocess.Popen("tar xfp -",
                              cwd=nacl_usr_include,
                              env=options.shell_env,
                              shell=True,
                              stdin=tar_cf.stdout)
    tar_copy_err = tar_xf.communicate()[1]


def InstallBoost(options):
  options.cwd = os.getcwd()
  # Create a temporary working directory.  This is where all the tar files go
  # and where the packages get built prior to installation in the toolchain.
  options.working_dir = tempfile.mkdtemp(prefix='boost')
  try:
    DownloadAndExtract(options, BOOST_URL, BOOST_PATH)
    print "Patching..."
    Patch(options, os.path.join(options.script_dir, BOOST_PATCH_FILE))
  except:
    print "Error in download and patch"
    return 1

  BuildAndInstall(options)
  # Clean up.
  shutil.rmtree(options.working_dir, ignore_errors=True)
  return 0


# Parse the command-line args and set up the options object.  There are two
# command-line switches:
#   --toolchain=<path to the platform-specific toolchain>
#               e.g.: --toolchain=../toolchain/mac-x86
#               default is 'toolchain'.
#   --bit-spec=<comma-separated list of instruction set bit-widths
#              e.g.: --bit_spec=32,64
#              default is "32,64" which means build 32- and 64-bit versions
#              of boost.
def main(argv):
  shell_env = build_utils.GetShellEnvironment();
  if not build_utils.CheckPatchVersion(shell_env):
    sys.exit(0)

  parser = OptionParser()
  parser.add_option(
      '-t', '--toolchain', dest='toolchain',
      default=build_utils.TOOLCHAIN_AUTODETECT,
      help='where to install boost')
  parser.add_option(
      '-b', '--bit-spec', dest='bit_spec',
      default=BIT_SPEC_DEFAULT,
      help='comma separated list of instruction set bit-widths')
  (options, args) = parser.parse_args(argv)
  if args:
    print 'ERROR: invalid argument: %s' % str(args)
    parser.print_help()
    sys.exit(1)

  options.shell_env = shell_env
  options.script_dir = os.path.abspath(os.path.dirname(__file__))
  options.toolchain = build_utils.NormalizeToolchain(options.toolchain)
  print "Installing boost into toolchain %s" % options.toolchain

  return InstallBoost(options)


if __name__ == '__main__':
  main(sys.argv[1:])
