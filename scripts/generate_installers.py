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
import subprocess
import sys
import tarfile

EXCLUDE_DIRS = ['.svn', '.download', 'scons-out', 'packages']

# Return True if |file| should be excluded from the tarball.
def ExcludeFile(file):
  return (file.startswith('.DS_Store') or
          re.search('^\._', file) or
          file == 'DEPS')


def SVNRevision():
  p = subprocess.Popen(['svn', 'info'],
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE, shell=True)
  (stdout, stderr) = p.communicate()
  m = re.search('Revision: ([0-9]+)', stdout)
  if m:
    return int(m.group(1))
  else:
    return 0


def VersionString():
  rev = SVNRevision()
  build_number = os.environ.get('BUILD_NUMBER', '0')
  return 'native_client_sdk_0_1_%d_%s' % (rev, build_number)


def main(argv):
  os.chdir('src')
  version = VersionString()
  tar = tarfile.open('../nacl-sdk.tgz', 'w:gz')
  for root, dirs, files in os.walk('.'):
    for excl in EXCLUDE_DIRS:
      if excl in dirs:
        dirs.remove(excl)
    files = [f for f in files if not ExcludeFile(f)]
    for name in files:
      path = os.path.join(root, name)
      arcname = os.path.join(version, path)
      tar.add(path, arcname=arcname)
  tar.close()


if __name__ == '__main__':
  main(sys.argv[1:])
