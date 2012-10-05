#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file

"""Python wrapper around gcc to make it behave a little
more like cl.exe WRT to parallel building.
"""

import multiprocessing
import os
import Queue
import shlex
import subprocess
import sys
import threading
import time

verbose = int(os.environ.get('NACL_GCC_VERBOSE', '0'))
show_commands = int(os.environ.get('NACL_GCC_SHOW_COMMANDS', '0'))
stop_on_error = False


def RunGCC(cmd, basename):
  """Run gcc and return the result along will the stdout/stderr."""
  cmdstring = subprocess.list2cmdline(cmd)
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  stdout, stderr = p.communicate()
  p.wait()
  if show_commands:
    logmsg = cmdstring
  else:
    logmsg = basename
  stderr = logmsg + '\n' + stderr
  return (p.returncode, stdout, stderr)


def BuildSerial(base_cmd, outpath, files):
  final_result = 0

  for filename in files:
    cmd, basename = MakeCommand(base_cmd, outpath, filename)
    rtn, stdout, stderr = RunGCC(cmd, basename)
    sys.stdout.write(stdout)
    sys.stdout.flush()
    sys.stderr.write(stderr)
    sys.stderr.flush()
    if rtn:
      final_result = rtn
      if stop_on_error:
        break

  return final_result


def Worker(queue, out_queue):
  """Entry point got worker threads.

  Each thread will compiler jobs from the queue until
  there are no jobs left or until the main thread signals
  for the work to stop.
  """
  while not queue.empty() and Worker.running:
    item = queue.get(False)
    if not item:
      break
    results = RunGCC(item[0], item[1])
    out_queue.put(results)


def MakeCommand(base_cmd, outpath, filename):
  """Build the full commandline given that output root
  and the intput filename.

  If VS passes an existing directory to -o, then we derive the
  actual object name by combining he directory name with the
  basename of the source file and andding ".obj"
  """
  basename = os.path.basename(filename)
  out = os.path.join(outpath, os.path.splitext(basename)[0] + '.obj')
  return (base_cmd + ['-c', filename, '-o', out], basename)


def BuildParallel(cores, base_cmd, outpath, files):
  Worker.running = True
  pool = []
  job_queue = Queue.Queue()
  out_queue = Queue.Queue()

  for filename in files:
    cmd, basename = MakeCommand(base_cmd, outpath, filename)
    job_queue.put((cmd, basename))

  # Create worker thread pool, passing job queue
  # and output queue to each worker.
  args = (job_queue, out_queue)
  for i in xrange(cores):
    t = threading.Thread(target=Worker, args=args)
    t.start()

  results = 0
  Trace("waiting for %d results" % len(files))
  final_result = 0
  while results < len(files):
    results += 1
    rtn, stdout, stderr = out_queue.get()
    # stdout seem to be completely ignored by visual studio
    # but GCC should output all useful information on stderr
    # anyway.
    sys.stdout.write(stdout)
    sys.stdout.flush()
    sys.stderr.write(stderr)
    sys.stderr.flush()
    if rtn:
      final_result = rtn
      if stop_on_error:
        # stop all workers
        Worker.running = False
        break

  return final_result


def Log(msg):
  """Log message to stderr."""
  # Since Visual Studio basically seems to completely ignore the stdout
  # of the compiler and only echo stderr we print everythign to stderr.
  sys.stderr.write(str(msg) + '\n')
  sys.stderr.flush()


def Trace(msg):
  if verbose:
    Log("nacl_compiler:" + str(msg))


def main(args):
  if args[0][0] == '@':
    rspfile = args[0][1:]
    args = shlex.split(open(rspfile).read())

  # find the last occurrence of '--' in the argument
  # list and use that to signify the start of the
  # list of sources
  index = list(reversed(args)).index('--')
  index = len(args) - index
  base_cmd = args[:index-1]
  files = args[index:]

  # remove -o <path> from base_cmd
  index = base_cmd.index('-o')
  outpath = base_cmd[index+1]
  del base_cmd[index+1]
  del base_cmd[index]

  cores = int(os.environ.get('NACL_GCC_CORES', '0'))
  if not cores:
    cores = multiprocessing.cpu_count()
  cores = min(cores, len(files))

  Trace("compiling %d sources using %d threads" % (len(files), cores))
  rtn = BuildParallel(cores, base_cmd, outpath, files)
  Trace("returning %d" % rtn)
  return rtn


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
