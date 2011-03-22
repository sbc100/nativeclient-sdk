#!/usr/bin/python
# Copyright 2011, Google Inc.
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

"""Assemble the final installer for windows."""

import build_utils
import optparse
import os
import shutil
import stat
import string
import subprocess
import sys

# TODO(NaCl SDK team):  Put tumbler back in the package when it's ported.
IGNORE_PATTERN = ('.download*', '.svn*', '.gitignore*', '.git*', 'tumbler*',
                  'cygwin*', 'toolchain*')
INSTALLER_DIRS = ['examples',
                  'project_templates',
                  'third_party']
INSTALLER_FILES = ['AUTHORS',
                   'COPYING',
                   'LICENSE',
                   'NOTICE',
                   'README']

def main(argv):
  print('generate_windows_installer is starting.')

  parser = optparse.OptionParser()
  parser.add_option(
      '--development', action='store_true', dest='development',
      default=False,
      help=('When set, the script will forego cleanup actions that can slow ' +
            'down subsequent runs.  Useful for testing.  Defaults to False.'))
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    print 'ERROR: invalid argument'
    sys.exit(1)

  if(options.development):
    print 'Running in development mode.'

  # Make sure that we are running python version 2.6 or higher
  (major, minor) = sys.version_info[:2]
  assert major == 2 and minor >= 6
  # Cache the current location so we can return here before removing the
  # temporary dirs.
  script_dir = os.path.abspath(os.path.dirname(__file__))
  home_dir = os.path.realpath(os.path.join(script_dir, '..', '..'))

  cygwin_dir = os.path.join(script_dir, '..', 'third_party', 'cygwin', 'bin')

  os.chdir(home_dir)
  os.chdir('src')

  version_dir = build_utils.VersionString()
  (parent_dir, _) = os.path.split(script_dir)
  deps_file = os.path.join(parent_dir, 'DEPS')
  NACL_REVISION = build_utils.GetNaClRevision(deps_file)

  # Create a temporary directory using the version string, then move the
  # contents of src to that directory, clean the directory of unwanted
  # stuff and finally create an installer.
  temp_dir = os.path.join(script_dir, 'installers_temp')
  installer_dir = os.path.join(temp_dir, version_dir)
  print('generate_windows_installer chose installer directory: %s' %
        (installer_dir))
  try:
    os.makedirs(installer_dir, mode=0777)
  except OSError:
    pass

  env = os.environ.copy()
  # TODO (mlinck, mball) make this unnecessary
  env['PATH'] = cygwin_dir + ';' + env['PATH']
  # TODO(mlinck, mball): maybe get rid of this
  variant = 'win_x86'
  toolchain = os.path.join('toolchain', variant)

  # Build the NaCl tools.
  print('generate_windows_installer is kicking off make_nacl_tools.py.')
  build_tools_dir = os.path.join(home_dir, 'src', 'build_tools')
  make_nacl_tools = os.path.join(build_tools_dir,
                                 'make_nacl_tools.py')
  make_nacl_tools_args = [sys.executable,
                          make_nacl_tools,
                          '--toolchain',
                          toolchain,
                          '--revision',
                          NACL_REVISION]
  if not options.development:
    make_nacl_tools_args.extend(['-c'])
  nacl_tools = subprocess.Popen(make_nacl_tools_args)
  assert nacl_tools.wait() == 0

  # Build c_salt
  # TODO(dspringer): add this part.
  c_salt_path = os.path.join(home_dir, 'src', 'c_salt')

  # Build the examples.
  print('generate_windows_installer is building examples.')
  example_path = os.path.join(home_dir, 'src', 'examples')
  make = subprocess.Popen('make install_prebuilt',
                          env=env,
                          cwd=example_path,
                          shell=True)
  assert make.wait() == 0

  # On windows we use copytree to copy the SDK into the build location
  # because there is no native tar and using cygwin's version has proven
  # to be error prone.

  # In case previous run didn't succeed, clean this out so copytree can make
  # its target directories.
  print('generate_windows_installer is cleaning out install directory.')
  shutil.rmtree(installer_dir)
  print('generate_windows_installer is copying contents to install directory.')
  for copy_source_dir in INSTALLER_DIRS:
    copy_target_dir = os.path.join(installer_dir, copy_source_dir)
    print("Copying %s to %s" % (copy_source_dir, copy_target_dir))
    shutil.copytree(copy_source_dir,
                    copy_target_dir,
                    symlinks=True,
                    ignore=shutil.ignore_patterns(*IGNORE_PATTERN))
  for copy_source_file in INSTALLER_FILES:
    copy_target_file = os.path.join(installer_dir, copy_source_file + '.txt')
    print("Copying %s to %s" % (copy_source_file, copy_target_file))
    with open(copy_source_file, "U") as source_file:
      text = source_file.read().replace("\n", "\r\n")
    with open(copy_target_file, "wb") as dest_file:
      dest_file.write(text)

  # Clean out the cruft.
  print('generate_windows_installer is cleaning up the installer directory.')
  os.chdir(installer_dir)

  # Make everything read/write (windows needs this).
  for root, dirs, files in os.walk('.'):
    for d in dirs:
      os.chmod(os.path.join(root, d), stat.S_IWRITE | stat.S_IREAD)
    for f in files:
      os.chmod(os.path.join(root, f), stat.S_IWRITE | stat.S_IREAD)

  print('generate_windows_installer is creating the installer archive')
  # Now that the SDK directory is copied and cleaned out, tar it all up using
  # the native platform tar.
  os.chdir(temp_dir)

  # Set the default shell command and output name.
  ar_cmd = ('tar cvzf %(ar_name)s %(input)s && cp %(ar_name)s %(output)s'
            ' && chmod 644 %(output)s')
  ar_name = 'nacl-sdk.tgz'

  # archive will be created in src\build_tools,
  # make_native_client_sdk.sh will create the real nacl-sdk.exe
  archive = os.path.join(home_dir, 'src', 'build_tools', ar_name)
  tarball = subprocess.Popen(
      ar_cmd % (
           {'ar_name':ar_name,
            'input':version_dir,
            'output':archive.replace('\\', '/')}),
      env=env, shell=True)
  assert tarball.wait() == 0

  print('generate_windows_installer is creating the windows installer.')
  os.chdir(os.path.join(home_dir, 'src', 'build_tools'))
  if os.path.exists('done1'):
    os.remove('done1')
  for i in xrange(100):
    print "Trying to make a script: try %i..." % (i+1)
    exefile = subprocess.Popen([
        os.path.join('..', 'third_party', 'cygwin', 'bin', 'bash.exe'),
        'make_native_client_sdk.sh', '-V',
        build_utils.RawVersion(), '-v', '-n'])
    exefile.wait()
    if os.path.exists('done1'):
      print "NSIS script created - time to run makensis!"
      if os.path.exists('done2'):
        os.remove('done2')
      for j in xrange(100):
        print "Trying to make a script: try %i..." % (j+1)
        exefile2 = subprocess.Popen([
            os.path.join('..', 'third_party', 'cygwin', 'bin', 'bash.exe'),
            'make_native_client_sdk2.sh', '-V',
            build_utils.RawVersion(), '-v', '-n'])
        exefile2.wait()
        if os.path.exists('done2'):
          print "Installer created!"
          break
      else:
        print "Can not create installer (even after 100 tries)"
      break
  else:
    print "Can not create NSIS script (even after 100 tries)"

  # Clean up.
  os.chdir(home_dir)
  shutil.rmtree(temp_dir)


if __name__ == '__main__':
  main(sys.argv[1:])
