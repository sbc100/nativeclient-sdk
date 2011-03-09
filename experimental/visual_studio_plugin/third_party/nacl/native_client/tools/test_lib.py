#!/usr/bin/python
# Copyright 2008, Google Inc.
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


"""Testing Library For Nacl

"""

import difflib
import re
import signal
import subprocess
import sys
import time

def RunTest(cmd):
  """Run a test where we only care about the return code."""
  assert type(cmd) == list
  failed = 0
  start = time.time()
  try:
    p = subprocess.Popen(cmd)
    retcode = p.wait()
  except OSError:
    print 'exception: ' + str(sys.exc_info()[1])
    retcode = 0
    failed = 1

  end = time.time()
  return (end - start, retcode, failed)


def RunTestWithInputOutput(cmd, input_data):
  """Run a test where we also care about stdin/stdout/stderr.

  NOTE: this function may have problems with arbitrarily
        large input or output, especially on windows
  NOTE: input_data can be either a string or or a file like object,
        file like objects may be better for large input/output
  """
  assert type(cmd) == list
  stdout = ''
  stderr = ''
  failed = 0
  start = time.time()
  try:
    # Python on windows does not include any notion of SIGPIPE.  On
    # Linux and OSX, Python installs a signal handler for SIGPIPE that
    # sets the handler to SIG_IGN so that syscalls return -1 with
    # errno equal to EPIPE, and translates those to exceptions;
    # unfortunately, the subprocess module fails to reset the handler
    # for SIGPIPE to SIG_DFL, and the SIG_IGN behavior is inherited.
    # subprocess.Popen's preexec_fn is apparently okay to use on
    # Windows, as long as its value is None.

    if hasattr(signal, 'SIGPIPE'):
      no_pipe = lambda : signal.signal(signal.SIGPIPE, signal.SIG_DFL)
    else:
      no_pipe = None

    if type(input_data) == str:
      p = subprocess.Popen(cmd,
                           bufsize=1000*1000,
                           stdin=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           stdout=subprocess.PIPE,
                           preexec_fn = no_pipe)
      stdout, stderr = p.communicate(input_data)
    else:
      # input_data is a file like object
      p = subprocess.Popen(cmd,
                           bufsize=1000*1000,
                           stdin=input_data,
                           stderr=subprocess.PIPE,
                           stdout=subprocess.PIPE,
                           preexec_fn = no_pipe)
      stdout, stderr = p.communicate()
    retcode = p.wait()
  except OSError, x:
    if x.errno == 10:
      print '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@'
      print 'ignoring exception', str(sys.exc_info()[1])
      print 'return code NOT checked'
      print 'this seems to be a windows issue'
      print '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@'
      failed = 0
      retcode = 0
    else:
      print 'exception: ' + str(sys.exc_info()[1])
      retcode = 0
      failed = 1
  end = time.time()
  return (end - start, retcode, failed, stdout, stderr)


def DiffStringsIgnoringWhiteSpace(a, b):
  a = a.splitlines()
  b = b.splitlines()
  # NOTE: the whitespace stuff seems to be broken in python
  cruncher = difflib.SequenceMatcher(lambda x: x in ' \t\r', a, b)

  for group in cruncher.get_grouped_opcodes():
    eq = True
    for tag, i1, i2, j1, j2 in group:
      if tag != 'equal':
        eq = False
        break
    if eq: continue
    i1, i2, j1, j2 = group[0][1], group[-1][2], group[0][3], group[-1][4]
    yield '@@ -%d,%d +%d,%d @@\n' % (i1+1, i2-i1, j1+1, j2-j1)

    for tag, i1, i2, j1, j2 in group:
      if tag == 'equal':
        for line in a[i1:i2]:
          yield ' [' + line + ']'
        continue
      if tag == 'replace' or tag == 'delete':
        for line in a[i1:i2]:
          yield '-[' + repr(line) + ']'
      if tag == 'replace' or tag == 'insert':
        for line in b[j1:j2]:
          yield '+[' + repr(line) + ']'


def RegexpFilterLines(regexp, lines):
  """Apply regexp to filter lines of text, keeping only those lines
  that match.

  Any carriage return / newline sequence is turned into a newline.

  Args:
    regexp: A regular expression which may contain regexp groups, in
      which case the line is replaced with the groups (text outside
      the groups are omitted, useful for eliminating file names that
      might change, etc).

    lines: A string containing newline-separated lines of text

  Return:
    Filtered lines of text, newline separated.
  """

  result = []
  nfa = re.compile(regexp)
  for line in lines.split('\n'):
    if line.endswith('\r'):
      line = line[:-1]
    mobj = nfa.search(line)
    if mobj:
      if not mobj.groups():
        result.append(line)
      else:
        matched_strings = []
        for s in mobj.groups():
          if s is not None:
            matched_strings.append(s)
        result.append(''.join(matched_strings))
  return '\n'.join(result)
