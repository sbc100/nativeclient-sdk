#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for nmf.py."""

import exceptions
import json
import optparse
import os
import subprocess
import sys
import tempfile
import unittest

from build_tools import build_utils
from build_tools.nacl_sdk_scons.site_tools import create_nmf

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def CallGenerateManifest(args):
  '''Calls the create_nmf.py utility and returns stdout as a string

  Args:
    args: command-line arguments as a list (not including program name)

  Returns:
    string containing stdout

  Raises:
    subprocess.CalledProcessError: non-zero return code from sdk_update'''
  command = ['python', os.path.join(SCRIPT_DIR, 'site_tools',
                                    'create_nmf.py')] + args
  process = subprocess.Popen(stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             args=command)
  output, error_output = process.communicate()

  retcode = process.poll() # Note - calling wait() can cause a deadlock
  if retcode != 0:
    print "stdout=%s\nstderr=%s" % (output, error_output)
    raise subprocess.CalledProcessError(retcode, command)
  return output


def GlobalInit(options):
  '''Global initialization for testing nmf file creation.
  '''
  hello_world_c = (
      '#include <stdio.h>\n'
      'int main(void) { printf("Hello World!\\n"); return 0; }\n')
  hello_world_cc = (
      '#include <iostream>\n'
      'int main(void) { std::cout << "Hello World!\\n"; return 0; }\n')

  files = {32: [], 64: []}
  def MakeNexe(contents, suffix, compiler, bits):
    source_filename = None
    try:
      with tempfile.NamedTemporaryFile(delete=False,
                                       suffix=suffix) as temp_file:
        source_filename = temp_file.name
        temp_file.write(contents)
      nexe_filename = os.path.splitext(source_filename)[0] + '.nexe'
      files[bits].append(nexe_filename)
      subprocess.check_call([compiler,
                             '-m32' if bits == 32 else '-m64',
                             source_filename,
                             '-o', nexe_filename])
    finally:
      if source_filename and os.path.exists(source_filename):
        os.remove(source_filename)

  try:
    for bits in build_utils.SupportedNexeBitWidths():
      c_file = MakeNexe(hello_world_c, '.c', options.gcc, bits)
      cc_file = MakeNexe(hello_world_cc, '.cc', options.gpp, bits)
  except:
    GlobalTeardown(files)
    raise

  return files


def TestingClosure(toolchain_dir, bits, files):
  '''Closure to provide variables to the test cases

  Args:
    toolchain_dir: path to toolchain that we are testing
    bits: bit width of nexe (either 32 or 64)'''

  class TestNmf(unittest.TestCase):
    ''' Test basic functionality of the sdk_update package '''

    def setUp(self):
      self.objdump = (os.path.join(toolchain_dir,
                                   'bin',
                                   'x86_64-nacl-objdump'))
      self.library_path = os.path.join(toolchain_dir,
                                       'x86_64-nacl',
                                       'lib32' if bits == 32 else 'lib')

    def testRunGenerateManifest(self):
      for file in files:
        json_text = CallGenerateManifest(
            ['--library-path', self.library_path,
             '--objdump', self.objdump,
             file])
        obj = json.loads(json_text)
        # For now, just do a simple sanity check that there is a file
        # and program section.
        # TODO(mball) Add more tests
        self.assertTrue(obj.get('files'))
        self.assertTrue(obj.get('program'))

  return TestNmf


def GlobalTeardown(temp_files):
  for filelist in temp_files.values():
    for filename in filelist:
      if os.path.exists(filename):
        os.remove(filename)


def main(argv):
  '''Usage: %prog [options]

  Runs the unit tests on the nmf utility'''
  parser = optparse.OptionParser(usage=main.__doc__)

  parser.add_option(
    '-t', '--toolchain-dir', dest='toolchain_dir',
    help='(required) root directory of toolchain')

  (options, args) = parser.parse_args(argv)

  options.gcc = os.path.join(options.toolchain_dir, 'bin', 'x86_64-nacl-gcc')
  options.gpp = os.path.join(options.toolchain_dir, 'bin', 'x86_64-nacl-g++')
  options.objdump = os.path.join(options.toolchain_dir, 'bin',
                                 'x86_64-nacl-objdump')

  success = True
  temp_files = {32: [], 64: []}
  try:
    temp_files = GlobalInit(options)
    for bits in [32, 64]:
      nexe_files = [name for name in temp_files[bits]
                    if os.path.splitext(name)[1] == '.nexe']
      suite = unittest.TestLoader().loadTestsFromTestCase(
          TestingClosure(toolchain_dir=options.toolchain_dir,
                         bits=bits,
                         files=nexe_files))
      result = unittest.TextTestRunner(verbosity=2).run(suite)
      success = result.wasSuccessful() and success
  finally:
    GlobalTeardown(temp_files)

  return int(not success)  # 0 = success, 1 = failure

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
