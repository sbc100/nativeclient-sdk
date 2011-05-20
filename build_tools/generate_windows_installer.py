#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Assemble the final installer for windows."""

import build_utils
import installer_contents
import optparse
import os
import shutil
import stat
import string
import subprocess
import sys

IGNORE_PATTERN = ('.download*', '.svn*')

# These are extra files that are exclusive to the Windows installer.
EXTRA_WINDOWS_INSTALLER_CONTENTS = [
    'examples/httpd.cmd',
    'examples/scons.bat',
    'project_templates/scons.bat',
]

def main(argv):
  bot = build_utils.BotAnnotator()
  bot.Print('generate_windows_installer is starting.')

  parser = optparse.OptionParser()
  parser.add_option(
      '--development', action='store_true', dest='development',
      default=False,
      help=('When set, the script will forego cleanup actions that can slow ' +
            'down subsequent runs.  Useful for testing.  Defaults to False.'))
  parser.add_option(
      '-j', '--jobs', dest='jobs', default='1',
      help='Number of parallel jobs to use while building')
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    bot.Print('ERROR: invalid argument')
    sys.exit(1)

  if(options.development):
    bot.Print('Running in development mode.')

  # Make sure that we are running python version 2.6 or higher
  (major, minor) = sys.version_info[:2]
  assert major == 2 and minor >= 6
  # Cache the current location so we can return here before removing the
  # temporary dirs.
  script_dir = os.path.abspath(os.path.dirname(__file__))
  home_dir = os.path.realpath(os.path.dirname(os.path.dirname(script_dir)))

  cygwin_dir = os.path.join(home_dir,
                            'src',
                            'third_party',
                            'cygwin',
                            'bin')

  version_dir = build_utils.VersionString()
  parent_dir = os.path.dirname(script_dir)
  deps_file = os.path.join(parent_dir, 'DEPS')
  NACL_REVISION = build_utils.GetNaClRevision(deps_file)

  # Create a temporary directory using the version string, then move the
  # contents of src to that directory, clean the directory of unwanted
  # stuff and finally create an installer.
  temp_dir = os.path.join(script_dir, 'installers_temp')
  installer_dir = os.path.join(temp_dir, version_dir)
  bot.Print('generate_windows_installer chose installer directory: %s' %
            (installer_dir))
  try:
    os.makedirs(installer_dir, mode=0777)
  except OSError:
    pass

  # TODO(mlinck, mball): maybe get rid of this
  variant = 'win_x86'
  toolchain = os.path.join('toolchain', variant)

  # Build the NaCl tools.
  bot.Print('generate_windows_installer is kicking off make_nacl_tools.py.')
  build_tools_dir = os.path.join(home_dir, 'src', 'build_tools')
  make_nacl_tools = os.path.join(build_tools_dir,
                                 'make_nacl_tools.py')
  make_nacl_tools_args = [sys.executable,
                          make_nacl_tools,
                          '--toolchain',
                          toolchain,
                          '--revision',
                          NACL_REVISION,
                          '--jobs',
                          options.jobs]
  if not options.development:
    make_nacl_tools_args.extend(['-c'])
  subprocess.check_call(make_nacl_tools_args, cwd=os.path.join(home_dir, 'src'))

  # Build c_salt
  # TODO(dspringer): add this part.
  c_salt_path = os.path.join(home_dir, 'src', 'c_salt')

  # Build the examples.
  bot.BuildStep('build examples')
  bot.Print('generate_windows_installer is building examples.')
  example_path = os.path.join(home_dir, 'src', 'examples')
  # Make sure the examples are clened out before creating the prebuilt
  # artifacts.
  subprocess.check_call(['scons.bat', '-c', 'install_prebuilt'],
                         cwd=example_path)
  subprocess.check_call(['scons.bat', 'install_prebuilt'],
                         cwd=example_path)

  # On windows we use copytree to copy the SDK into the build location
  # because there is no native tar and using cygwin's version has proven
  # to be error prone.

  # In case previous run didn't succeed, clean this out so copytree can make
  # its target directories.
  bot.BuildStep('copy to install dir')
  bot.Print('generate_windows_installer is cleaning out install directory.')
  shutil.rmtree(installer_dir)
  bot.Print('generate_windows_installer: copying files to install directory.')
  all_contents = installer_contents.INSTALLER_CONTENTS + \
                 EXTRA_WINDOWS_INSTALLER_CONTENTS
  for copy_source_dir in installer_contents.GetDirectoriesFromPathList(
      all_contents):
    copy_target_dir = os.path.join(installer_dir, copy_source_dir)
    bot.Print("Copying %s to %s" % (copy_source_dir, copy_target_dir))
    shutil.copytree(copy_source_dir,
                    copy_target_dir,
                    symlinks=True,
                    ignore=shutil.ignore_patterns(*IGNORE_PATTERN))
  for copy_source_file in installer_contents.GetFilesFromPathList(
      all_contents):
    copy_target_file = os.path.join(installer_dir, copy_source_file)
    bot.Print("Copying %s to %s" % (copy_source_file, copy_target_file))
    if not os.path.exists(os.path.dirname(copy_target_file)):
      os.makedirs(os.path.dirname(copy_target_file))
    shutil.copy(copy_source_file, copy_target_file)

  # Do special processing on the user-readable documentation files.
  for copy_source_file in installer_contents.DOCUMENTATION_FILES:
    copy_target_file = os.path.join(installer_dir, copy_source_file + '.txt')
    bot.Print("Copying %s to %s" % (copy_source_file, copy_target_file))
    with open(copy_source_file, "U") as source_file:
      text = source_file.read().replace("\n", "\r\n")
    with open(copy_target_file, "wb") as dest_file:
      dest_file.write(text)

  # Update the README.txt file with date and version number
  build_utils.UpdateReadMe(os.path.join(installer_dir, 'README.txt'))

  # Clean out the cruft.
  bot.Print('generate_windows_installer: cleaning up installer directory.')

  # Make everything read/write (windows needs this).
  for root, dirs, files in os.walk(installer_dir):
    for d in dirs:
      os.chmod(os.path.join(root, d), stat.S_IWRITE | stat.S_IREAD)
    for f in files:
      os.chmod(os.path.join(root, f), stat.S_IWRITE | stat.S_IREAD)

  bot.BuildStep('create archive')
  bot.Print('generate_windows_installer is creating the installer archive')
  # Now that the SDK directory is copied and cleaned out, tar it all up using
  # the native platform tar.

  # Set the default shell command and output name.
  ar_cmd = ('tar cvzf %(ar_name)s %(input)s && cp %(ar_name)s %(output)s'
            ' && chmod 644 %(output)s')
  ar_name = 'nacl-sdk.tgz'

  cygwin_env = os.environ.copy()
  # TODO (mlinck, mball) make this unnecessary
  cygwin_env['PATH'] = cygwin_dir + ';' + cygwin_env['PATH']
  # archive will be created in src\build_tools,
  # make_native_client_sdk.sh will create the real nacl-sdk.exe
  archive = os.path.join(home_dir, 'src', 'build_tools', ar_name)
  tarball = subprocess.Popen(
      ar_cmd % (
           {'ar_name':ar_name,
            'input':version_dir,
            'output':archive.replace('\\', '/')}),
      cwd=temp_dir,
      env=cygwin_env,
      shell=True)
  assert tarball.wait() == 0

  bot.BuildStep('create Windows installer')
  bot.Print('generate_windows_installer is creating the windows installer.')
  build_tools_dir = os.path.join(home_dir, 'src', 'build_tools')
  done1 = (os.path.join(build_tools_dir, 'done1'))
  if os.path.exists(done1):
    os.remove(done1)
  exefile = subprocess.Popen([
      os.path.join(cygwin_dir, 'bash.exe'),
      'make_native_client_sdk.sh', '-V',
      build_utils.RawVersion(), '-v', '-n'],
      cwd=build_tools_dir)
  exefile.wait()
  if os.path.exists(done1):
    done2 = (os.path.join(build_tools_dir, 'done2'))
    bot.Print("NSIS script created - time to run makensis!")
    if os.path.exists(done2):
      os.remove(done2)
    exefile2 = subprocess.Popen([
        os.path.join(cygwin_dir, 'bash.exe'),
        'make_native_client_sdk2.sh', '-V',
        build_utils.RawVersion(), '-v', '-n'],
        cwd=build_tools_dir)
    exefile2.wait()
    if os.path.exists(done2):
      bot.Print("Installer created!")

  # Clean up.
  shutil.rmtree(temp_dir)


if __name__ == '__main__':
  main(sys.argv[1:])
