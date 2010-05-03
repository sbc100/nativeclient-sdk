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

"""Assemble the final installer for each platform.

At this time this is just a tarball.
"""

import os
import re
import shutil
import stat
import subprocess
import sys
import tempfile

EXCLUDE_DIRS = ['.download', '.svn', 'build_tools', 'packages', 'scons-out']


# A list of all platforms that should use the Windows-based build strategy
# (which makes a self-extracting zip instead of a tarball).
WINDOWS_BUILD_PLATFORMS = ['cygwin', 'win32']


# Return True if |file| should be excluded from the tarball.
def ExcludeFile(file):
  return (file.startswith('.DS_Store') or
          re.search('^\._', file) or
          file == 'DEPS' or file == 'codereview.settings')


# Note that this function has to be run from within a subversion working copy.
def SVNRevision():
  p = subprocess.Popen('svn info', shell=True, stdout=subprocess.PIPE)
  svn_info = p.communicate()[0]
  m = re.search('Revision: ([0-9]+)', svn_info)
  if m:
    return int(m.group(1))
  else:
    return 0


def VersionString():
  rev = SVNRevision()
  build_number = os.environ.get('BUILD_NUMBER', '0')
  return 'native_client_sdk_0_1_%d_%s' % (rev, build_number)


def RawVersion():
  rev = SVNRevision()
  build_number = os.environ.get('BUILD_NUMBER', '0')
  return '0.1.%d.%s' % (rev, build_number)


def main(argv):
  # Cache the current location so we can return here before removing the
  # temporary dirs.
  script_dir = os.path.abspath(os.path.dirname(__file__))
  home_dir = os.path.realpath(os.path.join(script_dir, '..', '..'))

  os.chdir(home_dir)
  os.chdir('src')

  version_dir = VersionString()

  # Create a temporary directory using the version string, then move the
  # contents of src to that directory, clean the directory of unwanted
  # stuff and finally tar it all up using the platform's tar.  There seems to
  # be a problem with python's tarfile module and symlinks.
  temp_dir = tempfile.mkdtemp();
  installer_dir = os.path.join(temp_dir, version_dir)
  try:
    os.makedirs(installer_dir, mode=0777)
  except OSError:
    pass

  # Windows only: remove toolchain and cygwin. They will be added by
  # make_native_client_sdk.sh
  if sys.platform in WINDOWS_BUILD_PLATFORMS:
    EXCLUDE_DIRS.extend(['cygwin', 'toolchain'])

  # Decide environment to run in per platform.
  # This adds the assumption that cygwin is installed in the default location
  # when cooking the sdk for windows.
  env = os.environ.copy()
  if sys.platform == 'win32':
    env['PATH'] = r'c:\cygwin\bin;' + env['PATH']

  # Build the examples.
  example_path = os.path.join(home_dir, 'src/examples')
  make = subprocess.Popen('make install_prebuilt',
                          env=env,
                          cwd=example_path,
                          shell=True)
  make_err = make.communicate()[1]

  # Use native tar to copy the SDK into the build location; this preserves
  # symlinks.
  tar_src_dir = os.path.realpath(os.curdir)
  tar_cf = subprocess.Popen('tar cf - .',
                            cwd=tar_src_dir, env=env, shell=True,
                            stdout=subprocess.PIPE)
  tar_xf = subprocess.Popen('tar xfv -',
                            cwd=installer_dir, env=env, shell=True,
                            stdin=tar_cf.stdout)
  tar_copy_err = tar_xf.communicate()[1]

  # Clean out the cruft.
  os.chdir(installer_dir)

  # Make everything read/write (windows needs this).
  if sys.platform == 'win32':
    for root, dirs, files in os.walk('.'):
      for d in dirs:
        os.chmod(os.path.join(root, d), stat.S_IWRITE | stat.S_IREAD)
      for f in files:
        os.chmod(os.path.join(root, f), stat.S_IWRITE | stat.S_IREAD)

  # This loop prunes the result of os.walk() at each excluded dir, so that it
  # doesn't descend into the excluded dir.
  for root, dirs, files in os.walk('.'):
    rm_dirs = []
    for excl in EXCLUDE_DIRS:
      if excl in dirs:
        dirs.remove(excl)
        rm_dirs.append(os.path.join(root, excl))
    for rm_dir in rm_dirs:
      shutil.rmtree(rm_dir);
    rm_files = [os.path.join(root, f) for f in files if ExcludeFile(f)]
    for rm_file in rm_files:
      os.remove(rm_file);

  # Now that the SDK directory is copied and cleaned out, tar it all up using
  # the native platform tar.
  os.chdir(temp_dir)

  # Set the default shell command and output name.
  ar_cmd = ('tar cvzf %(ar_name)s %(input)s && cp %(ar_name)s %(output)s'
            ' && chmod 644 %(output)s')
  ar_name = 'nacl-sdk.tgz'

  # Windows only: change the command and output filename--we want to create
  # a self-extracting 7zip archive, not a tarball.

  # Windows only: archive will be created in src\build_tools,
  # make_native_client_sdk.sh will create the real nacl-sdk.exe
  if sys.platform in WINDOWS_BUILD_PLATFORMS:
    archive = os.path.join(home_dir, 'src', 'build_tools', ar_name)
  else:
    archive = os.path.join(home_dir, ar_name)
  tarball = subprocess.Popen(
      ar_cmd % (
           {'ar_name':ar_name,
            'input':version_dir,
            'output':archive.replace('\\', '/')}),
      env=env, shell=True)
  tarball_err = tarball.communicate()[1]


  # Windows only: use make_native_client_sdk.sh to create installer
  if sys.platform in WINDOWS_BUILD_PLATFORMS:
    os.chdir(os.path.join(home_dir, 'src', 'build_tools'))
    exefile = subprocess.Popen([
        os.path.join('..', 'third_party', 'cygwin', 'bin', 'bash.exe'),
        'make_native_client_sdk.sh', '-V', RawVersion(), '-v', '-n'])
    exefile_err = exefile.communicate()[1]
    exefile2 = subprocess.Popen([
        os.path.join('..', 'third_party', 'cygwin', 'bin', 'bash.exe'),
        'make_native_client_sdk2.sh', '-V', RawVersion(), '-v', '-n'])
    exefile_err2 = exefile2.communicate()[1]

  # Clean up.
  os.chdir(home_dir)
  shutil.rmtree(temp_dir)


if __name__ == '__main__':
  main(sys.argv[1:])
