#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Assemble the final installer for each platform.

At this time this is just a tarball.
"""

import build_utils
import installer_contents
import optparse
import os
import re
import shutil
import stat
import string
import subprocess
import sys

EXCLUDE_DIRS = ['.download',
                '.svn']

INSTALLER_NAME = 'nacl-sdk.tgz'

# These are extra files that are exclusive to the Mac and Linux installers.
EXTRA_POSIX_INSTALLER_CONTENTS = [
    'toolchain/',
]

# A list of all platforms that should use the Windows-based build strategy
# (which makes a self-extracting zip instead of a tarball).
WINDOWS_BUILD_PLATFORMS = ['cygwin', 'win32']

# A list of files from third_party/valgrind that should be included in the SDK.


# Return True if |file| should be excluded from the tarball.
def ExcludeFile(dir, file):
  return (file.startswith('.DS_Store') or
          file.startswith('._') or file == "make.cmd" or
          file == 'DEPS' or file == 'codereview.settings' or
          (file == "httpd.cmd"))

def main(argv):
  bot = build_utils.BotAnnotator()
  bot.Print('generate_installers is starting.')

  parser = optparse.OptionParser()
  parser.add_option(
      '--development', action='store_true', dest='development',
      default=False,
      help=('When set, the script will forego cleanup actions that can slow ' +
            'down subsequent runs.  Useful for testing.  Defaults to False.'))
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    bot.Print('ERROR: invalid argument')
    sys.exit(1)

  # Cache the current location so we can return here before removing the
  # temporary dirs.
  script_dir = os.path.abspath(os.path.dirname(__file__))
  home_dir = os.path.realpath(os.path.join(script_dir, '..', '..'))

  os.chdir(home_dir)
  os.chdir('src')

  version_dir = build_utils.VersionString()
  (parent_dir, _) = os.path.split(script_dir)
  deps_file = os.path.join(parent_dir, 'DEPS')
  NACL_REVISION = build_utils.GetNaClRevision(deps_file)

  # Create a temporary directory using the version string, then move the
  # contents of src to that directory, clean the directory of unwanted
  # stuff and finally tar it all up using the platform's tar.  There seems to
  # be a problem with python's tarfile module and symlinks.
  temp_dir = os.path.join(script_dir, 'installers_temp')
  installer_dir = os.path.join(temp_dir, version_dir)
  bot.Print('generate_installers chose installer directory: %s' %
            (installer_dir))
  try:
    os.makedirs(installer_dir, mode=0777)
  except OSError:
    pass

  # Decide environment to run in per platform.
  env = os.environ.copy()

  if sys.platform == 'darwin':
    variant = 'mac_x86'
  elif sys.platform in ['linux', 'linux2']:
    variant = 'linux_x86'
  toolchain = os.path.join('toolchain', variant)

  # Build the NaCl tools.
  bot.Print('generate_installers is kicking off make_nacl_tools.py.')
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
  bot.BuildStep('build examples')
  bot.Print('generate_installers is building examples.')
  example_path = os.path.join(home_dir, 'src', 'examples')
  make = subprocess.Popen('./scons install_prebuilt',
                          env=env,
                          cwd=example_path,
                          shell=True)
  assert make.wait() == 0

  # Use native tar to copy the SDK into the build location
  # because copytree has proven to be error prone and is not supported on mac.
  # We use a buffer for speed here.  -1 causes the default OS size to be used.
  bot.BuildStep('copy to install dir')
  bot.Print('generate_installers is copying contents to install directory.')
  tar_src_dir = os.path.realpath(os.curdir)
  all_contents = installer_contents.INSTALLER_CONTENTS + \
                 installer_contents.DOCUMENTATION_FILES + \
                 EXTRA_POSIX_INSTALLER_CONTENTS
  all_contents_string = string.join(
      installer_contents.GetFilesFromPathList(all_contents) +
      installer_contents.GetDirectoriesFromPathList(all_contents),
      ' ')
  tar_cf = subprocess.Popen('tar cf - %s' % all_contents_string,
                            bufsize=-1,
                            cwd=tar_src_dir, env=env, shell=True,
                            stdout=subprocess.PIPE)
  tar_xf = subprocess.Popen('tar xfv -',
                            cwd=installer_dir, env=env, shell=True,
                            stdin=tar_cf.stdout)
  assert tar_xf.wait() == 0
  assert tar_cf.poll() == 0

  # Clean out the cruft.
  bot.Print('generate_installers is cleaning up the installer directory.')
  os.chdir(installer_dir)

  # This loop prunes the result of os.walk() at each excluded dir, so that it
  # doesn't descend into the excluded dir.
  bot.Print('generate_installers is pruning installer directory')
  for root, dirs, files in os.walk('.'):
    rm_dirs = []
    for excl in EXCLUDE_DIRS:
      if excl in dirs:
        dirs.remove(excl)
        rm_dirs.append(os.path.join(root, excl))
    for rm_dir in rm_dirs:
      shutil.rmtree(rm_dir)
    rm_files = [os.path.join(root, f) for f in files if ExcludeFile(root, f)]
    for rm_file in rm_files:
      os.remove(rm_file)

  # Update the README file with date and version number
  build_utils.UpdateReadMe(os.path.join(installer_dir, 'README'))

  bot.BuildStep('create archive')
  bot.Print('generate_installers is creating the installer archive')
  # Now that the SDK directory is copied and cleaned out, tar it all up using
  # the native platform tar.
  os.chdir(temp_dir)

  # Set the default shell command and output name.
  ar_cmd = ('tar cvzf %(INSTALLER_NAME)s %(input)s && cp %(INSTALLER_NAME)s '
            '%(output)s && chmod 644 %(output)s')

  archive = os.path.join(home_dir, INSTALLER_NAME)
  tarball = subprocess.Popen(
      ar_cmd % (
           {'INSTALLER_NAME':INSTALLER_NAME,
            'input':version_dir,
            'output':archive}),
      env=env, shell=True)
  assert tarball.wait() == 0

  # Clean up.
  os.chdir(home_dir)
  shutil.rmtree(temp_dir)


if __name__ == '__main__':
  if(sys.platform in WINDOWS_BUILD_PLATFORMS):
    import generate_windows_installer
    generate_windows_installer.main(sys.argv[1:])
  else:
    main(sys.argv[1:])
