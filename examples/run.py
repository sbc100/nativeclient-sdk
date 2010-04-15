#!/usr/bin/python
#
# Copyright 2010, The Native Client SDK Authors.  All Rights Reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.
#

"""Build and run the SDK examples.

This script tries to find an installed version of the Chrome Browser that can
run the SDK examples.  If such a version is installed, it builds the publish
versions of each example, then launches a local http server.  It then runs
the installed verison of the Chrome Browser, open to the URL of the requested
example.

Options:
  --chrome-path=<abs_path> Absolute path to the Chrome Browser used to run
      the examples.
  --example=<example> Run |example|.  Possible values are: hello_world,
      pi_generator and tumbler.
"""

import getopt
import os
import subprocess
import sys
import urllib

help_message = '''
--chrome-path=<abs_path> Absolute path to the Chrome Browser used to run the
    examples.

--example=<example> Runs the selected example.  Possible values are:
    hello_world, pi_generator, tumbler.
'''


DEFAULT_CHROME_INSTALL_PATH_MAP = {
    'win32': r'c:\Program Files\chrome-win32',
    'cygwin': r'c:\cygwin\bin',
    'linux': '/opt/google/chrome',
    'linux2': '/opt/google/chrome',
    'darwin': '/Applications'
}


CHROME_EXECUTABLE_MAP = {
    'win32': 'chrome.exe',
    'cygwin': 'chrome.exe',
    'linux': 'chrome',
    'linux2': 'chrome',
    'darwin': 'Chromium.app/Contents/MacOS/Chromium'
}


PLATFORM_COLLAPSE = {
    'win32': 'win32',
    'cygwin': 'win32',
    'linux': 'linux',
    'linux2': 'linux',
    'darwin': 'mac',
}


SERVER_PORT = 5103


class Usage(Exception):
  def __init__(self, msg):
    self.msg = msg


# Look for a Google Chrome executable in the given install path.  If
# |chrome_install_path| is |None|, then the platform-specific install path is
# used.  Returns the 2-tuple whose elements are:
#     the actual install path used (which might be the default)
#     the full path to the executable
# If Google Chrome can't be found, then the 2-tuple returned is:
#     the actual install path searched
#     None
def FindChrome(chrome_install_path=None):
  if chrome_install_path is None:
    # Use the platform-specific default path for Chrome.
    chrome_install_path = DEFAULT_CHROME_INSTALL_PATH_MAP[sys.platform]
  chrome_exec = CHROME_EXECUTABLE_MAP[sys.platform]
  full_chrome_path = os.path.join(chrome_install_path, chrome_exec)
  if os.path.exists(full_chrome_path):
    return chrome_install_path, full_chrome_path
  return chrome_install_path, None


# Checks to see if there is a simple HTTP server running on |SERVER_PORT|.  Do
# this by attempting to open a URL socket on localhost:|SERVER_PORT|.
def IsHTTPServerRunning():
  is_running = False
  try:
    url = urllib.urlopen('http://localhost:%d' % SERVER_PORT)
    is_running = True

  except IOError:
    is_running = False

  return is_running


def main(argv=None):
  if argv is None:
    argv = sys.argv
  chrome_install_path = os.environ.get('CHROME_INSTALL_PATH', None)
  example = 'hello_world'
  try:
    try:
      opts, args = getopt.getopt(argv[1:], 'ho:p:v',
                                 ['help', 'example=', 'chrome-path='])
    except getopt.error, msg:
      raise Usage(msg)
  
    # option processing
    for option, value in opts:
      if option == '-v':
        verbose = True
      if option in ('-h', '--help'):
        raise Usage(help_message)
      if option in ('-e', '--example'):
        example = value
      if option in ('-p', '--chrome-path'):
        chrome_install_path = value
  
  except Usage, err:
    print >> sys.stderr, sys.argv[0].split('/')[-1] + ': ' + str(err.msg)
    print >> sys.stderr, '--help Print this help message.'
    return 2

  # Look for an installed version of the Chrome Browser.  The default path
  # is platform-dependent, and can be set with the --chrome-path= option.
  chrome_install_path, chrome_exec = FindChrome(chrome_install_path)
  if chrome_exec is None:
    print >> sys.stderr, 'Can\'t find Google Chrome in path \"%s\"' % \
        chrome_install_path
    return 2
  
  print 'Using Google Chrome found at: ', chrome_exec

  env = os.environ.copy()
  if sys.platform == 'win32':
    env['PATH'] = r'c:\cygwin\bin;' + env['PATH']
  home_dir = os.path.realpath(os.curdir)

  # Build the examples.
  make = subprocess.Popen('make publish', env=env, cwd=home_dir, shell=True)
  make_err = make.communicate()[1]

  # Run the local http server, if it isn't already running.
  if not IsHTTPServerRunning():
    subprocess.Popen('python ./httpd.py', cwd=home_dir, env=env, shell=True)
  else:
    print 'localhost HTTP server is running.'

  # Launch Google Chrome with the desired example.
  example_url = 'http://localhost:%(server_port)s/publish/' \
      '%(platform)s_%(target)s/%(example)s.html'
  subprocess.Popen('"' + chrome_exec + '"' + ' --enable-nacl ' +
                   example_url % ({'server_port':SERVER_PORT,
                                   'platform':PLATFORM_COLLAPSE[sys.platform],
                                   'target':'x86',
                                   'example':example}),
                   env=env,
                   cwd=home_dir,
                   shell=True)


if __name__ == '__main__':
  sys.exit(main())
