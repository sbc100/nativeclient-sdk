#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" Tests for the SDK Installer

The general structure is as follows:

 1. Extract the installer into a given temporary directory
 2. Run tests -- See TestSDK class (e.g., testExamples)
 3. Remove the installer directory
"""

from __future__ import with_statement

import datetime
import httplib
import optparse
import os
import platform
import shutil
import socket
import string
import subprocess
import sys
import time
import unittest

from build_tools import build_utils

annotator = build_utils.BotAnnotator()


def TestingClosure(_outdir, _jobs):
  '''This closure provides the variables needed by the various tests

  Args:
    _outdir: The output directory that holds the extracted installer
    _jobs: Number of parallel jobs for Make or Scons

  Returns:
    A TestCase class that can be used by the unittest loader
  '''
  toolchain_base = build_utils.NormalizeToolchain(base_dir=_outdir)
  toolchain_bin = os.path.join(toolchain_base, 'bin')
  gcc64 = os.path.join(toolchain_bin, 'x86_64-nacl-gcc')
  sel_ldr64 = os.path.join(toolchain_bin, 'sel_ldr_x86_64')

  class TestSDK(unittest.TestCase):
    '''Contains tests that run within an extracted SDK installer'''

    def testBuildExamples(self):
      '''Verify that we can build the SDK examples.'''

      scons = 'scons.bat' if sys.platform == 'win32' else 'scons'
      path = os.path.join(_outdir, 'examples')
      command = [os.path.join(path, scons), '-j', _jobs]

      annotator.Run(' '.join(command), cwd=path, shell=True)
      return True

    def testReadMe(self):
      '''Check that the current build version and date are in the README file'''

      filename = 'README.txt' if sys.platform == 'win32' else 'README'
      with open(os.path.join(_outdir, filename), 'r') as file:
        contents = file.read()
      version = build_utils.RawVersion()
      annotator.Print('Checking that SDK version = %s' % version)
      self.assertTrue(contents.count(version) == 1,
                      'Version mismatch in %s' % filename)

      # Check that the README contains either the current date or yesterday's
      # date (which happens when building over midnight)
      self.assertEqual(
          1,
          contents.count(str(datetime.date.today())) +
          contents.count(str(datetime.date.today() -
                             datetime.timedelta(days=1))),
          "Cannot find today's or yesterday's date in README")
      return True

    def testHttpd(self):
      '''Test the simple HTTP server.

      Run the simple server and make sure it quits when processing an URL that
      has the ?quit=1 parameter set.  This test runs the server on the default
      port (5103) and on a specified port.
      '''

      DEFAULT_SERVER_PORT = 5103

      def runAndQuitHttpServer(port=DEFAULT_SERVER_PORT,
                               alternate_cwd=None,
                               extra_args=[],
                               should_fail=False):
        '''A small helper function to launch the simple HTTP server.

        This function launches the simple HTTP server, then waits for its
        banner output to appear.  If the banner doesn't appear within 10
        seconds, the test fails.  The banner is checked validate that it
        displays the right port number.

        Once the server is verified as running, this function sends it a GET
        request with the ?quit=1 URL parmeter.  It then waits to see if the
        server process exits with a return code of 0.  If the server process
        doesn't exit within 20 seconds, the test fails.

        Args:
          port: The port to use, defaults to 5103.
        '''
        path = os.path.join(_outdir, 'examples')
        command = [sys.executable, os.path.join(path, 'httpd.py')]
        # Add the port only if it's not the default.
        if port != DEFAULT_SERVER_PORT:
          command += [str(port)]
        command += extra_args
        # Can't use annotator.Run() because the HTTP server doesn't stop, which
        # causes Run() to hang.
        annotator.Print('Starting server: %s' % command)
        annotator.Print('extra_args=%s' % str(extra_args))
        current_working_dir = path if alternate_cwd is None else alternate_cwd
        annotator.Print('cwd=%s' % current_working_dir)
        process = subprocess.Popen(command,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT,
                                   cwd=current_working_dir)
        self.assertNotEqual(None, process)
        # Wait until the process starts by trying to send it a GET request until
        # the server responds, or until the timeout expires.  If the timeout
        # expires, fail the test.
        time_start = time.time()
        time_now = time_start
        timeout_time = time_start + 20  # 20 sec timeout.
        output = ''
        conn = None
        while time_now < timeout_time:
          conn = httplib.HTTPConnection('localhost', port)
          try:
            # Send the quit request.
            conn.request("GET", "/?quit=1")
            output = process.stdout.readline()
            break
          except socket.error:
            # If the server is not listening for connections, then
            # HTTPConnetion() raises a socket exception, not one of the
            # exceptions defined in httplib.  In order to resend a request in
            # this case, the original connection has to be closed and re-opened.
            conn.close()
            conn = None
            time.sleep(1)  # Wait a second to try again.
          time_now = time.time()

        # If we expect the test to fail (e.g. bad directory without
        # --no_dir_check) then there should be no connection.
        if should_fail:
          self.assertEqual(None, conn)
          return_code = process.poll()
          # If the process has not terminated, return_code will be None
          # but since the server should have failed to launch, it should
          # have terminated by now.
          self.assertNotEqual(return_code, None)
          return

        self.assertNotEqual(None, conn)
        # Validate the first line of the startup banner.  An example of the
        # full line is:
        #   INFO:root:Starting local server on port 5103
        self.assertTrue(output.startswith('INFO:root:Starting'))
        self.assertEqual(1, output.count(str(port)))
        annotator.Print('Server startup banner: %s' % output)

        # Close down the connection and wait for the server to quit.
        conn.getresponse()
        conn.close()

        # Check to see if the server quit properly.  It should quit within
        # 0.5 seconds, so if the first poll() indicates that the process is
        # still running, wait 1 sec and then poll again.  If the process is
        # still running after 20 sec, then fail the test.
        return_code = process.poll()
        poll_count = 0
        while return_code == None and poll_count < 20:
          time.sleep(1)
          return_code = process.poll()
          poll_count += 1
        self.assertEqual(0, return_code)

      runAndQuitHttpServer()
      runAndQuitHttpServer(5280)
      # Make sure server works outside examles with --no_dir_check.
      runAndQuitHttpServer(5281, alternate_cwd=_outdir,
        extra_args=['--no_dir_check'])
      # Make sure it works in examples with --no_dir_check.
      runAndQuitHttpServer(5281, extra_args=['--no_dir_check'])
      # Make sure the test fails if --no_dir_check is left out and the CWD
      # is not examples.
      runAndQuitHttpServer(5281, alternate_cwd=_outdir, should_fail=True)
      # Retest port 5281 with the default parameters.
      runAndQuitHttpServer(5281)
      return True

    def testProjectTemplates(self):
      '''Create and build projects from project_templates.'''

      def initAndCompileProject(project_name, flags=[]):
        '''A small helper function that runs init_project.py and then runs
        a scons build in the resulting directory.

        Args:
          project_name: The project's name, set the --name= parameter for
              init_project to this value.
          flags: Any extra flags to pass to init_project.  Must be an array,
              can be empty.
        '''
        path = os.path.join(_outdir, 'project_templates')
        scons = 'scons.bat' if sys.platform == 'win32' else 'scons'
        scons_command = [os.path.join(path, project_name, scons), '-j', _jobs]
        init_project_command = [sys.executable,
                                'init_project.py',
                                '--name=%s' % project_name] + flags
        annotator.Run(' '.join(init_project_command), cwd=path, shell=True)
        annotator.Run(' '.join(scons_command),
                      cwd=os.path.join(path, project_name),
                      shell=True)

      initAndCompileProject('test_c_project', flags=['-c'])
      initAndCompileProject('test_cc_project')
      return True

    def testValgrind(self):
      '''Verify that Valgrind works properly (Linux 64-bit only)'''

      if (sys.platform not in ['linux', 'linux2'] or
          platform.machine() != 'x86_64'):
        annotator.Print('Not running on 64-bit Linux -- skip')
        return True
      true_basename = os.path.join(_outdir, 'true')
      true_c_filename = '%s.c' % true_basename
      true_nexe_filename = '%s.nexe' % true_basename
      with open(true_c_filename, 'w') as true_file:
        true_file.write('int main(void) { return 0; }\n')
      annotator.Run([gcc64, '-o', true_nexe_filename, '-m64', '-O0',
                     '-Wl,-u,have_nacl_valgrind_interceptors', '-g',
                     true_c_filename, '-lvalgrind'])
      memcheck = os.path.join(_outdir, 'third_party', 'valgrind', 'memcheck.sh')
      annotator.Run([memcheck, sel_ldr64, '-Q', true_nexe_filename])

  return TestSDK


def ExtractInstaller(installer, outdir):
  '''Extract the SDK installer into a given directory

  If the outdir already exists, then this function deletes it

  Args:
    installer: full path of the SDK installer
    outdir: output directory where to extract the installer

  Raises:
    OSError - if the outdir already exists
    CalledProcessError - if the extract operation fails
  '''

  annotator.Print('Extracting installer %s into %s' % (installer, outdir))

  if os.path.exists(outdir):
    RemoveDir(outdir)

  if sys.platform == 'win32':
    # Run the self-extracting installer in silent mode with a specified
    # output directory
    command = [installer, '/S', '/D=%s' % outdir]
  else:
    os.mkdir(outdir)
    command = ['tar', '-C', outdir, '--strip-components=1',
               '-xvzf', installer]

  annotator.Print('Running command: %s' % command)
  subprocess.check_call(command)


def RemoveDir(outdir):
  '''Removes the given directory

  On Unix systems, this just runs shutil.rmtree, but on Windows, this doesn't
  work when the directory contains junctions (as does our SDK installer).
  Therefore, on Windows, it runs rmdir /S /Q as a shell command.  This always
  does the right thing on Windows.

  Args:
    outdir: The directory to delete

  Raises:
    CalledProcessError - if the delete operation fails on Windows
    OSError - if the delete operation fails on Linux
  '''

  annotator.Print('Removing %s' % outdir)
  if sys.platform == 'win32':
    subprocess.check_call(['rmdir /S /Q', outdir], shell=True)
  else:
    shutil.rmtree(outdir)


def main():
  '''Main entry for installer tests

  Returns:
    0: Success
    1: Failure

  Also, raises various exceptions for error conditions.
  '''

  parser = optparse.OptionParser(
      usage='Usage: %prog [options] sdk_installer')
  parser.add_option(
      '-o', '--outdir', dest='outdir', default='sdk_temp_dir',
      help='temporary output directory for holding the installer')
  parser.add_option(
      '-j', '--jobs', dest='jobs', default=1,
      help='number of parallel jobs to run')

  options, args = parser.parse_args()

  if len(args) == 0:
    parser.error('Must provide an sdk_installer')

  if len(args) > 1:
    parser.error('Must provide only one sdk_installer')

  installer = args[0]
  outdir = os.path.abspath(options.outdir)

  annotator.Print("Running with installer = %s, outdir = %s, jobs = %s" % (
                  installer, outdir, options.jobs))
  ExtractInstaller(installer, outdir)

  suite = unittest.TestLoader().loadTestsFromTestCase(
      TestingClosure(outdir, options.jobs))
  result = unittest.TextTestRunner(verbosity=2).run(suite)

  RemoveDir(outdir)

  return int(not result.wasSuccessful())


if __name__ == '__main__':
  sys.exit(main())
