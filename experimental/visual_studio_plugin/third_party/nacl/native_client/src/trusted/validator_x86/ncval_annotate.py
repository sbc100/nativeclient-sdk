#!/usr/bin/env python

# Copyright 2009  The Native Client Authors.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Annotates the output of ncval with line numbers, taken from debugging
# information using binutils' addr2line.

# TODO(mseaborn): Merge this with validator_arm/v2/validation-report.py,
# the Python wrapper script for the ARM validator.

import linecache
import re
import subprocess
import sys


objdump_regexp = re.compile(r"\s*([0-9a-f]+):\s+")
ncval_regexp = re.compile("VALIDATOR: ([0-9a-f]+):")
addr2line_regexp = re.compile(r"(.*):(\d+)$")


def check(obj_file, output_fh):
  def disassemble_address(addr):
    proc = subprocess.Popen(
      ["nacl-objdump", "-d", obj_file,
       "--start-address", "0x" + addr,
       "--stop-address", "0x%x" % (int(addr, 16) + 16)],
    stdout=subprocess.PIPE)
    for line in proc.stdout:
      match = objdump_regexp.match(line)
      if match is not None:
        output_fh.write("  code: %s" % line[match.end():])
        break

  def decode_address(addr):
    # We have to do one invocation of addr2line per address when
    # getting inline context.  If we pipe addresses in, addr2line
    # does not tell us when the output ends for each input
    # address.
    proc = subprocess.Popen(
      ["nacl-addr2line", "--functions", "--inlines", "-e", obj_file, addr],
      stdout=subprocess.PIPE)
    for info in proc.stdout:
      match = addr2line_regexp.match(info.rstrip())
      if match is not None:
        filename = match.group(1)
        lineno = int(match.group(2))
        output_fh.write("  file: %s" % info)
        src_line = linecache.getline(filename, lineno)
        if src_line != "":
          output_fh.write("    " + src_line.lstrip())
      else:
        output_fh.write("  func: %s" % info)

  proc = subprocess.Popen(["ncval", obj_file], stdout=subprocess.PIPE)
  for line in proc.stdout:
    match = ncval_regexp.match(line)
    if match is not None:
      output_fh.write("\n")
      output_fh.write(line)
      addr = match.group(1)
      disassemble_address(addr)
      decode_address(addr)
  return proc.wait()


def main(args):
  result = 0
  for obj_file in args:
    if check(obj_file, sys.stdout) != 0:
      result = 1
  return result


if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
