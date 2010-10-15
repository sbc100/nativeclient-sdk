#!/usr/bin/python
# Copyright (c) 2010 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.

"""Install GTest and GMock that can be linked to a NaCl module.  By default
this script builds both the 32- and 64-bit versions of the libraries, and
installs them in <toolchain>/nacl/usr/lib and <toolchain>/nacl64/usr/lib.  The
header files are also installed in <toolchain>/<ncal_spec>/usr/include.
"""

import optparse
import os
import shutil
import subprocess
import sys
import tempfile
import urllib

# Default values for the --toolchain and --bit-spec coimmand line arguments.
TOOLCHAIN_AUTODETECT = "AUTODETECT"
BIT_SPEC = "32,64"

# The original gtest distro can be found here:
# http://code.google.com/p/googletest/downloads/detail?name=gtest-1.5.0.tar.gz
GTEST_URL = "http://build.chromium.org/mirror/nacl/gtest-1.5.0.tgz"
GTEST_PATH = "gtest-1.5.0"
GTEST_PATCH_FILE = "nacl-gtest-1.5.0.patch"

# The original gmock distro can be found here:
# http://googlemock.googlecode.com/files/gmock-1.5.0.tar.gz
GMOCK_URL = "http://build.chromium.org/mirror/nacl/gmock-1.5.0.tgz"
GMOCK_PATH = "gmock-1.5.0"
GMOCK_PATCH_FILE = "nacl-gmock-1.5.0.patch"

# Map the string stored in |sys.platform| into a toolchain platform specifier.
PLATFORM_MAPPING = {
    'win32': 'win_x86',
    'cygwin': 'win_x86',
    'linux': 'linux_x86',
    'linux2': 'linux_x86',
    'darwin': 'mac_x86',
    'macos': 'mac_x86',
}


# Make all the directories in |abs_path|.  If these paths already exist, this
# method does nothing.
def ForceMakeDirs(abs_path):
  try:
    os.makedirs(abs_path)
  except:
    pass


# patch version 2.6 doesn't work.  Most of our Linux distros use patch 2.6
def CheckPatchVersion():
  patch = subprocess.Popen("patch --version",
                            shell=True,
                            stdout=subprocess.PIPE)
  sed = subprocess.Popen("sed q",
                         shell=True,
                         stdin=patch.stdout,
                         stdout=subprocess.PIPE)
  sed_output = sed.communicate()[0]
  if sed_output.strip() == 'patch 2.6':
    print "patch 2.6 is incompatible with these scripts."
    print "Please install either version 2.5.9 (or earlier)"
    print "or version 2.6.1 (or later)."
    return False
  return True


# Create a temporary working directory.  This is where all the tar files go
# and where the packages get built prior to installation in the toolchain.
def MakeWorkingDir(options):
  # Pick work directory.
  options.working_dir = tempfile.mkdtemp(prefix='gtest')
  # Go into working area.
  options.old_cwd = os.getcwd()
  os.chdir(options.working_dir)


# Download GTest and GMock into the working directory, then extract them.
def DownloadAndExtractAll(options):
  def DownloadAndExtract(url, path):
    print "Download: %s" % url
    (zip_file, headers) = urllib.urlretrieve(url, '%s.tgz' % path)
    p = subprocess.Popen('tar xzf %s' % (zip_file), shell=True)
    assert p.wait() == 0

  os.chdir(options.working_dir)
  try:
    DownloadAndExtract(GTEST_URL, GTEST_PATH)
    DownloadAndExtract(GMOCK_URL, GMOCK_PATH)
  except (URLError, ContentTooShortError):
    os.chdir(options.old_cwd)
    print "Error retrieving %s" % url
    raise

  os.chdir(options.old_cwd)


# Apply the patch files to the extracted GTest and GMock pacakges.
def PatchAll(options):
  def Patch(abs_path, patch_file):
    print "Patching %s with: %s" % (abs_path, patch_file)
    p = subprocess.Popen('patch -p0 < %s' % (patch_file),
                         cwd=abs_path,
                         shell=True)
    assert p.wait() == 0

  Patch(options.working_dir,
        os.path.join(options.script_dir, 'patch_files', GTEST_PATCH_FILE))
  Patch(options.working_dir,
        os.path.join(options.script_dir, 'patch_files', GMOCK_PATCH_FILE))


