#!/usr/bin/python
# Copyright 2010, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Download all Native Client compilers for this platform.

This module downloads multiple tgz's and expands them.
"""

import optparse
import os
import shutil
import sys
import tarfile
import tempfile
import urllib


def DownloadCompiler(src, dst, base_url, version):
  """Download one Native Client compiler and extract it.

  Arguments:
    src: name of the host + target platform to download
    dst: destination directory under compilers/
    base_url: base url to download compiler tarballs from
    version: version directory to select tarballs from
  """
  path = 'naclsdk_' + src + '.tgz'
  url = base_url + version + '/' + path

  # Pick target directory.
  script_dir = os.path.abspath(os.path.dirname(__file__))
  parent_dir = os.path.split(script_dir)[0]
  compilers_dir = os.path.join(parent_dir, 'compilers')
  target = os.path.join(compilers_dir, dst)

  # Create a temp dir for the tarball.
  try:
    tgz_dir = tempfile.mkdtemp()
  except OSError:
    pass
  tgz_filename = os.path.join(tgz_dir, path)

  print 'Downloading "%s" to "%s"...' % (url, tgz_filename)

  # Download it.
  urllib.urlretrieve(url, tgz_filename)

  # Setup target directory.
  try:
    os.makedirs(target)
  except OSError:
    pass

  # Extract compiler.
  tgz = tarfile.open(tgz_filename, 'r')
  for m in tgz:
    print 'Extracting "%s"' % m.name
    tgz.extract(m, target)
  tgz.close()

  print 'Extract complete.'
  
  # Clean up: remove the download dir.
  shutil.rmtree(tgz_dir)


PLATFORM_COLLAPSE = {
    'win32': 'win32',
    'cygwin': 'win32',
    'linux': 'linux',
    'linux2': 'linux',
    'darwin': 'darwin',
}


PLATFORM_MAPPING = {
    'win32': [
        ['win_x86', 'host_win/target_x86'],  # Multilib toolchain
        # TODO(dspringer): Remove these once they are fully deprecated.
        ['win_x86-32', 'host_win/target_x86-32'],
        ['win_x86-64', 'host_win/target_x86-64'],
    ],
    'linux': [
        ['linux_x86', 'host_linux/target_x86'],  # Multilib toolchain
        ['linux_arm-trusted', 'host_linux/target_arm-trusted'],
        ['linux_arm-untrusted', 'host_linux/target_arm-trusted'],
        # TODO(dspringer): Remove these once they are fully deprecated.
        ['linux_x86-32', 'host_linux/target_x86-32'],
        ['linux_x86-64', 'host_linux/target_x86-64'],
    ],
    'darwin': [
        # TODO(dspringer): Add the multilib toolchain for Mac when available.
        # TODO(dspringer): Remove these once they are fully deprecated.
        ['mac_x86-32', 'host_mac/target_x86-32'],
    ],
}


def main(argv):
  parser = optparse.OptionParser()
  parser.add_option(
      '-p', '--platforms', dest='platforms',
      default=sys.platform,
      help='comma separated list of platform compilers to download')
  parser.add_option(
      '-b', '--base-url', dest='base_url',
      default='http://build.chromium.org/buildbot/nacl_archive/nacl/compiler/',
      help='base url to download from')
  parser.add_option(
      '-v', '--version', dest='version',
      default='latest',
      help='which version of the compilers to download')
  parser.add_option(
      '-a', '--all-platforms', dest='platforms',
      action='store_const', const='win32,darwin,linux',
      help='download for all platforms')
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    print 'ERROR: invalid argument'
    sys.exit(1)

  platforms = set(options.platforms.split(','))
  for p in platforms:
    pfix = PLATFORM_COLLAPSE.get(p, p)
    if pfix in PLATFORM_MAPPING:
      for flavor in PLATFORM_MAPPING[pfix]:
        DownloadCompiler(flavor[0], flavor[1],
                         base_url=options.base_url,
                         version=options.version)
    else:
      print 'ERROR: Unknown platform "%s"!' % p
      sys.exit(1)


if __name__ == '__main__':
  main(sys.argv[1:])
