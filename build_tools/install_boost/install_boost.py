#! -*- python -*-
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Install boost headers into the toolchain, in such a way that it gets pulled
into the SDK installer.  Note that this script only installs boost headers, and
does not build any of the boost libraries that require building. Most boost
libraries are header-only, so it should suffice to append
<toolchain_dir>/usr/include directory to CPPPATH for projects using only such
libraries to work.
"""

import build_utils
import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
import urllib

from optparse import OptionParser

# The original boost distro can be found here:
# http://sourceforge.net/projects/boost/files/boost/1.47.0/\
#     boost_1_47_0.tar.gz/download
BOOST_URL = ('http://commondatastorage.googleapis.com/nativeclient-mirror'
             '/nacl/boost_1_47_0.tar.gz')
BOOST_PATH = 'boost_1_47_0'


def DownloadAndExtract(options, url, path):
  boost_path = os.path.abspath(os.path.join(options.working_dir, path))
  print 'Download: %s' % url
  try:
    (tgz_file, headers) = urllib.urlretrieve(url, '%s.tgz' % boost_path)
    tar = None
    try:
      tar = tarfile.open(tgz_file)
      tar.extractall(options.working_dir)
    finally:
      if tar:
        tar.close()
  except (URLError, ContentTooShortError):
    print 'Error retrieving %s' % url
    raise


# Install the boost headers into the toolchain.
def InstallBoost(options):
  options.cwd = os.getcwd()
  # Create a temporary working directory.  This is where all the tar files go
  # and where the packages get built prior to installation in the toolchain.
  options.working_dir = tempfile.mkdtemp(prefix='boost')
  try:
    DownloadAndExtract(options, BOOST_URL, BOOST_PATH)
  except:
    print "Error in download"
    return 1
  print 'Installing boost headers...'
  # Make sure the target directories exist.
  nacl_include = os.path.abspath(os.path.join(options.toolchain,
                                              'x86_64-nacl',
                                              'include'))
  build_utils.ForceMakeDirs(nacl_include)
  boost_path = os.path.abspath(os.path.join(options.working_dir, BOOST_PATH))
  # Copy the headers.
  dst_include_dir = os.path.join(nacl_include, 'boost')
  shutil.rmtree(dst_include_dir, ignore_errors=True)
  shutil.copytree(os.path.join(boost_path, 'boost'),
                  dst_include_dir,
                  symlinks=True)
  # Copy the license file.
  print 'Installing boost license...'
  shutil.copy(os.path.join(boost_path, 'LICENSE_1_0.txt'), dst_include_dir)
  # Clean up.
  shutil.rmtree(options.working_dir, ignore_errors=True)
  return 0


# Parse the command-line args and set up the options object.  There are two
# command-line switches:
#   --toolchain=<path to the platform-specific toolchain>
#               e.g.: --toolchain=../toolchain/mac-x86
#               default is 'toolchain'.
def main(argv):
  shell_env = os.environ.copy();

  parser = OptionParser()
  parser.add_option(
      '-t', '--toolchain', dest='toolchain',
      default=build_utils.TOOLCHAIN_AUTODETECT,
      help='where to install boost')
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
