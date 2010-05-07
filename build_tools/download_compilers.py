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

"""Download all Native Client toolchains for this platform.

This module downloads multiple tgz's and expands them.
"""

import optparse
import os
import shutil
import stat
import subprocess
import sys
import tempfile
import time
import urllib


def DownloadToolchain(src, dst, base_url, version):
  """Download one Native Client toolchain and extract it.

  Arguments:
    src: name of the host + target platform to download
    dst: destination directory under toolchain/
    base_url: base url to download toolchain tarballs from
    version: version directory to select tarballs from
  """
  path = 'naclsdk_' + src + '.tgz'
  url = base_url + version + '/' + path

  # Pick target directory.
  script_dir = os.path.abspath(os.path.dirname(__file__))
  parent_dir = os.path.split(script_dir)[0]
  if sys.platform == 'win32':
    cygwin_dir = os.path.join(parent_dir, 'third_party', 'cygwin', 'bin')
  toolchain_dir = os.path.join(parent_dir, 'toolchain')
  target = os.path.join(toolchain_dir, dst)

  # Under Windows we need the tarballs for the make_native_client_sdk.sh
  if sys.platform == 'win32':
    tgz_dir = os.path.join(script_dir, 'packages')
  else:
    # Create a temp dir for the tarball.
    try:
      tgz_dir = tempfile.mkdtemp()
    except OSError:
      pass
  tgz_filename = os.path.join(tgz_dir, path)

  print 'Downloading "%s" to "%s"...' % (url, tgz_filename)
  sys.stdout.flush()

  # Download it.
  urllib.urlretrieve(url, tgz_filename)

  # Make sure the old cruft in the target is gone.
  # Special handling for windows.
  if sys.platform == 'win32':
    for root, dirs, files in os.walk(target):
      for d in dirs:
        os.chmod(os.path.join(root, d), stat.S_IWRITE | stat.S_IREAD)
      for f in files:
        os.chmod(os.path.join(root, f), stat.S_IWRITE | stat.S_IREAD)
  shutil.rmtree(target, True)

  # Setup target directory.
  try:
    os.makedirs(target)
  except OSError:
    pass

  # Decide environment to run in per platform.
  # This adds the assumption that cygwin is installed in the default location
  # when cooking the sdk for windows.
  env = os.environ.copy()
  if sys.platform == 'win32':
    env['PATH'] = cygwin_dir + ';' + env['PATH']

  # Extract toolchain.
  old_cwd = os.getcwd()
  os.chdir(tgz_dir)
  p = subprocess.Popen(
      'tar xfzv "%s" && ( mv sdk/nacl-sdk/* "%s" || mv */* "%s" )' %
          (path, target.replace('\\', '/'), target.replace('\\', '/')),
      env=env, shell=True) 
  p.communicate()
  assert p.returncode == 0
  os.chdir(old_cwd)

  print 'Extract complete.'
  
  if sys.platform == 'win32':
    # Create make.cmd files for trusted compilation
    p = subprocess.Popen(
        'bash %s/create_make_cmds.sh ' % (script_dir), env=env, shell=True)
    p.communicate()
    assert p.returncode == 0
  else:
    # Clean up: remove the download dir.
    shutil.rmtree(tgz_dir)


def InstallCygWin(cygwin_url):
  """Download Hermetic SDK and install it.

  Arguments:
    cygwin_url: cygwin url to download hermetic cygwin from
  """
  
  # Pick target directory.
  script_dir = os.path.abspath(os.path.dirname(__file__))
  parent_dir = os.path.split(script_dir)[0]
  cygwin_dir = os.path.join(parent_dir, 'third_party', 'cygwin')

  # Create a temp dir for the tarball.
  try:
    setup_dir = tempfile.mkdtemp()
  except OSError:
    pass
  setup_filename = os.path.join(setup_dir, 'CygSetup.exe')

  print 'Downloading "%s" to "%s"...' % (cygwin_url, setup_filename)
  sys.stdout.flush()

  # Download it.
  urllib.urlretrieve(cygwin_url, setup_filename)

  print 'Installing CygWin to "%s"...' % (cygwin_dir)
  sys.stdout.flush()

  # Extract cygwin.
  old_cwd = os.getcwd()
  os.chdir(setup_dir)
  p = subprocess.Popen(
      ['cmd', '/WAIT', '/C', 'CygSetup.exe', '/NACLSDKMAKE', '/S', '/D=%s' %
          (cygwin_dir)])
  p.communicate()
  assert p.returncode == 0
  os.chdir(old_cwd)

  print 'Extract complete.'
  
  # Clean up: remove the download dir.
  for i in xrange(100):
    try:
      print "Trying to remove dir: try %i..." % (i+1)
      shutil.rmtree(setup_dir)
      break
    except WindowsError:
      time.sleep(5)


PLATFORM_COLLAPSE = {
    'win32': 'win32',
    'cygwin': 'win32',
    'linux': 'linux',
    'linux2': 'linux',
    'darwin': 'darwin',
}


PLATFORM_MAPPING = {
    'win32': [
        ['win_x86', 'win_x86'],  # Multilib toolchain
    ],
    'linux': [
        ['linux_x86', 'linux_x86'],  # Multilib toolchain
        # TODO(dspringer): Add these in once they have stabilized.
        # ['linux_arm-trusted', 'host_linux/target_arm-trusted'],
        # ['linux_arm-untrusted', 'host_linux/target_arm-trusted'],
    ],
    'darwin': [
        ['mac_x86', 'mac_x86'], # Multilib toolchain
    ],
}


def main(argv):
  parser = optparse.OptionParser()
  parser.add_option(
      '-p', '--platforms', dest='platforms',
      default=sys.platform,
      help='comma separated list of platform toolchains to download')
  parser.add_option(
      '-b', '--base-url', dest='base_url',
      default='http://build.chromium.org/buildbot/nacl_archive/nacl/toolchain/',
      help='base url to download from')
  parser.add_option(
      '-c', '--cygwin-url', dest='cygwin_url',
      default='http://build.chromium.org/mirror/nacl/cygwin_mirror/hermetic_cygwin_1_7_5-1_0.exe',
      help='cygwin installer url to download from')
  parser.add_option(
      '-v', '--version', dest='version',
      default='latest',
      help='which version of the toolchain to download')
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
      if pfix == 'win32':
        InstallCygWin(cygwin_url=options.cygwin_url)
      for flavor in PLATFORM_MAPPING[pfix]:
        DownloadToolchain(flavor[0], flavor[1],
                          base_url=options.base_url,
                          version=options.version)
    else:
      print 'ERROR: Unknown platform "%s"!' % p
      sys.exit(1)


if __name__ == '__main__':
  main(sys.argv[1:])