# Build GTest and GMock, then install them into the toolchain.  Note that
# GTest has to be built and installed into the toolchain before GMock can be
# built, because GMock relies on headers from GTest.  This method sets up all
# the necessary shell environment variables for the makefiles, such as CC and
# CXX.
def BuildAndInstallAll(options):
  def BuildInPath(abs_path):
    # Run make clean and make in |abs_path|.  Assumes there is a makefile in
    # |abs_path|, if not then the assert will trigger.
    print "Building in %s" % (abs_path)
    p = subprocess.Popen('make clean ; make -j4',
                         cwd=abs_path,
                         shell=True)
    assert p.wait() == 0

  def InstallLib(lib, src_path, dst_path):
    # Use the install untility to install |lib| from |src_path| into
    # |dst_path|.
    p = subprocess.Popen("install -m 644 %s %s" % (lib, dst_path),
                         cwd=src_path,
                         shell=True)
    assert p.wait() == 0

  # Build and install for each bit-spec.
  for bit_spec in options.bit_spec.split(','):
    # 32-bits is treated specially, due to being the empty string in legacy
    # code.
    nacl_spec = bit_spec == '32' and 'nacl' or ('nacl%s' % bit_spec)
    print 'Building NaCl spec: %s.' % nacl_spec
    # Make sure the target directories exist.
    nacl_usr_include = os.path.join(options.toolchain,
                                    nacl_spec,
                                    'usr',
                                    'include')
    nacl_usr_lib = os.path.join(options.toolchain,
                                nacl_spec,
                                'usr',
                                'lib')
    ForceMakeDirs(nacl_usr_include)
    ForceMakeDirs(nacl_usr_lib)

    # Set up the nacl-specific environment variables used by make.
    toolchain_bin = os.path.join(options.toolchain, 'bin')
    os.putenv('CC', os.path.join(toolchain_bin, '%s-gcc' % nacl_spec))
    os.putenv('CXX', os.path.join(toolchain_bin, '%s-g++' % nacl_spec))
    os.putenv('AR', os.path.join(toolchain_bin, '%s-ar' % nacl_spec))
    os.putenv('RANLIB', os.path.join(toolchain_bin, '%s-ranlib' % nacl_spec))
    os.putenv('LD', os.path.join(toolchain_bin, '%s-ld' % nacl_spec))
    os.putenv('NACL_TOOLCHAIN_ROOT', options.toolchain)

    # GTest has to be built & installed before GMock can be built.
    gtest_path = os.path.join(options.working_dir, GTEST_PATH)
    BuildInPath(gtest_path)
    gtest_tar_excludes = "--exclude='gtest-death-test.h' \
                          --exclude='gtest-death-test-internal.h'"
    tar_cf = subprocess.Popen("tar cf - %s gtest" % gtest_tar_excludes,
                              cwd=os.path.join(gtest_path, 'include'),
                              shell=True,
                              stdout=subprocess.PIPE)
    tar_xf = subprocess.Popen("tar xfp -",
                              cwd=nacl_usr_include,
                              shell=True,
                              stdin=tar_cf.stdout)
    tar_copy_err = tar_xf.communicate()[1]
    InstallLib('libgtest.a', gtest_path, nacl_usr_lib)

    gmock_path = os.path.join(options.working_dir, GMOCK_PATH)
    BuildInPath(gmock_path)
    tar_cf = subprocess.Popen("tar cf - gmock",
                              cwd=os.path.join(gmock_path, 'include'),
                              shell=True,
                              stdout=subprocess.PIPE)
    tar_xf = subprocess.Popen("tar xfp -",
                              cwd=nacl_usr_include,
                              shell=True,
                              stdin=tar_cf.stdout)
    tar_copy_err = tar_xf.communicate()[1]
    InstallLib('libgmock.a', gmock_path, nacl_usr_lib)


# Main driver method that creates a working directory, then downloads and
# extracts GTest and GMock, patches and builds them both, then installs them
# into the toolchain specified in |options.toolchain|.
def InstallTestingLibs(options):
  MakeWorkingDir(options)
  try:
    DownloadAndExtractAll(options)
    PatchAll(options)
  except:
    return 1
  BuildAndInstallAll(options)
  # Clean up.
  shutil.rmtree(options.working_dir, ignore_errors=True)
  return 0


# Build a toolchain path based on the platform type.  |base_dir| is the root
# directory which includes the platform-specific toolchain.  This could be
# something like "/usr/local/mydir/nacl_sdk/src".  This method assumes that
# the platform-specific toolchain is found under
# <base_dir>/toolchain/<platform_spec>.
def AutoDetectToolchain(toochain_base=''):
  if sys.platform in PLATFORM_MAPPING:
    return os.path.join(toochain_base,
                        'toolchain',
                        PLATFORM_MAPPING[sys.platform])
  else:
    print 'ERROR: Unsupported platform "%s"!' % sys.platform
    return base_dir


# Parse the command-line args and set up the options object.  There are two
# command-line switches:
#   --toolchain=<path to the platform-specific toolchain>
#               e.g.: --toolchain=../toolchain/mac-x86
#               default is |TOOLCHAIN_AUTODETECT| which means try to determine
#               the toolchain dir from |sys.platform|.
#   --bit-spec=<comma-separated list of instruction set bit-widths
#              e.g.: --bit_spec=32,64
#              default is "32,64" which means build 32- and 64-bit versions
#              of the libraries.
def main(argv):
  if not CheckPatchVersion():
    sys.exit(0)

  parser = optparse.OptionParser()
  parser.add_option(
      '-t', '--toolchain', dest='toolchain',
      default=TOOLCHAIN_AUTODETECT,
      help='where to put the NaCl tool binaries')
  parser.add_option(
      '-b', '--bit-spec', dest='bit_spec',
      default=BIT_SPEC,
      help='comma separated list of instruction set bit-widths')
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    print 'ERROR: invalid argument'
    sys.exit(1)

  options.script_dir = os.path.abspath(os.path.dirname(__file__))

  if options.toolchain == TOOLCHAIN_AUTODETECT:
    (base_dir, _) = os.path.split(options.script_dir)
    options.toolchain = AutoDetectToolchain(toochain_base=base_dir)
  else:
    options.toolchain = os.path.abspath(options.toolchain)
  print "Installing into toolchain %s" % options.toolchain

  return InstallTestingLibs(options)


if __name__ == '__main__':
  main(sys.argv[1:])
