#!/usr/bin/python
#
# Copyright (c) 2010 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This generates symbol signatures with the same algorithm as
# src/breakpad/src/common/linux/file_id.cc@461

import struct
import sys
import subprocess

if len(sys.argv) != 2:
  sys.stderr.write("Error, no filename specified.\n")
  sys.exit(1)

# Shell out to objdump to get the offset of the .text section
objdump = subprocess.Popen(['objdump', '-h', sys.argv[1]], stdout = subprocess.PIPE)
(sections, _) = objdump.communicate()
if objdump.returncode != 0:
  sys.stderr.write('Failed to run objdump to find .text section.\n')
  sys.exit(1)

text_section = [x for x in sections.splitlines() if '.text' in x]
if len(text_section) == 0:
  sys.stderr.write('objdump failed to find a .text section.\n')
  sys.exit(1)
text_section = text_section[0]
try:
  file_offset = int(text_section.split()[5], 16)
except ValueError:
  sys.stderr.write("Failed to parse objdump output. Here is the failing line:\n");
  sys.stderr.write(text_section)
  sys.exit(1)

bin = open(sys.argv[1])
bin.seek(file_offset)
if bin.tell() != file_offset:
  sys.stderr.write("Failed to seek to the .text segment. Truncated file?\n");
  sys.exit(1)

data = bin.read(4096)
if len(data) != 4096:
  sys.stderr.write("Error, did not read first page of data.\n");
  sys.exit(1)
bin.close()

signature = [0] * 16
for i in range(0, 4096):
  signature[i % 16] ^= ord(data[i])

# Append a 0 at the end for the generation number (always 0 on Linux)
out = ('%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X0' %
      struct.unpack('I2H8B', struct.pack('16B', *signature)))
sys.stdout.write(out)
